cmake_minimum_required(VERSION 3.10.0)
project(unique_components VERSION 0.1.0 LANGUAGES C CXX)

# g++ main.cpp -std=c++20 -isystem benchmark/include -Lbenchmark/build/src -lbenchmark -lpthread -o mybenchmark -ggdb

add_executable(unique_components main.cpp)
target_compile_features(unique_components PRIVATE cxx_std_20 )
target_include_directories(unique_components PRIVATE benchmark/include)
target_link_libraries(unique_components PRIVATE benchmark pthread)


