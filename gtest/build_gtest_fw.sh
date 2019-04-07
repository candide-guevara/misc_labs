#! /bin/bash

BUILD_DIR="`mktemp -u -p .`"

[[ -d include ]] || mkdir include
[[ -d lib ]] || mkdir lib

mkdir "$BUILD_DIR"
pushd "$BUILD_DIR"

git clone https://github.com/google/googletest.git
cmake googletest
make -f googlemock/Makefile

mv googlemock/*.a ../lib
mv googlemock/gtest/*.a ../lib

mv googletest/googletest/include/*  ../include
mv googletest/googlemock/include/*  ../include

popd

