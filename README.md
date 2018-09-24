# DEBS 2013 - Soccer Monitoring
## Project assignment
The project goal is to develop a system to provide analytics over high velocity sensor data originating from a soccer game. The data comes from a number of wireless sensors embedded in the shoes and a ball used during a soccer match. The data spans the whole duration of the game.

Using a **OpenMP** framework, the goal is to create a software that computes the real-time statistics of ball possession during the game. (i) A player is considered in possession of the ball when a) He is the player closest to the ball b) He is not farther than _K_ meters from the ball. (ii) Ball possession is undefined whenever the game was paused. (iii) The statistics need to be output for every _T_ time units as the game unfolds. (iv) The statistics accumulate every _T_ time units.

Your software needs to take as input: (i) An integer value defining _K_, ranging from 1 to 5. (ii) An integer value defining _T_, ranging from 1 to 60. It need to output a string for every _T_ time units of play, arbitrarily formatted, with the ball possession statistics for _each player_ and for the _whole team_.

The output may be directed to `stdout` or a file.

## Performance
Check [docs](docs) for information about our implementation scalability.

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
