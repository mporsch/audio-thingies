cmake_minimum_required (VERSION 3.0.0)

project (audio-thingies CXX)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable (loopback loopback.cpp)
target_link_libraries(loopback
  #mingw32
  ${SDL2_LIBRARIES}
)
