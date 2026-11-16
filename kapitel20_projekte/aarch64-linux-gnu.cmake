# Kapitel 20: Toolchain-Datei für den Raspberry Pi (64 Bit, Raspberry Pi OS / Ubuntu arm64)
#   sudo apt install g++-aarch64-linux-gnu        (Compiler)
#   sudo dpkg --add-architecture arm64 && sudo apt update
#   sudo apt install libcurl4-openssl-dev:arm64   (Bibliothek)
#   cmake -S . -B build-pi -DCMAKE_BUILD_TYPE=Release \
#         -DCMAKE_TOOLCHAIN_FILE=kapitel20_projekte/aarch64-linux-gnu.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)     # /usr/lib/aarch64-linux-gnu
# Bibliotheken und Header nur für das Ziel suchen, Programme (cmake, git) auf dem Host
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu / $ENV{PI_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
# libstdc++ fest einbinden: Der Pi braucht dann nur noch die C-Bibliothek (glibc), und
# die darf nicht älter sein als die des Rechners, auf dem übersetzt wird
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libstdc++ -static-libgcc")
