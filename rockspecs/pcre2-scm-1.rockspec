rockspec_format = "3.0"
package = "pcre2"
version = "scm-1"
source = {
    url = "git+https://github.com/mah0x211/lua-pcre2.git",
}
description = {
    summary = "PCRE2 bindings for lua",
    homepage = "https://github.com/mah0x211/lua-pcre2",
    license = "MIT/X11",
    maintainer = "Masatoshi Fukunaga",
}
dependencies = {
    "lua >= 5.1",
}
external_dependencies = {
    -- external_dependencies field must be defined to preventing luarocks
    -- autodetecting dependencies and causing build failure when the
    -- dependencies are not found.
}
build_dependencies = {
    "luarocks-build-hooks >= 0.4.0",
}
build = {
    type = "hooks",
    before_build = "$(pkgconfig)",
    pkgconfig_dependencies = {
        ["LIBPCRE2-8"] = {
            header = "pcre2.h",
            library = "pcre2-8",
        },
    },
    modules = {
        ["pcre2"] = {
            sources = {
                "src/pcre2.c",
            },
            libraries = {
                "$(LIBPCRE2-8_LIB)",
            },
            incdirs = {
                "deps/lauxhlib",
                "$(LIBPCRE2-8_INCDIR)",
            },
            libdirs = {
                "$(LIBPCRE2-8_LIBDIR)",
            },
        },
    },
}
