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
mcst_imp fp vo=[0,1,2,3] pf=[0,1] δ
``

This executable _accepts_ the following parameters:

- fp: the path of the selected dataset
- vo: the order of vertices, "0" for original order, "1" for degree order, "2" for core order, "3" for the efficient search strategies
- pf: the flag for the advanced pruning rules, and "0" for disabled, "1" for enabled
- δ: a user-defined parameter, and should be within [0.5,1)
