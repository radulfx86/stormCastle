cmake_minimum_required(VERSION 3.5)
project(StormCastle)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -fstack-check -g")
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -fno-omit-frame-pointer -fdiagnostics-show-option -fno-optimize-sibling-calls")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -fno-omit-frame-pointer -fdiagnostics-show-option")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb")

link_libraries(m)
link_libraries(GL)
link_libraries(SDL2)
link_libraries(SDL2_mixer)
link_libraries(SDL2_image)

add_subdirectory(src)
