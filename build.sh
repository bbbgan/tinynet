#!/bin/sh

set -x

SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-./tinynet-build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
# BUILD_TYPE=${BUILD_TYPE:-Release}
INSTALL_DIR=${INSTALL_DIR:-../${BUILD_TYPE}-install}

rm -rf $BUILD_DIR \
    && mkdir -p $BUILD_DIR/$BUILD_TYPE \
    && mkdir -p $BUILD_DIR/${BUILD_TYPE}-install \
    && cd $BUILD_DIR/$BUILD_TYPE \
    && cmake \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
        $SOURCE_DIR \
    && make $*