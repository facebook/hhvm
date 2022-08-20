# FbThrift Compiler

The Thrift compiler is a standalone binary to read and generate code for any proper `*.thrift` file.

Contents
========
* [Downloading](#downloading)
* [Dependencies](#dependencies)
  * [Ubuntu](#ubuntu)
  * [macOS](#macos)
  * [Windows (MSVC)](#windows-msvc)
* [Building](#building)
* [Testing](#testing)
* [Usage](#usage)

## Downloading
```
git clone https://github.com/facebook/fbthrift.git
```

## Dependencies
- [Cmake](https://cmake.org/) package builder
- C++ [Boost](http://www.boost.org/)
- [Bison](https://www.gnu.org/software/bison/)

### Ubuntu
```
sudo apt-get install cmake bison libboost-all-dev
```

### macOS
Using [Homebrew](http://brew.sh/)
```
brew install cmake bison boost
```

### Windows (MSVC)
- Install [Microsoft Visual Studio](https://www.visualstudio.com/vs/)
 - Make sure to select Visual C++ tools and the Windows SDK during installation
 - Otherwise, open Visual Studio File > New > Project > Online. Then, instal Visual C++ and Windows SDK
- Install [Microsoft Build Tools](https://www.microsoft.com/en-us/download/details.aspx?id=48159)
 - Add MSBuild to the PATH variable
 ```
 # Using Powershell (make sure path matches your install directory):
 [System.Environment]::SetEnvironmentVariable("PATH", "$env:Path;C\Program Files\MSBuild\14.0\Bin", [System.EnvironmentVariableTarget]::Machine)
 ```

- Install [CMake](http://www.cmake.org)
 - Download and install one of the latest Windows .msi file
 - During install, select: Make path available for all users or for this user
- Install [precompiled Boost](https://sourceforge.net/projects/boost/files/boost-binaries/), or compile it on your own
 - Make sure that Boost matches your machine architecture (32 or 64) and your MSVC version
 - After installing open the installation path an rename the directory 'lib<version>' -> 'lib'
 - Add C:\path\to\boost_1_<ver>_0 to your PATH variable
 ```
 # Using Powershell (make sure path matches your install directory):
 [System.Environment]::SetEnvironmentVariable("PATH", "$env:Path;C\local\boost_1_<ver>_0", [System.EnvironmentVariableTarget]::Machine)
 ```
- Install [precompiled OpenSSL](https://slproweb.com/products/Win32OpenSSL.html), or compile it on your own
 - Download the complete version (no Light) and make sure that OpenSSL matches your machine architecture (32 or 64)
 - Default install, it will add the path to your PATH variable
- Download [winflexbison.zip](https://sourceforge.net/projects/winflexbison/)
 - Create a directory where your boost_1_<ver>_0 directory is called 'win_flex_bison'(or any name you want to use)
 - Unzip and move win_bison.exe, and data/ to that directory
 - Add the directory to your PATH variable
 ```
 # Using Powershell (make sure path matches your install directory):
 [System.Environment]::SetEnvironmentVariable("PATH", "$env:Path;C\local\win_flex_bison", [System.EnvironmentVariableTarget]::Machine)
 ```

## Building
```
mkdir fbthrift/build
cd fbthrift/build
```
Locally installed dependencies:
```
cmake \
  -Dcompiler_only=ON \
  ..
```
Globally installed dependencies:
```
cmake -Dcompiler_only=ON ..
```

## Testing
To test, you need to install the external dependencies and then build with the proper flags

### Dependencies
- [Python](https://www.python.org/)
- [GTest and GMock](https://github.com/google/googletest)
```
mkdir -p ~/tmp/gtest
git clone https://github.com/google/googletest ~/tmp/gtest
cd ~/tmp/gtest/googletest
cmake .
make
cd ~/tmp/gtest/googlemock
cmake .
make
```
Global Install:
```
cd ~/tmp/gtest/googletest
make install
cd ~/tmp/gtest/googlemock
make install
```

### Building and Running Tests
```
mkdir fbthrift/build
cd fbthrift/build
```
Locally installed dependencies:
```
cmake \
  -Denable_tests=ON \
  -Dcompiler_only=ON \
  -DGTEST_ROOT="~/tmp/googletest" \
  -DGMOCK_ROOT="~/tmp/googlemock" \
  ..
make
make test
```
Globally installed dependencies:
```
cmake -Denable_tests=ON -Dcompiler_only=ON ..
make
make test
```

## Installing

This will create a `thrift1` binary file inside the build directory.

### Linux or macOS
```
make install
```

### Windows (MSVC)
```
# Using PowerShell
msbuild .\INSTALL.vcxproj
```

## Usage

### Linux or macOS
```
./thrift1 --help
```

### Windows (MSVC)
```
# Using PowerShell
thrift1.exe --help
```
