cmake_minimum_required(VERSION 3.10.2)
project(subtitles C)

set(CMAKE_C_STANDARD 11)
add_definitions(-D_GNU_SOURCE -O3)
#add_definitions(-D_GNU_SOURCE -O0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../public)

add_executable(subtitles
        cpp/main.c
        cpp/subtitle.c
        cpp/libass-0.14.0/libass/ass.c
        cpp/libass-0.14.0/libass/ass_library.c
        cpp/libass-0.14.0/libass/ass_utils.c
        cpp/libass-0.14.0/libass/ass_string.c
        cpp/libass-0.14.0/libass/ass_strtod.c)
target_include_directories(subtitles PUBLIC
        cpp # need for config.h file
        cpp/libass-0.14.0)
# add empty config.h file
set_target_properties(subtitles PROPERTIES LINK_FLAGS " \
         -s WASM=1 \
         -s BINARYEN_METHOD='native-wasm' \
         -s TOTAL_MEMORY=536870912 \
         -s EXTRA_EXPORTED_RUNTIME_METHODS='[ccall]' \
         -s EXPORTED_FUNCTIONS='[_parse]' \
         -s FILESYSTEM=0")
