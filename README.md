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
  
