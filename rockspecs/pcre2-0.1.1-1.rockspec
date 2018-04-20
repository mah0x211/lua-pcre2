package = "pcre2"
version = "0.1.1-1"
source = {
    url = "gitrec://github.com/mah0x211/lua-pcre2.git",
    tag = "v0.1.1"
}
description = {
    summary = "PCRE2 bindings for lua",
    homepage = "https://github.com/mah0x211/lua-pcre2",
    license = "MIT/X11",
    maintainer = "Masatoshi Teruya"
}
dependencies = {
    "lua >= 5.1",
    "luarocks-fetch-gitrec >= 0.2"
}
build = {
    type = "command",
    build_command = [[
        CFLAGS="$(CFLAGS)" sh build_deps.sh && autoreconf -ivf && CFLAGS="$(CFLAGS)" CPPFLAGS="-I$(LUA_INCDIR)" LIBFLAG="$(LIBFLAG)" OBJ_EXTENSION="$(OBJ_EXTENSION)" LIB_EXTENSION="$(LIB_EXTENSION)" LIBDIR="$(LIBDIR)" CONFDIR="$(CONFDIR)" ./configure && make clean && make
    ]],
    install_command = [[
        make install
    ]]
}



