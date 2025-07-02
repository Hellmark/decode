# Decode
Repository for an attempt to recreate the Decode app using more modern methods. Decode was one of the first applications that I had written, and given the current state of the world, it still has use.

One of the primary ideas behind Decode was enabling the layering of multiple different ciphers.

Uses [Qt6](https://www.qt.io), and [QHotkey](https://github.com/Hellmark/QHotkey), set up to be built using [CMake](https://www.cmake.org)

## How to build

Included is a .pro file for use for building using Qt Creator / qmake, and a CMakeLists.txt file for use with CMake.

### CMake

If you're building using CMake, things should be pretty much ready to go, with the steps needed to be taken are fairly simple.

For the most simple you could do the following:

  ```cmake .
  make
  ```

If you're wanting a little more clean build, with the source directory being kept clean, you could do the following:

  ```mkdir build
  cd build
  cmake -S ../ -B ./
  make
  ```

### Qt Creator or qmake

A .pro file is included for use with Qt's own build tools, but some work may need to be done with that, depending on your particular environment.

#### Homebrew

In the testing that has been done so far, some complications seem to arise with Macs, especially in regard to [Homebrew](https://www.brew.sh). Certain aspects may not be included in your path by default, and may need to be supplied. To do this, in the Decode.pro file, if you have issues regarding OpenSSL not being found, uncomment the LIBS and INCLUDEPATH lines. After it has been uncommented, make sure that the values reflect the version installed on your system.

Homebrew also doesn't necessarily support universal binaries at this point, so you may need to comment out the architecture that you're not currently using. So, like if you're on Apple silicon, set QMAKE_APPLE_DEVICE_ARCHS to just arm64.
