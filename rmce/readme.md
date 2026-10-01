## Compile

``
mkdir build
``

``
cd ./build
``

``
cmake ..
``

``
make
``

## Usage

``
mcst_naive fp vo=[0,1,2] δ
``

This executable _accepts_ the following parameters:

- fp: the path of the selected dataset
- vo: the order of vertices, "0" for original order, "1" for degree order, "2" for core order
- δ: a user-defined parameter, and should be within [0.5,1)
