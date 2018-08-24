//
//  audioloader_ios.cpp
//  ESMusicExtractor
//
//  Created by Ragnar Hrafnkelsson on 07/08/2018.
//  Copyright © 2018 Reactify. All rights reserved.
//

/*
 * Copyright (C) 2006-2016  Music Technology Group - Universitat Pompeu Fabra
 *
 * This file is part of Essentia
 *
 * Essentia is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation (FSF), either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * version 3 along with this program.  If not, see http://www.gnu.org/licenses/
 */

#include "audioloader_ios.h"
#include "algorithmfactory.h"
#include <iomanip>  //  setw()

using namespace std;

namespace essentia {
  namespace streaming {
    
    const char* AudioLoader::name = essentia::standard::AudioLoader::name;
    const char* AudioLoader::category = essentia::standard::AudioLoader::category;
    const char* AudioLoader::description = essentia::standard::AudioLoader::description;
    
    
    AudioLoader::~AudioLoader() {
      closeAudioFile();
#if defined(USE_ACCELERATE)
//      free(_buffer);
#else
      av_freep(&_buffer);
      av_freep(&_md5Encoded);
      av_freep(&_decodedFrame);
#endif
    }
    
    void AudioLoader::configure() {
      // set ffmpeg to be silent by default, so we don't have these annoying
      // "invalid new backstep" messages anymore, when everything is actually fine
#if defined(USE_ACCELERATE)
#else
      av_log_set_level(AV_LOG_QUIET);
#endif
      //av_log_set_level(AV_LOG_VERBOSE);
      _computeMD5 = parameter("computeMD5").toBool();
      _selectedStream = parameter("audioStream").toInt();
      reset();
    }
    
    
    void AudioLoader::openAudioFile(const string& filename) {
      E_DEBUG(EAlgorithm, "AudioLoader: opening file: " << filename);
#if defined(USE_ACCELERATE)
      auto path =
      CFStringCreateWithCString(kCFAllocatorDefault, filename.c_str(),kCFStringEncodingUTF8);
      auto url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, path, kCFURLPOSIXPathStyle, false);
      OSStatus result = ExtAudioFileOpenURL(url, &_file);
      if ( result != noErr ) {
        throw EssentiaException("AudioLoader: Could not open file \"", filename, "\", error = ", result);
      }
#else
      // Open file
      int errnum;
      if ((errnum = avformat_open_input(&_demuxCtx, filename.c_str(), NULL, NULL)) != 0) {
        char errorstr[128];
        string error = "Unknown error";
        if (av_strerror(errnum, errorstr, 128) == 0) error = errorstr;
        throw EssentiaException("AudioLoader: Could not open file \"", filename, "\", error = ", error);
      }
      
      // Retrieve stream information
      if ((errnum = avformat_find_stream_info(_demuxCtx, NULL)) < 0) {
        char errorstr[128];
        string error = "Unknown error";
        if (av_strerror(errnum, errorstr, 128) == 0) error = errorstr;
        avformat_close_input(&_demuxCtx);
        _demuxCtx = 0;
        throw EssentiaException("AudioLoader: Could not find stream information, error = ", error);
      }
      
      // Dump information about file onto standard error
      //dump_format(_demuxCtx, 0, filename.c_str(), 0);
      
      // Check that we have only 1 audio stream in the file
      _streams.clear();
      for (int i=0; i<(int)_demuxCtx->nb_streams; i++) {
        if (_demuxCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
          _streams.push_back(i);
        }
      }
      int nAudioStreams = _streams.size();
      
      if (nAudioStreams == 0) {
        avformat_close_input(&_demuxCtx);
        _demuxCtx = 0;
        throw EssentiaException("AudioLoader ERROR: found 0 streams in the file, expecting one or more audio streams");
      }
      
      if (_selectedStream >= nAudioStreams) {
        avformat_close_input(&_demuxCtx);
        _demuxCtx = 0;
        throw EssentiaException("AudioLoader ERROR: 'audioStream' parameter set to ", _selectedStream ,". It should be smaller than the audio streams count, ", nAudioStreams);
      }
      
      _streamIdx = _streams[_selectedStream];
      
      // Load corresponding audio codec
      _audioCtx = _demuxCtx->streams[_streamIdx]->codec;
      _audioCodec = avcodec_find_decoder(_audioCtx->codec_id);
      
