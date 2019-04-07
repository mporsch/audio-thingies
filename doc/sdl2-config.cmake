# download SDL2-devel-2.0.9-mingw.tar.gz from https://www.libsdl.org/download-2.0.php
# place in x86_64-w64-mingw32/

set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

set(SDL2_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/lib/libSDL2main.a;${CMAKE_CURRENT_LIST_DIR}/lib/libSDL2.dll.a")

string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
