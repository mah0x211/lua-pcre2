rockspec_format = "3.0"
package = "pcre2"
version = "scm-1"
source = {
    url = "git+https://github.com/mah0x211/lua-pcre2.git"
}
description = {
    summary = "PCRE2 bindings for lua",
    homepage = "https://github.com/mah0x211/lua-pcre2",
    license = "MIT/X11",
    maintainer = "Masatoshi Teruya"
}
dependencies = {
    "lua >= 5.1",
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
