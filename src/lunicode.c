#include <lua.h>
#include <luaconf.h>
#include <lauxlib.h>
#include <stdbool.h>
#include <limits.h>
#include <utf8proc.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

static int lunicode_error(lua_State* L, utf8proc_ssize_t code) {
    return luaL_error(L, "lunicode: utf8proc error: %s", utf8proc_errmsg(code));
}

static int lunicode_isvalid(lua_State* L) {
    lua_Integer i = luaL_checkinteger(L, 1);

    if (i > 0x10FFFF || i < 0) lua_pushboolean(L, false);
    else lua_pushboolean(L, utf8proc_codepoint_valid(i));

    return 1;
}

// unused
static utf8proc_int32_t* lunicode_new_codepoint_array(lua_State* L, utf8proc_ssize_t entries) {
    utf8proc_int32_t* buffer = lua_newuserdatauv(L, entries * sizeof(utf8proc_int32_t), 0);
    luaL_getmetatable(L, "lunicode_buffer");
    lua_setmetatable(L, -2);
    return buffer;
}

static utf8proc_option_t lunicode_check_options(lua_State* L, int arg) {
    if (lua_gettop(L) >= 2) {
        switch (lua_type(L, arg)) {
            case LUA_TNUMBER:;
                lua_Integer field = luaL_checkinteger(L, arg);
                return (utf8proc_option_t)field;
            case LUA_TTABLE:;
                lua_pushnil(L);
                utf8proc_option_t opts = 0;
                while (lua_next(L, arg) != 0) {
                    size_t size;
                    lua_pop(L, 1);
                    if (lua_type(L, -1) == LUA_TSTRING){
                        const char* key = luaL_tolstring(L, -1, &size);
                        if (size == 8 && strncmp(key, "NULLTERM", 8) == 0) {
                            opts |= UTF8PROC_NULLTERM;
                        } else if (size == 6 && strncmp(key, "STABLE", 6) == 0) {
                            opts |= UTF8PROC_STABLE;
                        } else if (size == 6 && strncmp(key, "COMPAT", 6) == 0) {
                            opts |= UTF8PROC_COMPAT;
                        } else if (size == 7 && strncmp(key, "COMPOSE", 7) == 0) {
                            opts |= UTF8PROC_COMPOSE;
                        } else if (size == 9 && strncmp(key, "DECOMPOSE", 9) == 0) {
                            opts |= UTF8PROC_DECOMPOSE;
                        } else if (size == 6 && strncmp(key, "IGNORE", 6) == 0) {
                            opts |= UTF8PROC_IGNORE;
                        } else if (size == 8 && strncmp(key, "REJECTNA", 8) == 0) {
                            opts |= UTF8PROC_REJECTNA;
                        } else if (size == 6 && strncmp(key, "NFL2LS", 6) == 0) {
                            opts |= UTF8PROC_NLF2LS;
                        } else if (size == 6 && strncmp(key, "NLF2PS", 6) == 0) {
                            opts |= UTF8PROC_NLF2PS;
                        } else if (size == 6 && strncmp(key, "NLF2LF", 6) == 0) {
                            opts |= UTF8PROC_NLF2LF;
                        } else if (size == 7 && strncmp(key, "STRIPCC", 7) == 0) {
                            opts |= UTF8PROC_STRIPCC;
                        } else if (size == 8 && strncmp(key, "CASEFOLD", 8) == 0) {
                            opts |= UTF8PROC_CASEFOLD;
                        } else if (size == 9 && strncmp(key, "CHARBOUND", 9) == 0) {
                            opts |= UTF8PROC_CHARBOUND;
                        } else if (size == 4 && strncmp(key, "LUMP", 4) == 0) {
                            opts |= UTF8PROC_LUMP;
                        } else if (size == 9 && strncmp(key, "STRIPMARK", 9) == 0) {
                            opts |= UTF8PROC_STRIPMARK;
                        } else if (size == 9 && strncmp(key, "STRIPNA", 9) == 0) {
                            opts |= UTF8PROC_STRIPNA;
                        }
                    } else {
                        luaL_error(L, "lunicode.check_options: Invalid option table.");
                        return 0;
                    }
                }
                return opts;
            default:
                return luaL_argerror(L, 2, "lunicode.check_options: Invalid options provided.");
        }
    } else {
        return luaL_argerror(L, 2, "lunicode.check_options: No options provided.");
    }
}