      if (!_audioCodec) {
        throw EssentiaException("AudioLoader: Unsupported codec!");
      }
      
      if (avcodec_open2(_audioCtx, _audioCodec, NULL) < 0) {
        throw EssentiaException("AudioLoader: Unable to instantiate codec...");
      }
      
      // Configure format convertion  (no samplerate conversion yet)
      int64_t layout = av_get_default_channel_layout(_audioCtx->channels);
      
      /*
       const char* fmt = 0;
       get_format_from_sample_fmt(&fmt, _audioCtx->sample_fmt);
       E_DEBUG(EAlgorithm, "AudioLoader: converting from " << (fmt ? fmt : "unknown") << " to FLT");
       */
      
      E_DEBUG(EAlgorithm, "AudioLoader: using sample format conversion from libavresample");
      
#ifndef TARGET_IOS
      _convertCtxAv = avresample_alloc();
#else
      _convertCtxAv = swr_alloc();
#endif
      
      av_opt_set_int(_convertCtxAv, "in_channel_layout", layout, 0);
      av_opt_set_int(_convertCtxAv, "out_channel_layout", layout, 0);
      av_opt_set_int(_convertCtxAv, "in_sample_rate", _audioCtx->sample_rate, 0);
      av_opt_set_int(_convertCtxAv, "out_sample_rate", _audioCtx->sample_rate, 0);
      av_opt_set_int(_convertCtxAv, "in_sample_fmt", _audioCtx->sample_fmt, 0);
      av_opt_set_int(_convertCtxAv, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
      
#ifndef TARGET_IOS
      if (avresample_open(_convertCtxAv) < 0) {
        throw EssentiaException("AudioLoader: Could not initialize avresample context");
      }
#else
  #if defined(USE_ACCELERATE)
#warning TODO
  #else
      swr_init(_convertCtxAv);
      
      if (swr_is_initialized(_convertCtxAv) == 0) {
        throw EssentiaException("AudioLoader: Could not initialize swresample context");
      }
  #endif
#endif
      av_init_packet(&_packet);
      
      _decodedFrame = av_frame_alloc();
      if (!_decodedFrame) {
        throw EssentiaException("AudioLoader: Could not allocate audio frame");
      }
      
      av_md5_init(_md5Encoded); // TODO
#endif
#warning TODO
    }
    
    
    void AudioLoader::closeAudioFile() {
#if defined(USE_ACCELERATE)
#else
      if (!_demuxCtx) {
        return;
      }
#endif
      
#ifndef TARGET_IOS
      if (_convertCtxAv) {
        avresample_close(_convertCtxAv);
        avresample_free(&_convertCtxAv);
      }
#else
  #if defined(USE_ACCELERATE)
  #else
      if (_convertCtxAv) {
        swr_close(_convertCtxAv);
        swr_free(&_convertCtxAv);
      }
  #endif
#endif
      
#if defined(USE_ACCELERATE)
      if ( _file != NULL ) {
        ExtAudioFileDispose( _file );
        _file = NULL;
      }
#else
      // Close the codec
      if (_audioCtx) avcodec_close(_audioCtx);
      // Close the audio file
      if (_demuxCtx) avformat_close_input(&_demuxCtx);
      
      // free AVPacket
      // TODO: use a variable for whether _packet is initialized or not
      av_free_packet(&_packet);
      _demuxCtx = 0;
      _audioCtx = 0;
      
      _streams.clear();
#endif
    }
    
    
    void AudioLoader::pushChannelsSampleRateInfo(int nChannels, Real sampleRate) {
      if (nChannels > 2) {
        throw EssentiaException("AudioLoader: could not load audio. Audio file has more than 2 channels.");
      }
      if (sampleRate <= 0) {
        throw EssentiaException("AudioLoader: could not load audio. Audio sampling rate must be greater than 0.");
      }
      
      _nChannels = nChannels;
      
      _channels.push(nChannels);
      _sampleRate.push(sampleRate);
    }
    
    
    void AudioLoader::pushCodecInfo(std::string codec, int bit_rate) {
      _codec.push(codec);
      _bit_rate.push(bit_rate);
    }
    
    
    string uint8_t_to_hex(uint8_t* input, int size) {
      ostringstream result;
      for(int i=0; i<size; ++i) {
        result << setw(2) << setfill('0') << hex << (int) input[i];
      }
      return result.str();
    }
    
    
    AlgorithmStatus AudioLoader::process() {
      if (!parameter("filename").isConfigured()) {
        throw EssentiaException("AudioLoader: Trying to call process() on an AudioLoader algo which hasn't been correctly configured.");
      }
      
#if defined(USE_ACCELERATE)
      auto audioFormat = ({
        AudioStreamBasicDescription audioDescription;
        memset(&audioDescription, 0, sizeof(audioDescription));
        audioDescription.mFormatID          = kAudioFormatLinearPCM;
        audioDescription.mFormatFlags       = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved;
        audioDescription.mChannelsPerFrame  = 2;
        audioDescription.mBytesPerPacket    = sizeof(float);
        audioDescription.mFramesPerPacket   = 1;
        audioDescription.mBytesPerFrame     = sizeof(float);
        audioDescription.mBitsPerChannel    = 8 * sizeof(float);
        audioDescription.mSampleRate        = 44100.0;
        audioDescription;
      });
      
      SInt64 frameCount = FFMPEG_BUFFER_SIZE / (audioFormat.mBytesPerFrame * audioFormat.mChannelsPerFrame);
//      UInt32 propertySize = sizeof(frameCount);
//      if ( ExtAudioFileGetProperty(_file, kExtAudioFileProperty_FileLengthFrames, &propertySize, &frameCount) != noErr ) {
//        throw EssentiaException("AudioLoader: Error getting file size");
//      }
      
      // Create temporary audio bufferlist
      int numberOfBuffers = audioFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved ? audioFormat.mChannelsPerFrame : 1;
      int channelsPerBuffer = audioFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved ? 1 : audioFormat.mChannelsPerFrame;
      int bytesPerBuffer = audioFormat.mBytesPerFrame * frameCount;
      
      AudioBufferList *bufferList = (AudioBufferList *)malloc(sizeof(AudioBufferList) + (numberOfBuffers-1)*sizeof(AudioBuffer));
      if ( !bufferList ) {
        throw EssentiaException("AudioLoader: Error creating AudioBufferList");
      }
      bufferList->mNumberBuffers = numberOfBuffers;
      for ( int i=0; i<numberOfBuffers; i++ ) {
        if ( bytesPerBuffer > 0 ) {
          bufferList->mBuffers[i].mData = calloc(bytesPerBuffer, 1);
          if ( !bufferList->mBuffers[i].mData ) {
            for ( int j=0; j<i; j++ ) free(bufferList->mBuffers[j].mData);
            free(bufferList);
            throw EssentiaException("AudioLoader: Error allocating data for AudioBufferList");
          }
        } else {
          bufferList->mBuffers[i].mData = NULL;
        }
        bufferList->mBuffers[i].mDataByteSize = bytesPerBuffer;
        bufferList->mBuffers[i].mNumberChannels = channelsPerBuffer;
      }
      
      // Set destination format
      if ( ExtAudioFileSetProperty(_file, kExtAudioFileProperty_ClientDataFormat, sizeof(audioFormat), &audioFormat) != noErr ) {
        throw EssentiaException("AudioLoader: Error setting client data format");
      }
      
      // Read audio
      // TODO: Read in chunks instead of whole file at once
      UInt32 readFrames = frameCount;
      if ( ExtAudioFileRead(_file, &readFrames, bufferList) != noErr ) {
        throw EssentiaException("AudioLoader: Error reading file" );
      }
      // assert( readFrames == frameCount && "Read frames don't match file size" );
      bool eof = readFrames < frameCount;
      if ( eof ) { // EOF
        shouldStop(true);
      }
      
      int nsamples = readFrames;
      
      // acquire necessary data
      bool ok = _audio.acquire(nsamples);
      if (!ok) {
        throw EssentiaException("AudioLoader: could not acquire output for audio");
      }
      
      vector<StereoSample>& audio = *((vector<StereoSample>*)_audio.getTokens());
      
      if (_nChannels == 1) {
        for (int i=0; i<nsamples; i++) {
          audio[i].left() = ((float *)bufferList->mBuffers[0].mData)[i];
          //audio[i].left() = scale(_buffer[i]);
        }
      }
      else { // _nChannels == 2
        // The output format is always AV_SAMPLE_FMT_FLT, which is interleaved
        for (int i=0; i<nsamples; i++) {
          audio[i].left() = ((float *)bufferList->mBuffers[0].mData)[i];
          audio[i].right() = ((float *)bufferList->mBuffers[1].mData)[i];
          //audio[i].left() = scale(_buffer[2*i]);
          //audio[i].right() = scale(_buffer[2*i+1]);
        }
        /*
         // planar
         for (int i=0; i<nsamples; i++) {
         audio[i].left() = scale(_buffer[i]);
         audio[i].right() = scale(_buffer[nsamples+i]);
         }
         */
      }
      
      // release data
      _audio.release(nsamples);
      
      // Free bufferlist
      for ( int i=0; i<bufferList->mNumberBuffers; i++ ) {
        if ( bufferList->mBuffers[i].mData ) free(bufferList->mBuffers[i].mData);
      }
      free(bufferList);
      
      if ( eof ) {
        closeAudioFile();
      }
#else
      // read frames until we get a good one
      do {
        int result = av_read_frame(_demuxCtx, &_packet);
        //E_DEBUG(EAlgorithm, "AudioLoader: called av_read_frame(), got result = " << result);
        if (result != 0) {
          // 0 = OK, < 0 = error or EOF
          if (result != AVERROR_EOF) {
            char errstring[1204];
            av_strerror(result, errstring, sizeof(errstring));
            ostringstream msg;
            msg << "AudioLoader: Error reading frame: " << errstring;
            E_WARNING(msg.str());
          }
          // TODO: should try reading again on EAGAIN error?
          //       https://github.com/FFmpeg/FFmpeg/blob/master/ffmpeg.c
          shouldStop(true);
          flushPacket();
          closeAudioFile();
          if (_computeMD5) {
            av_md5_final(_md5Encoded, _checksum);
            _md5.push(uint8_t_to_hex(_checksum, 16));
          }
          else {
            string md5 = "";
            _md5.push(md5);
          }
          return FINISHED;
        }
      } while (_packet.stream_index != _streamIdx);
      
      // compute md5 first
      if (_computeMD5) {
        av_md5_update(_md5Encoded, _packet.data, _packet.size);
      }
      
      // decode frames in packet
      while(_packet.size > 0) {
        if (!decodePacket()) break;
        copyFFmpegOutput();
      }
      // neds to be freed !!
      av_free_packet(&_packet);
      
#endif
      return OK;
    }
    
    
#if defined(USE_ACCELERATE)
#warning TODO
#else
    int AudioLoader::decode_audio_frame(AVCodecContext* audioCtx,
                                        float* output,
                                        int* outputSize,
                                        AVPacket* packet) {
      
      // _dataSize  input = number of bytes available for write in buff
      //           output = number of bytes actually written (actual: FLT data)
      //E_DEBUG(EAlgorithm, "decode_audio_frame, available bytes in buffer = " << _dataSize);
      int gotFrame = 0;
      av_frame_unref(_decodedFrame); //avcodec_get_frame_defaults(_decodedFrame);
      
      int len = avcodec_decode_audio4(audioCtx, _decodedFrame, &gotFrame, packet);
      
      if (len < 0) return len; // error handling should be done outside
      
      if (gotFrame) {
        int inputSamples = _decodedFrame->nb_samples;
        int inputPlaneSize = av_samples_get_buffer_size(NULL, _nChannels, inputSamples,
                                                        audioCtx->sample_fmt, 1);
        int outputPlaneSize = av_samples_get_buffer_size(NULL, _nChannels, inputSamples,
                                                         AV_SAMPLE_FMT_FLT, 1);
        // the size of the output buffer in samples
        int outputBufferSamples = *outputSize /
        (av_get_bytes_per_sample(AV_SAMPLE_FMT_FLT) * _nChannels);
        
        if (outputBufferSamples < inputSamples) {
          // this should never happen, throw exception here
          throw EssentiaException("AudioLoader: Insufficient buffer size for format conversion");
        }
        
        if (audioCtx->sample_fmt == AV_SAMPLE_FMT_FLT) {
          // TODO: no need in this check? Not many of common formats support FLT
          // no conversion needed, direct copy from our frame to output buffer
          memcpy(output, _decodedFrame->data[0], inputPlaneSize);
        }
        else {
#ifndef TARGET_IOS
          int samplesWrittern = avresample_convert(_convertCtxAv,
                                                   (uint8_t**) &output,
                                                   outputPlaneSize,
                                                   outputBufferSamples,
                                                   (uint8_t**)_decodedFrame->data,
                                                   inputPlaneSize,
                                                   inputSamples);
#else
          int samplesWrittern = swr_convert(_convertCtxAv,
                                            (uint8_t**)&output,
                                            outputBufferSamples,
                                            (const uint8_t**)_decodedFrame->data,
                                            inputSamples);
#endif
          if (samplesWrittern < inputSamples) {
            // TODO: there may be data remaining in the internal FIFO buffer
            // to get this data: call avresample_convert() with NULL input
            // Test if this happens in practice
            ostringstream msg;
            msg << "AudioLoader: Incomplete format conversion (some samples missing)"
            << " from " << av_get_sample_fmt_name(_audioCtx->sample_fmt)
            << " to "   << av_get_sample_fmt_name(AV_SAMPLE_FMT_FLT);
            throw EssentiaException(msg);
          }
        }
        *outputSize = outputPlaneSize;
      }
      else {
        E_DEBUG(EAlgorithm, "AudioLoader: tried to decode packet but didn't get any frame...");
        *outputSize = 0;
      }
      
      return len;
    }
#endif
    
    
    void AudioLoader::flushPacket() {
#if defined(USE_ACCELERATE)
#warning TODO
#else
      AVPacket empty;
      av_init_packet(&empty);
      do {
        _dataSize = FFMPEG_BUFFER_SIZE;
        empty.data = NULL;
        empty.size = 0;
        
        int len = decode_audio_frame(_audioCtx, _buffer, &_dataSize, &empty);
        if (len < 0) {
          char errstring[1204];
          av_strerror(len, errstring, sizeof(errstring));
          ostringstream msg;
          msg << "AudioLoader: decoding error while flushing a packet:" << errstring;
          E_WARNING(msg.str());
        }
        copyFFmpegOutput();
        
      } while (_dataSize > 0);
#endif
    }
    
    
    /**
     * Gets the AVPacket stored in _packet, and decodes all the samples it can from it,
     * putting them in _buffer, the total number of bytes written begin stored in _dataSize.
     */
    int AudioLoader::decodePacket() {
      /*
       E_DEBUG(EAlgorithm, "-----------------------------------------------------");
       E_DEBUG(EAlgorithm, "decoding packet of " << _packet.size << " bytes");
       E_DEBUG(EAlgorithm, "pts: " << _packet.pts << " - dts: " << _packet.dts); //" - pos: " << pkt->pos);
       E_DEBUG(EAlgorithm, "flags: " << _packet.flags);
       E_DEBUG(EAlgorithm, "duration: " << _packet.duration);
       */
      int len = 0;
#if defined(USE_ACCELERATE)
#warning TODO
#else
      
      // buff is an offset in our output buffer, it points to where we should start
      // writing the next decoded samples
      float* buff = _buffer;
      
      // _dataSize gets the size of the buffer, in bytes
      _dataSize = FFMPEG_BUFFER_SIZE;
      
      // Note: md5 should be computed before decoding frame, as the decoding may
      // change the content of a packet. Still, not sure if it is correct to
      // compute md5 over packet which contains incorrect frames, potentially
      // belonging to id3 metadata (TODO: or is it just a missing header issue?),
      // but computing md5 hash using ffmpeg will also treat it as audio:
      //      ffmpeg -i file.mp3 -acodec copy -f md5 -
      
      len = decode_audio_frame(_audioCtx, buff, &_dataSize, &_packet);
      
      if (len < 0) {
        char errstring[1204];
        av_strerror(len, errstring, sizeof(errstring));
        ostringstream msg;
        
        if (_audioCtx->codec_id == AV_CODEC_ID_MP3) {
          msg << "AudioLoader: invalid frame, skipping it: " << errstring;
          // mp3 streams can have tag frames (id3v2?) which libavcodec tries to
          // read as audio anyway, and we probably don't want print an error
          // message for that...
          // TODO: Are these frames really id3 tags?
          
          //E_DEBUG(EAlgorithm, msg);
          E_WARNING(msg.str());
        }
        else {
          msg << "AudioLoader: error while decoding, skipping frame: " << errstring;
          E_WARNING(msg.str());
        }
        return 0;
      }
      
      if (len != _packet.size) {
        // https://www.ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga834bb1b062fbcc2de4cf7fb93f154a3e
        
        // Some decoders may support multiple frames in a single AVPacket. Such
        // decoders would then just decode the first frame and the return value
        // would be less than the packet size. In this case, avcodec_decode_audio4
        // has to be called again with an AVPacket containing the remaining data
        // in order to decode the second frame, etc... Even if no frames are
        // returned, the packet needs to be fed to the decoder with remaining
        // data until it is completely consumed or an error occurs.
        
        E_WARNING("AudioLoader: more than 1 frame in packet, decoding remaining bytes...");
        E_WARNING("at sample index: " << output("audio").totalProduced());
        E_WARNING("decoded samples: " << len);
        E_WARNING("packet size: " << _packet.size);
      }
      
      // update packet data pointer to data left undecoded (if any)
      _packet.size -= len;
      _packet.data += len;
      
      if (_dataSize <= 0) {
        // No data yet, get more frames
        // cout << "no data yet, get more frames" << endl;
        _dataSize = 0;
      }
#endif
      
      return len;
    }
    
