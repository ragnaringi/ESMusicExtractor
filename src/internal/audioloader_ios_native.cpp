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


#include <fstream>
#include <iomanip>  //  setw()
#include <CommonCrypto/CommonDigest.h>

#include "audioloader_ios.h"
#include "algorithmfactory.h"

using namespace std;

static std::vector<char> readAllBytes(char const* filename) {
  ifstream ifs(filename, ios::binary|ios::ate);
  ifstream::pos_type pos = ifs.tellg();
  
  vector<char> result(pos);
  
  ifs.seekg(0, ios::beg);
  ifs.read(result.data(), pos);
  
  string header(result.begin(), result.begin() + 4);
  
  if (header == "RIFF") { // WAVE file
    return vector<char>( result.begin() + 44, result.end() );
  } else if (header == "FORM") {
    return result;
  }
  // TODO: Handle other formats
  return result;
}

string uint8_t_to_hex(uint8_t* input, int size) {
  ostringstream result;
  for(int i=0; i<size; ++i) {
    result << setw(2) << setfill('0') << hex << (int) input[i];
  }
  return result.str();
}

namespace essentia { namespace streaming {
    
  const char* AudioLoader::name = essentia::standard::AudioLoader::name;
  const char* AudioLoader::category = essentia::standard::AudioLoader::category;
  const char* AudioLoader::description = essentia::standard::AudioLoader::description;
  
  AudioLoader::~AudioLoader() {
    closeAudioFile();
  }
  
  void AudioLoader::configure() {
    _computeMD5 = parameter("computeMD5").toBool();
    // _selectedStream = parameter("audioStream").toInt();
    reset();
  }
  
  void AudioLoader::openAudioFile(const string& filename) {
    E_DEBUG(EAlgorithm, "AudioLoader: opening file: " << filename);

    auto path =
    CFStringCreateWithCString(kCFAllocatorDefault, filename.c_str(),kCFStringEncodingUTF8);
    auto url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, path, kCFURLPOSIXPathStyle, false);
    OSStatus result = ExtAudioFileOpenURL(url, &_file);
    if ( result != noErr ) {
      throw EssentiaException("AudioLoader: Could not open file \"", filename, "\", error = ", result);
    }
    
    if ( !CC_MD5_Init(&hashObject) ) {
      throw EssentiaException("Error allocating the MD5 context");
    }

    auto fileData = readAllBytes(filename.c_str());
    for ( int i = 0; i < 100; i++ ) {
      std::cout << i << " : " << fileData[i] << std::endl;
    }

    // compute md5 first
    if (_computeMD5) {
      size_t chunkSize = 4096;
      for ( size_t i = 0; i < fileData.size(); i += chunkSize ) {
        auto len = std::min(fileData.size()-i, chunkSize);
        CC_MD5_Update(&hashObject,
                      fileData.data()+i,
                      (CC_LONG)len);
      }
      
      // Compute the hash digest
      unsigned char digest[CC_MD5_DIGEST_LENGTH];
      CC_MD5_Final(digest, &hashObject);
      _md5.push(uint8_t_to_hex(digest, 16));
    
    } else {
      string md5 = "";
      _md5.push(md5);
    }
    
  }
  
  void AudioLoader::closeAudioFile() {
    if ( _file != NULL ) {
      ExtAudioFileDispose( _file );
      _file = NULL;
    }
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
  
  AlgorithmStatus AudioLoader::process() {
    if (!parameter("filename").isConfigured()) {
      throw EssentiaException("AudioLoader: Trying to call process() on an AudioLoader algo which hasn't been correctly configured.");
    }
    
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
    
    const SInt64 frameCount = 4096;
    
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
    UInt32 readFrames = frameCount;
    if ( ExtAudioFileRead(_file, &readFrames, bufferList) != noErr ) {
      throw EssentiaException("AudioLoader: Error reading file" );
    }

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
      }
    }
    else { // _nChannels == 2
      for (int i=0; i<nsamples; i++) {
        audio[i].left() = ((float *)bufferList->mBuffers[0].mData)[i];
        audio[i].right() = ((float *)bufferList->mBuffers[1].mData)[i];
      }
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

    return OK;
  }
  
  void AudioLoader::reset() {
    Algorithm::reset();
    
    if (!parameter("filename").isConfigured()) return;
    
    string filename = parameter("filename").toString();
    
    closeAudioFile();
    openAudioFile(filename);
    
    // Get audio file data format
    AudioStreamBasicDescription asbd;
    UInt32 propertySize = sizeof(asbd);
    if ( ExtAudioFileGetProperty(_file, kExtAudioFileProperty_FileDataFormat, &propertySize, &asbd) != noErr ) {
      throw EssentiaException("AudioLoader: Error getting audio file channel and sample rate info");
    }
    pushChannelsSampleRateInfo(asbd.mChannelsPerFrame, asbd.mSampleRate);
    
    // Get bit rate
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
