cmake_minimum_required (VERSION 3.0.0)

project (audio-thingies CXX)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_library(audio STATIC
  SdlGuard.cpp
  AudioDevice.h
  AudioDevice_impl.h
  AudioSequence.h
  AudioSequence_impl.h
  SdlGuard.h
)
target_include_directories(audio PUBLIC ${CURRENT_SOURCE_DIR})
target_link_libraries(audio PUBLIC ${SDL2_LIBRARIES})

add_executable (echo echo.cpp)
target_link_libraries(echo audio)

add_executable (loopback loopback.cpp)
target_link_libraries(loopback audio)

add_executable (spectrum spectrum.cpp)
target_link_libraries(spectrum audio)

add_executable (sweep sweep.cpp)
target_link_libraries(sweep audio)
