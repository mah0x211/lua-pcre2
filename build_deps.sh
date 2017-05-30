#!/bin/sh

set -e
set -x

cd deps/pcre2/
./configure --disable-shared --enable-jit --disable-pcre2grep-jit --enable-newline-is-any
make
make check