static int lunicode_map2(lua_State* L, utf8proc_option_t options) {
    utf8proc_ssize_t size;
    const char* str = luaL_checklstring(L, 1, &size);

    utf8proc_ssize_t numwords = utf8proc_decompose(str, size, NULL, 0, options);

    if (numwords < 0) {
        return lunicode_error(L, numwords);
    }

    luaL_Buffer b;

    luaL_buffinit(L, &b);

    utf8proc_ssize_t initial_bytes = numwords * 4;

    char* space = luaL_prepbuffsize(&b, initial_bytes);

    numwords = utf8proc_decompose(str, size, space, numwords, options);

    if (numwords < 0) {
        return lunicode_error(L, numwords);
    }

    utf8proc_ssize_t numbytes = utf8proc_reencode(space, numwords, options);

    if (numbytes < 0) {
        return lunicode_error(L, numbytes);
    }

    luaL_addsize(&b, numbytes);

    luaL_pushresult(&b);

    return 1;
}



static int lunicode_map(lua_State* L) {
    return lunicode_map2(L, lunicode_check_options(L, 2));
}


static int lunicode_normalize(lua_State* L) {
    size_t ksize;
    const char* kind = luaL_optlstring(L, 2, "NFC", &ksize);

    if (ksize > 4)
        return luaL_argerror(L, 2,
            "lunicode.normalize: Unkown normalization type, expected one of: 'NFC', 'NFD', 'NFKC', 'NFKD'.");

    if (strncmp(kind, "NFC", 3) == 0) {
        return lunicode_map2(L, UTF8PROC_STABLE | UTF8PROC_COMPOSE);
    } else if (strncmp(kind, "NFD", 3) == 0) {
        return lunicode_map2(L, UTF8PROC_STABLE | UTF8PROC_DECOMPOSE);
    } else if (strncmp(kind, "NFKC", 4) == 0) {
        return lunicode_map2(L, UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_COMPAT);
    } else if (strncmp(kind, "NFKD", 4) == 0) {
        return lunicode_map2(L, UTF8PROC_STABLE | UTF8PROC_DECOMPOSE | UTF8PROC_COMPAT);
    } else {
        return luaL_argerror(L, 2,
            "lunicode.normalize: Unkown normalization type, expected one of: 'NFC', 'NFD', 'NFKC', 'NFKD'.");
    }
}

static int lunicode_category(lua_State* L) {
    lua_Integer i = luaL_checkinteger(L, 1);

    if ((0 <= i && i <= 0x10FFFF) && utf8proc_codepoint_valid(i)) {
        lua_pushinteger(L, utf8proc_category(i));
    } else {
        return luaL_error(L, "lunicode.category: Invalid codepoint %d", i);
    }

    return 1;
}

static int lunicode_category_string(lua_State* L) {
    lua_Integer i = luaL_checkinteger(L, 1);

    if ((0 <= i && i <= 0x10FFFF) && utf8proc_codepoint_valid(i)) {
        lua_pushlstring(L, utf8proc_category_string(i), 2);
    } else {
        return luaL_error(L, "lunicode.category: Invalid codepoint %d", i);
    }

    return 1;
}

static const luaL_Reg lunicode_methods[] = {
    {"valid", lunicode_isvalid},
    {"map", lunicode_map},
    {"normalize", lunicode_normalize},
    {"category", lunicode_category},
    {"category_string", lunicode_category_string},
    {NULL, NULL}
};

LUALIB_API int luaopen_lunicode(lua_State* L) {
    luaL_newlib(L, lunicode_methods);

    lua_pushliteral(L, "metadata");
    lua_createtable(L, 0, 2);

    lua_pushliteral(L, "utf8proc_version");
    lua_pushstring(L, utf8proc_version());
    lua_settable(L, -3);

    lua_pushliteral(L, "unicode_version");
    lua_pushstring(L, utf8proc_unicode_version());
    lua_settable(L, -3);

    lua_settable(L, -3);

    return 1;
}
