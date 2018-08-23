# DEBS 2013 - Soccer Monitoring
## Scope
A system to provide analytics over high velocity sensor data originating from a soccer game. The data comes from a number of wireless sensors embedded in the shoes and a ball used during a soccer match.

Our scope is to create a parallel software that computes the real-time statistics of ball possession during the game, exploiting **OpenMP** framework.

## Setup
**Soccer Monitoring** tool requires the following dependencies to be installed on your system:
 - OpenMP headers (>= 3.x)
 - Boost::program_options
 - CMake (>= 3.x)
 
Moreover, this projects uses `git submodules` so one more step is required to get the complete source:
```bash
git clone https://github.com/polimi-mt-acg/DEBS-2013-soccer-monitoring
cd DEBS-2013-soccer-monitoring
chmod -R +x script
script/bootstrap.sh
```

## Build
You can build either manually:
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

or via a convenient script:
```bash
script/build_debug.sh # For debugging build
script/build_release.sh # For release build (i.e. optimizations enabled)
```

## Run tests
```bash
build/tests
```
## Performance
Check [docs](docs) for information about the application scalability.
