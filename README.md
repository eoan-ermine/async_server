# async_server

Simple multithreaded asynchronous http server written with Boost.Beast 

## Build

```shell
mkdir build && cd build
conan install .. -of .
cmake --preset conan-release ..
cmake --build .
```
