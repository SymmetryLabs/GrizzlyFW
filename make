#!/bin/sh
clear
echo "==========================================================================="
if g++-mp-4.8 -std=c++11 -I./inc/ -I./libs/oscpack/ ./src/test_main.cpp ./libs/oscpack/osc/*.cpp ./libs/oscpack/ip/posix/*.cpp
then ./a.out
fi
exit $?