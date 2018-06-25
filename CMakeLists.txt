cmake_minimum_required(VERSION 3.9)
project(Gumo_Editor C)

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

include_directories("src;${deps_include_dir}")
set(SOURCE_FILES src/utils.h src/layer.h src/layer.c src/tileset.h src/tileset.c src/entity.h src/entity.c src/tilemap.h src/utils.c src/tilemap.c gumo_serializer.h gumo_serializer.c main.c)
add_executable(Gumo_Editor ${SOURCE_FILES} resource.rc)
target_link_libraries(Gumo_Editor ${libs})

file(COPY ${deps_bin_dir}/ DESTINATION .)
file(COPY content DESTINATION .)