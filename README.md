## Requirements
* Boost
* [NTL](http://www.shoup.net/ntl/)

## Build
1. Setup `NTL_HEADER` and `NTL_LIB` in the main CMakeLists.txt file.
2. Clone the submodule by `git submodule init & git submodule update`
3. Make the build directory `mkdir build`
4. Run cmake inside the build director `cmake -DCMAKE_BUILD_TYPE=Release & make`
