#/bin/bash

brew install cmake ninja llvm

SOURCE_PATH="$( cd "$(dirname $0)/../../.." && pwd  )" 
SWIFT_PATH=$SOURCE_PATH/swift
BUILD_SCRIPT=$SWIFT_PATH/utils/webassembly/build-mac.sh
cd $SWIFT_PATH

export current_sha=`git rev-parse HEAD`
./utils/update-checkout --clone --scheme wasm
git checkout $current_sha

cd $SOURCE_PATH
wget -O wasi-sdk.tar.gz https://github.com/swiftwasm/wasi-sdk/releases/download/20191022.1/wasi-sdk-4.39g3025a5f47c04-linux.tar.gz
tar xfz wasi-sdk.tar.gz
mv wasi-sdk-4.39g3025a5f47c04 ./wasi-sdk
mv wasi-sdk/share/wasi-sysroot wasi-sdk/share/sysroot
# Link sysroot/usr/include to sysroot/include because Darwin sysroot doesn't 
# find header files in sysroot/include but sysroot/usr/include
mkdir wasi-sdk/share/sysroot/usr/
ln -s ../include wasi-sdk/share/sysroot/usr/include
# Link wasm32-wasi-unknown to wasm32-wasi because clang finds crt1.o from sysroot
# with os and environment name `getMultiarchTriple`.
ln -s wasm32-wasi wasi-sdk/share/sysroot/lib/wasm32-wasi-unknown
wget -O icu.tar.xz "https://github.com/swiftwasm/icu4c-wasi/releases/download/20190421.3/icu4c-wasi.tar.xz"
tar xf icu.tar.xz

$BUILD_SCRIPT --release --debug-swift-stdlib --verbose -t
