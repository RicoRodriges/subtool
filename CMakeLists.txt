cmake_minimum_required(VERSION 3.10.2)
project(subtitles C)

set(CMAKE_C_STANDARD 11)
add_definitions(-D_GNU_SOURCE -O3)
#add_definitions(-D_GNU_SOURCE -O0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../public)

add_executable(subtitles
        cpp/main.c
        cpp/subtitle.c
        cpp/demux.c
        cpp/libass/libass/ass.c
        cpp/libass/libass/ass_library.c
        cpp/libass/libass/ass_utils.c
        cpp/libass/libass/ass_string.c
        cpp/libass/libass/ass_strtod.c)
target_include_directories(subtitles PUBLIC
        cpp
        cpp/libass)
# add empty config.h file
set_target_properties(subtitles PROPERTIES LINK_FLAGS " \
         -s WASM=1 \
         -s INITIAL_MEMORY=536870912 \
         -s EXPORTED_RUNTIME_METHODS='[lengthBytesUTF8,stringToUTF8]' \
         -s EXPORTED_FUNCTIONS='[_parse,_malloc,_free]' \
         -sMODULARIZE -s EXPORT_NAME='createMyModule' \
         -s FILESYSTEM=0")