    /*
     inline Real scale(int16_t value) {
     return value / (Real)32767;
     }
     */
    
    void AudioLoader::copyFFmpegOutput() {
#if defined(USE_ACCELERATE)
#else
      int nsamples = _dataSize / (av_get_bytes_per_sample(AV_SAMPLE_FMT_FLT)  * _nChannels);

      if (nsamples == 0) return;
      
      // acquire necessary data
      bool ok = _audio.acquire(nsamples);
      if (!ok) {
        throw EssentiaException("AudioLoader: could not acquire output for audio");
      }
      
      vector<StereoSample>& audio = *((vector<StereoSample>*)_audio.getTokens());
      
      if (_nChannels == 1) {
        for (int i=0; i<nsamples; i++) {
          audio[i].left() = _buffer[i];
          //audio[i].left() = scale(_buffer[i]);
        }
      }
      else { // _nChannels == 2
        // The output format is always AV_SAMPLE_FMT_FLT, which is interleaved
        for (int i=0; i<nsamples; i++) {
          audio[i].left() = _buffer[2*i];
          audio[i].right() = _buffer[2*i+1];
          //audio[i].left() = scale(_buffer[2*i]);
          //audio[i].right() = scale(_buffer[2*i+1]);
        }
        /*
         // planar
         for (int i=0; i<nsamples; i++) {
         audio[i].left() = scale(_buffer[i]);
         audio[i].right() = scale(_buffer[nsamples+i]);
         }
         */
      }
      
      // release data
      _audio.release(nsamples);
#endif
    }
    
