#include "luelite.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int lite_match(lua_State *L) {
    luaL_checktype(L, 2, LUA_TTABLE);
    
    lua_pushvalue(L, 1);
    lua_gettable(L, 2);
    if (!lua_isnil(L, -1)) {
        if (lua_isfunction(L, -1)) {
            lua_call(L, 0, 1);
        }
        return 1;
    }
    lua_pop(L, 1);

    if (lua_isnumber(L, 1)) {
        double val = lua_tonumber(L, 1);
        
        for (int i = -100; i <= 100; i++) {
            char key[32];
            snprintf(key, sizeof(key), "<LT:%d", i);
            lua_pushstring(L, key);
            lua_gettable(L, 2);
            
            if (!lua_isnil(L, -1)) {
                if (val < (double)i) {
                    if (lua_isfunction(L, -1)) {
                        lua_call(L, 0, 1);
                    }
                    return 1;
                }
            }
            lua_pop(L, 1);
        }
    }
    
    if (lua_isnumber(L, 1)) {
        double val = lua_tonumber(L, 1);
        
        for (int a = -100; a <= 100; a++) {
            for (int b = a; b <= 100; b++) {
                char key[64];
                snprintf(key, sizeof(key), "<BT:%d:%d", a, b);
                lua_pushstring(L, key);
                lua_gettable(L, 2);
                
                if (!lua_isnil(L, -1)) {
                    if (val >= a && val <= b) {
                        if (lua_isfunction(L, -1)) {
                            lua_call(L, 0, 1);
                        }
                        return 1;
                    }
                }
                lua_pop(L, 1);
            }
        }
    }
    
    lua_pushstring(L, "<DF>");
    lua_gettable(L, 2);
    if (!lua_isnil(L, -1)) {
        if (lua_isfunction(L, -1)) {
            lua_call(L, 0, 1);
        }
        return 1;
    }
    lua_pop(L, 1);
    
    return 0;
}

static int lite_match_default(lua_State *L) {
    lua_pushstring(L, "<DF>");
    return 1;
}

static int lite_match_lt(lua_State *L) {
    int n = (int)luaL_checknumber(L, 1);
    lua_pushfstring(L, "<LT:%d", n);
    return 1;
}

static int lite_match_between(lua_State *L) {
    int a = (int)luaL_checknumber(L, 1);
    int b = (int)luaL_checknumber(L, 2);
    lua_pushfstring(L, "<BT:%d:%d", a, b);
    return 1;
}

int luaopen_luelib_match(lua_State *L) {
    lua_pushcfunction(L, lite_match);
    lua_setglobal(L, "match");
    
    lua_pushcfunction(L, lite_match_default);
    lua_setglobal(L, "_default");
    lua_pushcfunction(L, lite_match_lt);
    lua_setglobal(L, "_lt");
    lua_pushcfunction(L, lite_match_between);
    lua_setglobal(L, "_between");
    
    return 0;
}