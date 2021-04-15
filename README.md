## ExaM2M

_Exascale Mesh-To-Mesh Transfer_

## Install prerequisites

* _Debian/Ubuntu_  (line 1: required, line 2: recommended)
```
apt-get install cmake gcc g++ openmpi-bin libopenmpi-dev
apt-get install libhdf5-dev libhdf5-openmpi-dev libnetcdf-mpi-dev
```

## Build
```
git clone --recurse-submodules https://github.com/Charmworks/ExaM2M.git
cd ExaM2M; mkdir external/build; cd external/build; cmake .. && make; cd -
mkdir build; cd build; cmake ../src && make && ./charmrun +p2 Main/exam2m
```
