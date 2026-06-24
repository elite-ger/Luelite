#ifndef LUELITE_H
#define LUELITE_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef struct LueliteTask {
    lua_State *coroutine;
    double resumeTime;
    struct LueliteTask *next;
} LueliteTask;

typedef struct {
    lua_State *mainL;
    LueliteTask *tasks;
    double globalTime;
    int running;
} LueliteScheduler;

LueliteScheduler* luelite_sched_create(lua_State *L);
void luelite_sched_destroy(LueliteScheduler *sched);
void luelite_sched_add_task(LueliteScheduler *sched, lua_State *co, double delay);
void luelite_sched_resume_ready(LueliteScheduler *sched);
int luelite_sched_has_tasks(LueliteScheduler *sched);

double get_time(void);
void sleep_ms(int ms);

int luaopen_luelite(lua_State *L);

#endif