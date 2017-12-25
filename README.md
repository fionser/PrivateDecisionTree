## Requirements
* Boost
* [NTL](http://www.shoup.net/ntl/)

## Build
1. Setup `NTL_HEADER` and `NTL_LIB` in the main CMakeLists.txt file.
2. Clone the submodule by `git submodule init & git submodule update`
3. Make the build directory `mkdir build`
4. Run cmake inside the build director `cmake -DCMAKE_BUILD_TYPE=Release & make`

## RUN
1. For playing the server's role, `./main r=1 i=<tree model file>`
2. For playing the client's role, `./main r=0 i=<client input file>`

There are some samples in the `samples` directory.

## CONFIG
We can run the decision tree evaluation with both encrypted tree model and encrypted client's input.
To do so, turns the macos `PLAIN_THRESHOLD` to `0` in files `PPDTClient.cpp` and
`PPDTServer.cpp`.

