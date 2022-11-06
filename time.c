#include "yasl/yasl.h"
#include "yasl/yasl_aux.h"

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define TIME_PRE "time"
static const char *TIME_NAME = "time";

#define TIME_DELTA_PRE "timedelta"
static const char *TIME_DELTA_NAME = "timedelta";

struct YASL_Time {
	time_t time;
	int milliseconds;
};

struct YASL_TimeDelta {
	long long milliseconds;
};

static struct YASL_Time *YASLX_checktime(struct YASL_State *S, const char *name, int pos) {
	if (!YASL_isuserdata(S, TIME_NAME)) {
		YASL_print_err(S, "TypeError: %s expected arg in position %d to be of type time, got arg of type %s.",
			       name, pos, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return (struct YASL_Time *) YASL_popuserdata(S);
}

static struct YASL_TimeDelta *YASLX_checktimedelta(struct YASL_State *S, const char *name, int pos) {
	if (!YASL_isuserdata(S, TIME_DELTA_NAME)) {
		YASL_print_err(S, "TypeError: %s expected arg in position %d to be of type timedelta, got arg of type %s.",
			       name, pos, YASL_peektypename(S));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}
	return (struct YASL_TimeDelta *) YASL_popuserdata(S);
}

static struct YASL_Time *allocate_time(time_t time, int milliseconds) {
	struct YASL_Time *ptr = malloc(sizeof(struct YASL_Time));
	*ptr = ((struct YASL_Time) {.time = time, .milliseconds = milliseconds});
	return ptr;
}

static struct YASL_TimeDelta *allocate_timedelta(long long milliseconds) {
	struct YASL_TimeDelta *ptr = malloc(sizeof(struct YASL_TimeDelta));
	*ptr = ((struct YASL_TimeDelta) { .milliseconds = milliseconds});
	return ptr;
}

static void free_time(struct YASL_State *S, struct YASL_Time *time) {
    (void) S;
    free(time);
}

static void free_timedelta(struct YASL_State *S, struct YASL_TimeDelta *timedelta) {
    (void) S;
    free(timedelta);
}

static void YASL_pushtime(struct YASL_State *S, time_t time, int milliseconds) {
	YASL_pushuserdata(S, allocate_time(time, milliseconds), TIME_NAME, (void(*)(struct YASL_State *, void *))free_time);
	YASL_loadmt(S, TIME_NAME);
	YASL_setmt(S);
}

static void YASL_pushtimedelta(struct YASL_State *S, long long milliseconds) {
	YASL_pushuserdata(S, allocate_timedelta(milliseconds), TIME_DELTA_NAME, (void(*)(struct YASL_State *, void *))free_timedelta);
	YASL_loadmt(S, TIME_DELTA_NAME);
	YASL_setmt(S);
}

static int YASL_time_tostr(struct YASL_State *S) {
	char buff[30]; // 26 for the standard C stuff, 1 for decimal place, 3 more for milliseconds.
	struct YASL_Time *time = YASLX_checktime(S, TIME_PRE ".tostr", 0);
	size_t len = strftime(buff, sizeof buff, "%FT%T%z", gmtime(&time->time));
	buff[len] = '\0';

	YASL_pushzstr(S, buff);
	return 1;
}

static int YASL_time_now(struct YASL_State *S) {
	YASL_pushtime(S, time(NULL), 0);
	return 1;
}

static int YASL_time_parse(struct YASL_State *S) {
	const char *str = YASL_peekcstr(S);
	if (!str) {
		YASLX_print_err_bad_arg_type(S, "time.parse", 0, TIME_NAME, YASL_peekntypename(S, 0));
		YASL_throw_err(S, YASL_TYPE_ERROR);
	}

	int y, M, d, h, m;
	float s;
	int n = 0;

	int written = sscanf(str, "%d-%d-%dT%d:%d:%fZ%n", &y, &M, &d, &h, &m, &s, &n);
	if (written == 6 && n == (int)strlen(str)) {
		struct tm time= { .tm_year = y - 1900, .tm_mon = M - 1, .tm_mday = d, .tm_hour = h, .tm_min = m, .tm_sec = (int)truncf(s)};
		int ms = (int)truncf((s - truncf(s)) * 1000);

		YASL_pushtime(S, mktime(&time) - timezone, ms);
		return 1;
	}

	int lh, lm;
	written = sscanf(str, "%d-%d-%dT%d:%d:%f+%d:%d%n", &y, &M, &d, &h, &m, &s, &lh, &lm, &n);
	if (written == 8 && n == (int)strlen(str)) {
		struct tm time= { .tm_year = y - 1900, .tm_mon = M - 1, .tm_mday = d, .tm_hour = h - lh, .tm_min = m - lm, .tm_sec = (int)truncf(s)};
		int ms = (int)truncf((s - truncf(s)) * 1000);

		YASL_pushtime(S, mktime(&time) - timezone, ms);
		return 1;
	}

	written = sscanf(str, "%d-%d-%dT%d:%d:%f-%d:%d%n", &y, &M, &d, &h, &m, &s, &lh, &lm, &n);
	if (written == 8 && n == (int)strlen(str)) {
		struct tm time= { .tm_year = y - 1900, .tm_mon = M - 1, .tm_mday = d, .tm_hour = h + lh, .tm_min = m + lm, .tm_sec = (int)truncf(s)};
		int ms = (int)truncf((s - truncf(s)) * 1000);

		YASL_pushtime(S, mktime(&time) - timezone, ms);
		return 1;
	}

	YASL_print_err(S, "unable to parse date");
	YASL_throw_err(S, YASL_VALUE_ERROR);
}

int YASL_time_time(struct YASL_State *S) {
	yasl_int y = YASL_peeknint(S, 0);
	yasl_int M = YASL_peeknint(S, 1);
	yasl_int d = YASL_peeknint(S, 2);
	yasl_int h = YASL_peeknint(S, 3);
	yasl_int m = YASL_peeknint(S, 4);
	yasl_int s = YASL_peeknint(S, 5);
	yasl_int ms = YASL_peeknint(S, 6);

	struct tm time= { .tm_year = y - 1900, .tm_mon = M - 1, .tm_mday = d, .tm_hour = h, .tm_min = m, .tm_sec = s };

	YASL_pushtime(S, mktime(&time) - timezone, ms);
	return 1;
}

int YASL_time___sub(struct YASL_State *S) {
	struct YASL_Time *right = YASLX_checktime(S, TIME_PRE ".__sub", 1);
	struct YASL_Time *left = YASLX_checktime(S, TIME_PRE ".__sub", 0);

	long long diff = (long long)difftime(left->time, right->time);
	int ms_diff = left->milliseconds - right->milliseconds;

	YASL_pushtimedelta(S, diff * 1000 + ms_diff);
	return 1;
}

int YASL_timedelta_tostr(struct YASL_State *S) {
	struct YASL_TimeDelta *td = YASLX_checktimedelta(S, TIME_DELTA_PRE ".tostr", 1);

    long long ms = td->milliseconds;
    const char *sign = "";
    if (ms < 0) {
        sign = "-";
        ms = -ms;
    }

    char *buffer = NULL;
	int len = snprintf(NULL, 0, "timedelta(%s%lld.%03lld)", sign, ms / 1000, ms % 1000);
	buffer = realloc(buffer, len + 1);
	snprintf(buffer, len + 1, "timedelta(%s%lld.%03lld)", sign, ms / 1000, ms % 1000);
	buffer[len] = '\0';

	YASL_pushzstr(S, buffer);
	return 1;
}

int YASL_timedelta___neg(struct YASL_State *S) {
	struct YASL_TimeDelta *td = YASLX_checktimedelta(S, TIME_DELTA_PRE ".__neg", 1);

    YASL_pushtimedelta(S, -td->milliseconds);
    return 1;
}

int YASL_timedelta___add(struct YASL_State *S) {
	struct YASL_TimeDelta *left = YASLX_checktimedelta(S, TIME_DELTA_PRE ".__add", 0);
	struct YASL_TimeDelta *right = YASLX_checktimedelta(S, TIME_DELTA_PRE ".__add", 1);

    int total_ms = left->milliseconds + right->milliseconds;

	YASL_pushtimedelta(S, total_ms);
    return 1;
}


int YASL_timedelta___sub(struct YASL_State *S) {
	struct YASL_TimeDelta *left = YASLX_checktimedelta(S, TIME_DELTA_PRE ".__sub", 0);
	struct YASL_TimeDelta *right = YASLX_checktimedelta(S, TIME_DELTA_PRE ".__sub", 1);

    int total_ms = right->milliseconds - left->milliseconds;

	YASL_pushtimedelta(S, total_ms);
    return 1;
}

int YASL_timedelta_frommilliseconds(struct YASL_State *S) {
	yasl_int milliseconds = YASLX_checknint(S, TIME_PRE ".frommilliseconds", 0);

	YASL_pushtimedelta(S, milliseconds);
	return 1;
}

int YASL_timedelta_fromseconds(struct YASL_State *S) {
	yasl_int seconds = YASLX_checknint(S, TIME_PRE ".fromseconds", 0);

	YASL_pushtimedelta(S, seconds * 1000);

	return 1;
}

int YASL_timedelta_fromminutes(struct YASL_State *S) {
	yasl_int minutes = YASLX_checknint(S, TIME_PRE ".fromminutes", 0);

	long seconds = minutes * 60;
	YASL_pushtimedelta(S, seconds * 1000);

	return 1;
}

int YASL_timedelta_fromhours(struct YASL_State *S) {
	yasl_int hours = YASLX_checknint(S, TIME_PRE ".fromhours", 0);

	long seconds = hours * 60 * 60;
	YASL_pushtimedelta(S, seconds * 1000);

	return 1;
}

void YASL_load_dyn_lib(struct YASL_State *S) {
	YASL_pushtable(S);

	YASL_pushlit(S, "tostr");
	YASL_pushcfunction(S, YASL_time_tostr, 1);
	YASL_tableset(S);

	YASL_pushlit(S, "__sub");
	YASL_pushcfunction(S, YASL_time___sub, 2);
	YASL_tableset(S);

	YASL_registermt(S, TIME_NAME);

	YASL_pushtable(S);

	YASL_pushlit(S, "tostr");
	YASL_pushcfunction(S, YASL_timedelta_tostr, 1);
	YASL_tableset(S);

	YASL_pushlit(S, "__neg");
	YASL_pushcfunction(S, YASL_timedelta___neg, 1);
	YASL_tableset(S);

	YASL_pushlit(S, "__add");
	YASL_pushcfunction(S, YASL_timedelta___add, 2);
	YASL_tableset(S);

	YASL_pushlit(S, "__sub");
	YASL_pushcfunction(S, YASL_timedelta___sub, 2);
	YASL_tableset(S);

	YASL_registermt(S, TIME_DELTA_NAME);

	YASL_pushtable(S);

	YASL_pushlit(S, "now");
	YASL_pushcfunction(S, YASL_time_now, 0);
	YASL_tableset(S);

	YASL_pushlit(S, "parse");
	YASL_pushcfunction(S, YASL_time_parse, 1);
	YASL_tableset(S);

	YASL_pushlit(S, "time");
	YASL_pushcfunction(S, YASL_time_time, 7);
	YASL_tableset(S);

	YASL_pushlit(S, "timedelta");
	YASL_pushtable(S);

	YASL_pushlit(S, "frommilliseconds");
	YASL_pushcfunction(S, YASL_timedelta_frommilliseconds, 1);
	YASL_tableset(S);

	YASL_pushlit(S, "fromseconds");
	YASL_pushcfunction(S, YASL_timedelta_fromseconds, 1);
	YASL_tableset(S);

	YASL_pushlit(S, "fromminutes");
	YASL_pushcfunction(S, YASL_timedelta_fromminutes, 1);
	YASL_tableset(S);

	YASL_pushlit(S, "fromhours");
	YASL_pushcfunction(S, YASL_timedelta_fromhours, 1);
	YASL_tableset(S);

	YASL_tableset(S);
}
