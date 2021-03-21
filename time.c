#include "yasl/yasl.h"

#include <time.h>
#include <string.h>

#define TIME_PRE "time"
static const char *TIME_NAME = "time";

struct YASL_Time {
	time_t time;
	int milliseconds;
};

static struct YASL_Time *YASLX_checktime(struct YASL_State *S, const char *name, int pos) {
	if (!YASL_isuserdata(S, TIME_NAME)) {
		YASL_print_err(S, "TypeError: %s expected arg in position %d to be of type time, got arg of type %s.",
			       name, pos, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return (struct YASL_Time *) YASL_popuserdata(S);
}

static struct YASL_Time *allocate_time(time_t time, int milliseconds) {
	struct YASL_Time *ptr = malloc(sizeof(struct YASL_Time));
	*ptr = ((struct YASL_Time) {.time = time, .milliseconds = milliseconds});
	return ptr;
}

static void YASL_pushtime(struct YASL_State *S, time_t time, int milliseconds) {
	YASL_pushuserdata(S, allocate_time(time, milliseconds), TIME_NAME, free);
	YASL_loadmt(S, TIME_NAME);
	YASL_setmt(S);
}

static int YASL_time_tostr(struct YASL_State *S) {
	char buff[30]; // 26 for the standard C stuff, 1 for decimal place, 3 more for milliseconds.
	struct YASL_Time *time = YASLX_checktime(S, "time.tostr", 0);
	size_t len = strftime(buff, sizeof buff, "%FT%T%z", gmtime(&time->time));
	buff[len] = '\0';

	YASL_pushzstr(S, buff);
	return 1;
}

static int YASL_time_now(struct YASL_State *S) {
	YASL_pushtime(S, time(NULL), 0);
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
