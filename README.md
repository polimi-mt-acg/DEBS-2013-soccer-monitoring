# DEBS 2013 - Soccer Monitoring
## Scope
A system to provide analytics over high velocity sensor data originating from a soccer game. The data comes from a number of wireless sensors embedded in the shoes and a ball used during a soccer match.

Our scope is to create a parallel software that computes the real-time statistics of ball possession during the game, exploiting **OpenMP** framework.

## Build
You can build either manually:
```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

or via a convenient script:
```
script/build_debug.sh # For debugging build
script/build_release.sh # For release build (i.e. optimizations enabled)
```

## Run tests
```sh
build/tests
```
