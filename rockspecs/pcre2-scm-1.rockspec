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
    maintainer = "Masatoshi Fukunaga"
}
dependencies = {
    "lua >= 5.1",
}
external_dependencies = {
    LIBPCRE2 = {
        header = "pcre2.h",
        library = "pcre2-8",
    }
}
build = {
    type = "builtin",
    modules = {
        ["pcre2"] = {
            sources = { "src/pcre2.c" },
            libraries = { "pcre2-8" },
            incdirs = {
                "deps/lauxhlib",
                "$(LIBPCRE2_INCDIR)"
            },
            libdirs = {
                "$(LIBPCRE2_LIBDIR)"
            }
        },
    }
}