    void AudioLoader::reset() {
      Algorithm::reset();
      
      if (!parameter("filename").isConfigured()) return;
      
      string filename = parameter("filename").toString();
      
      closeAudioFile();
      openAudioFile(filename);
      
#if defined(USE_ACCELERATE)
      AudioStreamBasicDescription asbd;
      UInt32 propertySize = sizeof(asbd);
      if ( ExtAudioFileGetProperty(_file, kExtAudioFileProperty_FileDataFormat, &propertySize, &asbd) != noErr ) {
        throw EssentiaException("AudioLoader: Error getting audio file channel and sample rate info");
      }
      pushChannelsSampleRateInfo(asbd.mChannelsPerFrame, asbd.mSampleRate);
      
      OSStatus status = noErr;
      AudioFileID audioFileId = NULL;
      
      UInt32 size = sizeof(audioFileId);
      status = ExtAudioFileGetProperty(_file, kExtAudioFileProperty_AudioFile, &size, &audioFileId);
      assert(status == noErr);
      
      UInt32 bitRate = 0;
      size = sizeof(bitRate);
      if ( AudioFileGetProperty(audioFileId, kAudioFilePropertyBitRate, &size, &bitRate) != noErr ) {
        throw EssentiaException("AudioLoader: Error getting audio file bitRate");
      }
      
      E_DEBUG(EAlgorithm, "AudioLoader: TODO: get audio codec");
      pushCodecInfo("pcm_s16le", bitRate);
#else
      pushChannelsSampleRateInfo(_audioCtx->channels, _audioCtx->sample_rate);
      pushCodecInfo(_audioCodec->name, _audioCtx->bit_rate);
#endif
    }
    
  } // namespace streaming
} // namespace essentia


