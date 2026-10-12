# Animixel: A Rigged Animation Toll for Pixel Art  

## Building and Running:  
This tool requires [CMake](https://cmake.org/) (min version 3.10) as well as a C compiler ([GCC](https://gcc.gnu.org/), [Clang](https://clang.llvm.org/) or [MSVC](https://visualstudio.microsoft.com/vs/features/cplusplus/))  

### Windows  
```
cd ${DIR-WITH-CODE}  
mkdir build
cd build
cmake -G "${YOUR-CMAKE-BUILD-CONFIG}" ..
cmake --build .
.\amxl.exe
```

### Mac and Linux  
```
cd ${DIR-WITH-CODE}  
mkdir build
cd build
cmake ..
make
./amxl
```
