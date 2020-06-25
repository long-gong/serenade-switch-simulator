# SWITCH SIMULATOR for SERENADE

[![Build Status](https://travis-ci.org/long-gong/switch-simulator-serenade.svg?branch=master)](https://travis-ci.org/long-gong/switch-simulator-serenade)

Simulation codes for our switching paper:

Gong, L., Liu, L., Yang, S., Xu, J.J., Xie, Y. and Wang, X., 2020, May. SERENADE: A Parallel Iterative Algorithm for Crossbar Scheduling in Input-Queued Switches. In 2020 IEEE 21st International Conference on High Performance Switching and Routing (HPSR) (pp. 1-6). IEEE.

## Platforms 

LWS-Serenade supports the following platforms

  * Linux
  * Max OS X

## Dependencies 

  * [HdrHistogram_c](https://github.com/HdrHistogram/HdrHistogram_c.git)
  * [fmt](https://github.com/fmtlib/fmt)

The following two are already included in **[common](./common)** directory.

  * [json](https://github.com/nlohmann/json.git)
  * [cxxopts](https://github.com/jarro2783/cxxopts.git)

The following is optional (for Unit Test).

  * [googletest](https://github.com/google/googletest.git)

Dependencies can be installed by 
```bash
chmod +x ./install_dependencies.sh
./install_dependencies.sh
```


## Build 

```bash
mkdir build
cd build 
cmake ..
make 
```


Happy serenading!