namespace essentia {
  namespace standard {
    
    const char* AudioLoader::name = "AudioLoader";
    const char* AudioLoader::category = "Input/output";
    const char* AudioLoader::description = DOC("This algorithm loads the single audio stream contained in a given audio or video file. Supported formats are all those supported by the FFmpeg library including wav, aiff, flac, ogg and mp3.\n"
                                               "\n"
                                               "This algorithm will throw an exception if it was not properly configured which is normally due to not specifying a valid filename. Invalid names comprise those with extensions different than the supported  formats and non existent files. If using this algorithm on Windows, you must ensure that the filename is encoded as UTF-8\n\n"
                                               "Note: ogg files are decoded in reverse phase, due to be using ffmpeg library.\n"
                                               "\n"
                                               "References:\n"
                                               "  [1] WAV - Wikipedia, the free encyclopedia,\n"
                                               "      http://en.wikipedia.org/wiki/Wav\n"
                                               "  [2] Audio Interchange File Format - Wikipedia, the free encyclopedia,\n"
                                               "      http://en.wikipedia.org/wiki/Aiff\n"
                                               "  [3] Free Lossless Audio Codec - Wikipedia, the free encyclopedia,\n"
                                               "      http://en.wikipedia.org/wiki/Flac\n"
                                               "  [4] Vorbis - Wikipedia, the free encyclopedia,\n"
                                               "      http://en.wikipedia.org/wiki/Vorbis\n"
                                               "  [5] MP3 - Wikipedia, the free encyclopedia,\n"
                                               "      http://en.wikipedia.org/wiki/Mp3");
    
    
    void AudioLoader::createInnerNetwork() {
      _loader = streaming::AlgorithmFactory::create("AudioLoader");
      _audioStorage = new streaming::VectorOutput<StereoSample>();
      
      _loader->output("audio")           >>  _audioStorage->input("data");
      _loader->output("sampleRate")      >>  PC(_pool, "internal.sampleRate");
      _loader->output("numberChannels")  >>  PC(_pool, "internal.numberChannels");
      _loader->output("md5")             >>  PC(_pool, "internal.md5");
      _loader->output("codec")           >>  PC(_pool, "internal.codec");
      _loader->output("bit_rate")        >>  PC(_pool, "internal.bit_rate");
      _network = new scheduler::Network(_loader);
    }
    
