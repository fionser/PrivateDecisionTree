cmake_minimum_required(VERSION 3.2)
project(private_decision_tree)
set(CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(NTL_HEADER /usr/local/include)
set(NTL_LIB /usr/local/lib)
add_subdirectory(HElib)

include_directories(${NTL_HEADER} HElib/include include)
link_directories(${NTL_LIB})

set(HELIB_HEADER HELib/include)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_subdirectory(src)
add_executable(main main.cpp)
target_link_libraries(main ppdt helib ntl pthread gmp boost_system)
add_executable(xcmp bench_XCMP.cpp)
target_link_libraries(xcmp ppdt helib ntl pthread gmp boost_system)
