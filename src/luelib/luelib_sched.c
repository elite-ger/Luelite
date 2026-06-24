#include "luelite.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
    double get_time(void) {
        LARGE_INTEGER freq, count;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&count);
        return (double)count.QuadPart / (double)freq.QuadPart;
    }
    void sleep_ms(int ms) { Sleep(ms); }
#else
    #include <sys/time.h>
    #include <unistd.h>
    double get_time(void) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }
    void sleep_ms(int ms) { usleep(ms * 1000); }
#endif


LueliteScheduler* luelite_sched_create(lua_State *L) {
    LueliteScheduler *sched = malloc(sizeof(LueliteScheduler));
    sched->mainL = L;
    sched->tasks = NULL;
    sched->globalTime = 0.0;
    sched->running = 1;
    lua_pushlightuserdata(L, sched);
    lua_setfield(L, LUA_REGISTRYINDEX, "_LueliteScheduler");
    return sched;
}

void luelite_sched_destroy(LueliteScheduler *sched) {
    LueliteTask *task = sched->tasks;
    while (task) {
        LueliteTask *next = task->next;
        free(task);
        task = next;
    }
    free(sched);
}

void luelite_sched_add_task(LueliteScheduler *sched, lua_State *co, double delay) {
    LueliteTask *task = malloc(sizeof(LueliteTask));
    task->coroutine = co;
    task->resumeTime = sched->globalTime + delay;
    task->next = NULL;
    
    if (!sched->tasks || sched->tasks->resumeTime > task->resumeTime) {
        task->next = sched->tasks;
        sched->tasks = task;
    } else {
        LueliteTask *curr = sched->tasks;
        while (curr->next && curr->next->resumeTime <= task->resumeTime) {
            curr = curr->next;
        }
        task->next = curr->next;
        curr->next = task;
    }
}

void luelite_sched_resume_ready(LueliteScheduler *sched) {
    while (sched->tasks && sched->tasks->resumeTime <= sched->globalTime) {
        LueliteTask *task = sched->tasks;
        sched->tasks = task->next;
        
        int status = lua_status(task->coroutine);
        int nresults;
        int ret;
        
        if (status == LUA_OK) {
            ret = lua_resume(task->coroutine, NULL, 1, &nresults);
        } else if (status == LUA_YIELD) {
            ret = lua_resume(task->coroutine, NULL, 0, &nresults);
        } else {
            free(task);
            continue;
        }
        
        if (ret != LUA_OK && ret != LUA_YIELD) {
            fprintf(stderr, "Luelite: error: %s\n",
                    lua_tostring(task->coroutine, -1));
            lua_pop(task->coroutine, 1);
        }
        
        free(task);
    }
}

int luelite_sched_has_tasks(LueliteScheduler *sched) {
    return sched->tasks != NULL;
}


static int lite_wait(lua_State *L) {
    double seconds = luaL_optnumber(L, 1, 1.0/30.0);
    lua_getfield(L, LUA_REGISTRYINDEX, "_LueliteScheduler");
    LueliteScheduler *sched = lua_touserdata(L, -1);
    luelite_sched_add_task(sched, L, seconds);
    return lua_yield(L, 0);
}

static int lite_spawn_runner(lua_State *L) {
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_call(L, 0, 0);
    return 0;
}

static int lite_spawn(lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    
    lua_State *co = lua_newthread(L);
    
    luaL_loadstring(co, "local f = ...; f()");
    lua_pushvalue(L, 1);
    lua_xmove(L, co, 1);
    
    lua_getfield(L, LUA_REGISTRYINDEX, "_LueliteScheduler");
    LueliteScheduler *sched = (LueliteScheduler*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    
    luelite_sched_add_task(sched, co, 0.0);
    
    return 0;
}

static int lite_tick(lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "_LueliteScheduler");
    LueliteScheduler *sched = lua_touserdata(L, -1);
    lua_pushnumber(L, sched->globalTime);
    return 1;
}


static const luaL_Reg luelitelib[] = {
    {"wait",  lite_wait},
    {"spawn", lite_spawn},
    {"tick",  lite_tick},
    {NULL, NULL}
};

int luaopen_luelite(lua_State *L) {
    luaL_newlib(L, luelitelib);
    
    lua_pushvalue(L, -1);
    lua_setglobal(L, "luelite");
    
    lua_getfield(L, -1, "wait");  lua_setglobal(L, "wait");
    lua_getfield(L, -1, "spawn"); lua_setglobal(L, "spawn");
    lua_getfield(L, -1, "tick");  lua_setglobal(L, "tick");
    
    luaL_dostring(L,
        "delay = function(t, f) spawn(function() wait(t); f() end) end"
    );
    
    return 1;
}