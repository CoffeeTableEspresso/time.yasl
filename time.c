#include "yasl/yasl.h"

#include <time.h>
#include <string.h>

#define TIME_PRE "time"
static const char *TIME_NAME = "time";

static time_t *YASLX_checktime(struct YASL_State *S, const char *name, int pos) {
    if (!YASL_isuserdata(S, TIME_NAME)) {
        YASL_print_err(S, "TypeError: %s expected arg in position %d to be of type time, got arg of type %s.",
                name, pos, YASL_peektypename(S));
        YASL_throw_err(S, YASL_TYPE_ERROR);
    }
    return (time_t *)YASL_popuserdata(S);
}

static time_t *allocate_time(time_t time) {
    time_t *ptr = malloc(sizeof(time_t));
    *ptr = time;
    return ptr;
}

static void YASL_pushtime(struct YASL_State *S, time_t time) {
    YASL_pushuserdata(S, allocate_time(time), TIME_NAME, free);
    YASL_loadmt(S, TIME_NAME);
    YASL_setmt(S);
}

static int YASL_time_tostr(struct YASL_State *S) {
    char buff[26];
    time_t *time = YASLX_checktime(S, "time.tostr", 0);
    size_t len = strftime(buff, sizeof buff , "%FT%T%z", gmtime(time));
    buff[len] = '\0';

    char *buffer = malloc(strlen(buff) + 1);
    strcpy(buffer, buff);
    YASL_pushzstr(S, buffer);
    return 1;
}

static int YASL_time_now(struct YASL_State *S) {
    YASL_pushtime(S, time(NULL));
    return 1;
}

void YASL_load_dyn_lib(struct YASL_State *S) {
    YASL_pushtable(S);
    YASL_registermt(S, TIME_NAME);

    YASL_loadmt(S, TIME_NAME);
    YASL_pushlit(S, "tostr");
    YASL_pushcfunction(S, YASL_time_tostr, 1);
    YASL_tableset(S);

    YASL_pushtable(S);

    YASL_pushlit(S, "now");
    YASL_pushcfunction(S, YASL_time_now, 0);
    YASL_tableset(S);
}
