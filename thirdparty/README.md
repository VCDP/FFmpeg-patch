# Dependent thirdparty

## Inference Engine C-API

The Inference Engine C-API is a C library  based on the Inference Engine C++ library implementation. The C library provides an interface to read the Intermediate Representation, set the input and output formats,  execute the model on devices and infer input data(images) and get a result.

## Build Inference Engine C-API

### Build on Linux\* Systems

The software was validated on:

- Ubuntu* 18.04(64-bit) with default GCC* 7.4.0

#### Software Requirements

- [CMake]\* 3.5 or higher
- GCC\* 4.8 or higher to build the Inference Engine
- Install OpenVINO R3
- (Optional) [Install Intel® Graphics Compute Runtime for OpenCL™ Driver package 19.04.12237].

#### Install

- You can run the `build.sh` script or by cmake manually to install the IE C API. Install steps by cmake as fallows:

1. Create a build folder:

   ```sh
   mkdir build && cd build
   ```
2. In the created `build` directory, run `cmake` to fetch project dependencies and create Unix makefiles, then run `make` to build the project:

   ```sh
   cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_AVX512F=OFF ..
   make --jobs=$(nproc --all)
   ```
3. Run `make install` to install the library, default installation path is `/usr/local`

   ```sh
   sudo make install
   ```
