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
// https://codereview.stackexchange.com/a/22907
static std::vector<char> readAllBytes(char const* filename) {
  ifstream ifs(filename, ios::binary|ios::ate);
  ifstream::pos_type pos = ifs.tellg();
  
  vector<char> result(pos);
  
  ifs.seekg(0, ios::beg);
  ifs.read(result.data(), pos);
  
  auto header = string(result.begin(), result.begin() + 4);
  if ( header == "RIFF" ) { // WAVE file
    return vector<char>( result.begin() + 44, result.end() );
  } else if ( header == "FORM" ) { // AIFF
    for ( int i = 4; i < result.size(); i++ ) {
      auto str = string(result.begin()+i, result.begin()+i+4);
      if ( str == "SSND" ) {
        return vector<char>( result.begin()+i+16, result.end() );
      }
    }
  } else {
    auto str = string(result.begin(), result.begin()+11);
    if ( str.substr(0,3) == "ID3" ) { // mp3
      // Extract ID3 tag size from synchsafe integer
      // https://stackoverflow.com/a/5652842
      uint8_t* sync_safe = (uint8_t *)result.data()+6;
      uint32_t byte0 = sync_safe[0];
      uint32_t byte1 = sync_safe[1];
      uint32_t byte2 = sync_safe[2];
      uint32_t byte3 = sync_safe[3];
      int len =  byte0 << 21 | byte1 << 14 | byte2 << 7 | byte3;
      int tagSize = len + 10; //
      return vector<char>( result.begin()+tagSize, result.end() );
    } else if ( str.substr(4, 11) == "ftypM4A" ) { // m4a
      for ( int i = 0; i < result.size()-4; i++ ) {
        if ( result[i] == '!' && result[i+3] == '@' && result[i+4] == 'h' ) {
          return vector<char>( result.begin()+i, result.end() );
        }
      }
    } else {
      cout << "File format not supported" << endl;
    }
  }
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

    auto fileData = readAllBytes(filename.c_str());
//    for ( int i = 0; i < 100; i++ ) {
//      std::cout << i << " : " << fileData[i] << std::endl;
//    }

    // compute md5 first
    if ( parameter("computeMD5").toBool() ) {
      CC_MD5_CTX hashObject;
      
      if ( !CC_MD5_Init(&hashObject) ) {
        throw EssentiaException("Error allocating the MD5 context");
      }
      
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
    
    _channels.push(nChannels);
    _sampleRate.push(sampleRate);
  }
  
  void AudioLoader::pushCodecInfo(std::string codec, int bit_rate) {
    _codec.push(codec);
    _bit_rate.push(bit_rate);
  }
  
  AlgorithmStatus AudioLoader::process() {
    if ( !parameter("filename").isConfigured() ) {
      throw EssentiaException("AudioLoader: Trying to call process() on an AudioLoader algo which hasn't been correctly configured.");
    }
    
    AudioStreamBasicDescription audioFormat = {0};
    audioFormat.mFormatID          = kAudioFormatLinearPCM;
    audioFormat.mFormatFlags       = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved;
    audioFormat.mChannelsPerFrame  = lastTokenProduced<int>(_channels);
    audioFormat.mBytesPerPacket    = sizeof(float);
    audioFormat.mFramesPerPacket   = 1;
    audioFormat.mBytesPerFrame     = sizeof(float);
    audioFormat.mBitsPerChannel    = 8 * sizeof(float);
    audioFormat.mSampleRate        = 44100.0;
    
    const SInt64 frameCount = 4096;
    const UInt32 channels = audioFormat.mChannelsPerFrame;
    const UInt32 bytesPerBuffer = audioFormat.mBytesPerFrame * frameCount;
    
    // Create temporary audio bufferlist
    auto bufferList = (AudioBufferList *)malloc(sizeof(AudioBufferList) * channels);
    bufferList->mNumberBuffers = channels;
    for ( auto i = 0; i < bufferList->mNumberBuffers; i++ ) {
      bufferList->mBuffers[i].mNumberChannels = 1;
      bufferList->mBuffers[i].mDataByteSize = frameCount * audioFormat.mBytesPerFrame;
      bufferList->mBuffers[i].mData = calloc(bytesPerBuffer, 1);
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
    
    // Acquire necessary data
    if ( !_audio.acquire(readFrames) ) {
      throw EssentiaException("AudioLoader: could not acquire output for audio");
    }
    
    auto& audio = *((vector<StereoSample>*)_audio.getTokens());
    
    for ( auto i = 0; i < readFrames; i++ ) {
      switch (channels) {
        case 2:
          audio[i].right() = ((float *)bufferList->mBuffers[1].mData)[i];
        default:
          audio[i].left()  = ((float *)bufferList->mBuffers[0].mData)[i];
      }
    }
    
    _audio.release(readFrames);
    
    // Free bufferlist
    for ( auto i = 0; i < bufferList->mNumberBuffers; i++ ) {
      free(bufferList->mBuffers[i].mData);
    }
    free(bufferList);
    
    if ( readFrames < frameCount /* EOF */ ) {
      shouldStop(true);
      
      return FINISHED;
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
