# Decode
Repository for an attempt to recreate the Decode app using more modern methods. Decode was one of the first applications that I had written, and given the current state of the world, it still has use.

One of the primary ideas behind Decode was enabling the layering of multiple different ciphers.

Uses [Qt6](https://www.qt.io), and [QHotkey](https://github.com/Hellmark/QHotkey), set up to be built using [CMake](https://www.cmake.org)

## How to use

### GUI

Decode's basic operation is pretty simple. If you're using the Qt based GUI, it opens up like a text editor, and you can then place the text that you want to encode or decode into the text field, and then select the operation to perform. One of the big concepts is the ability to perform multiple operations on a given text to increase difficulty for unintended parties to find the real meaning.

If you're going to use the RSA encoding, it makes use of GPG private and public pem key files. Public to encode, and private to decode. You can save a default key in settings and when performing an RSA action, select a different key if desired or remain with the default.

### Commandline

Decode's base operations are also include for use from a CLI. Make use of the following flags when using at the commandline

- **-i** or **--input**
  - specify the path of the file used for input

- **-o** or **--output** (optional)
  - specify the path of the file used for output. If none are specified, it will output to stdout.

- **-f** or **--function**
  - specify whether to **encode** or **decode**

- **-c** or **--codec**
  - declare which codec to use. Available options are base64, rot13, caesar, binary, hex, piglatin, atbash, morse, aes, rsa

- **-k** or **--key** (only for AES or RSA codecs)
  - state the key to be used for encoding or decoding. For AES, this is a text string like a password. For RSA, this would be the path for the gpg key file.

- **-h** or **--help**
  - this will state the basic CLI options for assistance.

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
