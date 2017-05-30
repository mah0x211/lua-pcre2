#!/bin/sh

set -e
set -x

cd deps/pcre2/
./autogen.sh
./configure --disable-shared --enable-jit --disable-pcre2grep-jit --enable-newline-is-any
make
make check
