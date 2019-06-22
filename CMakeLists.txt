cmake_minimum_required(VERSION 3.8)

project(RtAudioExample VERSION 1.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_executable(RtAudioExample ${CMAKE_CURRENT_SOURCE_DIR}/src/RtAudioExample
                              ${CMAKE_CURRENT_SOURCE_DIR}/src/wav_header)

target_compile_options(RtAudioExample
                       PRIVATE -fPIC
                               -O3
                               $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a>)     

find_package(rtaudio)
target_link_libraries(RtAudioExample
                      PRIVATE rtaudio
                              stdc++fs)