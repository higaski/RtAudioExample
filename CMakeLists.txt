cmake_minimum_required(VERSION 3.11)
include(FetchContent)

project(
  RtAudioExample
  VERSION 1.1
  LANGUAGES C CXX)

add_executable(RtAudioExample ${CMAKE_CURRENT_SOURCE_DIR}/src/RtAudioExample
                              ${CMAKE_CURRENT_SOURCE_DIR}/src/wav_header)

target_compile_options(RtAudioExample
                       PRIVATE -fPIC -O3 $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a>)

message("Cloning rtaudio")
FetchContent_Declare(rtaudio
                     GIT_REPOSITORY https://github.com/thestk/rtaudio.git)
FetchContent_MakeAvailable(rtaudio)

# Dirty
target_include_directories(RtAudioExample PRIVATE ${rtaudio_SOURCE_DIR})

target_link_libraries(RtAudioExample PRIVATE rtaudio stdc++fs)
