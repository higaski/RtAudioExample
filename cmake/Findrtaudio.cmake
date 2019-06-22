execute_process(COMMAND git submodule update --init  --recursive  --
                                                                  external/rtaudio
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_subdirectory(${PROJECT_SOURCE_DIR}/external/rtaudio
                 ${PROJECT_BINARY_DIR}/external/rtaudio)