    void AudioLoader::configure() {
      _loader->configure(INHERIT("filename"),
                         INHERIT("computeMD5"),
                         INHERIT("audioStream"));
    }
    
    void AudioLoader::compute() {
      if (!parameter("filename").isConfigured()) {
        throw EssentiaException("AudioLoader: Trying to call compute() on an "
                                "AudioLoader algo which hasn't been correctly configured.");
      }
      
      Real& sampleRate = _sampleRate.get();
      int& numberChannels = _channels.get();
      string& md5 = _md5.get();
      int& bit_rate = _bit_rate.get();
      string& codec = _codec.get();
      vector<StereoSample>& audio = _audio.get();
      
      _audioStorage->setVector(&audio);
      // TODO: is using VectorInput indeed faster than using Pool?
      
      // FIXME:
      // _audio.reserve(sth_meaningful);
      
      _network->run();
      
      sampleRate = _pool.value<Real>("internal.sampleRate");
      numberChannels = (int) _pool.value<Real>("internal.numberChannels");
      md5 = _pool.value<std::string>("internal.md5");
      bit_rate = (int) _pool.value<Real>("internal.bit_rate");
      codec = _pool.value<std::string>("internal.codec");
      
      // reset, so it is ready to load audio again
      reset();
    }
    
    void AudioLoader::reset() {
      _network->reset();
      _pool.remove("internal.md5");
      _pool.remove("internal.sampleRate");
      _pool.remove("internal.numberChannels");
      _pool.remove("internal.codec");
      _pool.remove("internal.bit_rate");
    }
    
  } // namespace standard
} // namespace essentia
