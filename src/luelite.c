#include "luelite.h"
#include "luelib/luelib_http.h"
#include "luelib/luelib_match.h"
#include <stdio.h>
#include <string.h>

static int is_luelite_file(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot)
        return 0;
    return (strcmp(dot, ".lua") == 0 || strcmp(dot, ".luelite") == 0);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Luelite 0.1\n");
        printf("Usage: luelite <script.lua|script.luelite>\n");
        return 0;
    }

    if (!is_luelite_file(argv[1]))
    {
        printf("Error: '%s' is not a .lua or .luelite file\n", argv[1]);
        return 1;
    }

    FILE *test = fopen(argv[1], "r");
    if (!test)
    {
        printf("Error: cannot open '%s'\n", argv[1]);
        return 1;
    }
    fclose(test);

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    luaL_requiref(L, "luelite", luaopen_luelite, 1);
    lua_pop(L, 1);
    luaL_requiref(L, "http", luaopen_luelib_http, 1);
    lua_pop(L, 1);
    luaopen_luelib_match(L);

    LueliteScheduler *sched = luelite_sched_create(L);
    sched->globalTime = get_time();

    lua_State *mainCo = lua_newthread(L);
    if (luaL_loadfile(mainCo, argv[1]) != LUA_OK)
    {
        printf("Error: %s\n", lua_tostring(mainCo, -1));
        luelite_sched_destroy(sched);
        lua_close(L);
        return 1;
    }

    int nresults;
    int ret = lua_resume(mainCo, NULL, 0, &nresults);

    if (ret == LUA_OK)
    {
    }
    else if (ret == LUA_YIELD)
    {
    }
    else
    {
        printf("Runtime error: %s\n", lua_tostring(mainCo, -1));
        luelite_sched_destroy(sched);
        lua_close(L);
        return 1;
    }

    while (luelite_sched_has_tasks(sched))
    {
        double currentTime = get_time();
        sched->globalTime = currentTime;
        luelite_sched_resume_ready(sched);
        double elapsed = get_time() - currentTime;
        if (elapsed < 1.0 / 30.0)
        {
            sleep_ms((int)((1.0 / 30.0 - elapsed) * 1000));
        }
    }

    luelite_sched_destroy(sched);
    lua_close(L);
    return 0;
}