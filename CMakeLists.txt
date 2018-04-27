cmake_minimum_required(VERSION 3.9)
project(Tilemap C)

add_definitions(-DDEBUG)

set(plataform "x32")
#set(plataform "x64")

### DEPS
set(deps_dir "${CMAKE_SOURCE_DIR}/deps")
set(deps_include_dir "${CMAKE_SOURCE_DIR}/deps/include")
set(deps_bin_dir "${CMAKE_SOURCE_DIR}/deps/bin/${plataform}")
set(deps_lib_dir "${CMAKE_SOURCE_DIR}/deps/lib/${plataform}")

### LIBS
set(libs "-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer")

include_directories("include;${deps_include_dir}")
set(SOURCE_FILES main.c)
add_executable(Tilemap ${SOURCE_FILES})
target_link_libraries(Tilemap ${libs})

file(COPY ${deps_bin_dir}/ DESTINATION .)
file(COPY content DESTINATION .)