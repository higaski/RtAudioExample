#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

// RtAudio installs its headers to CMAKE_INSTALL_INCLUDEDIR...
#include "RtAudio.h"

#include "wav_header.hpp"

/// Length of data
unsigned int data_length{};

/// Raw wav data
std::vector<uint8_t> raw{};

/// Pointer to raw wav data
uint8_t* in;

/// Read wave data into global variables
///
/// \param  path    Filepath
WavHeader read_wav_data(char const* path) {
  using namespace std;
  namespace fs = filesystem;

  WavHeader wav_header{};

  if (auto p{fs::current_path() /= path}; fs::exists(p)) {
    if (ifstream wav{p, ios::binary}; wav.is_open()) {
      array<uint8_t, 1024> buf{};
      wav.read(reinterpret_cast<char*>(begin(buf)), 1024);
      wav_header = encode_wav_header(begin(buf));
      data_length = wav_header.data_size;
      raw.reserve(wav_header.data_size);
      wav.seekg(wav_header.data_offset);
      wav.read(reinterpret_cast<char*>(&raw[0]), wav_header.data_size);
      in = &raw[0];

    } else {
      cout << "can't open file\n";
      exit(0);
    }
  } else {
    cout << "file not found\n";
    exit(0);
  }

  return wav_header;
}

/// RtAudio callback
///
/// \param  outputBuffer    Destination
/// \param  inputBuffer     Source
/// \param  nFrames         Samples per buffer
/// \param  streamTime      Unused
/// \param  status          Unused
/// \param  userData        Wav header
/// \return 0               Continue
/// \return 1               Stop stream, drain output
/// \return 2               Abort immediately
static int rtCallback(void* outputBuffer,
                      void* inputBuffer,
                      unsigned int nFrames,
                      double streamTime,
                      RtAudioStreamStatus status,
                      void* userData) {
  WavHeader* wav_header{static_cast<WavHeader*>(userData)};

  unsigned int n{data_length >= nFrames ? nFrames : data_length};

  // 8 bit
  if (wav_header->bit_depth == 8) {
    uint8_t* out{static_cast<uint8_t*>(outputBuffer)};

    // Assuming 8 bit is always mono
    for (auto i{0ul}; i < n; ++i) {
      *out++ = (*in++) - 128;  // RtAudio does not support uint8_t...
      data_length--;
    }
    // 16 bit
  } else if (wav_header->bit_depth == 16) {
    int16_t* out{static_cast<int16_t*>(outputBuffer)};

    // THIS IS UB! DONT do that in production
    int16_t* in_cpy{reinterpret_cast<int16_t*>(in)};

    // Assuming 16 bit is stereo
    for (auto i{0ul}; i < n; ++i) {
      *out++ = *in_cpy++;
      *out++ = *in_cpy++;
      data_length -= 4;
      in += 4;
    }
  }

  return n == nFrames ? 0 : 1;
}

/// Main
///
/// \param  argv    Filepath
/// \return Exit
int main(int argc, char* argv[]) {
  using namespace std;

  // Not enough arguments
  if (argc < 2) {
    cout << "useage: PortAudioExample <path>\n";
    exit(0);
  }

  // Read .wav header
  auto wav_header{read_wav_data(argv[1])};

  // Create RtAudio instance
  RtAudio audio;

  // Check if audio device is available
  if (audio.getDeviceCount() < 1) {
    std::cout << "\nNo audio devices found!\n";
    exit(1);
  }

  // Let RtAudio print messages to stderr
  audio.showWarnings(true);

  // Set stream parameters
  unsigned int bufferFrames{256};
  RtAudio::StreamParameters oParams;
  oParams.deviceId = audio.getDefaultOutputDevice();
  oParams.nChannels = wav_header.channels;
  oParams.firstChannel = 0;

  RtAudio::StreamOptions options;
  options.flags |= RTAUDIO_SCHEDULE_REALTIME;

  try {
    // Open stream
    audio.openStream(&oParams,
                     NULL,  // No input
                     wav_header.bit_depth == 8 ? RTAUDIO_SINT8 : RTAUDIO_SINT16,
                     wav_header.sample_rate,
                     &bufferFrames,
                     &rtCallback,
                     &wav_header,
                     &options,
                     NULL);  // No error callback

    // Start stream
    audio.startStream();
  } catch (RtAudioError& e) {
    e.printMessage();
    goto cleanup;
  }

  while (data_length)
    asm volatile("nop");

cleanup:
  if (audio.isStreamOpen())
    audio.closeStream();

  return 0;
}
