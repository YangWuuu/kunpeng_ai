/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */
#pragma once

#define USE_LOG
#ifdef USE_LOG

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef void (*log_LockFn)(void *udata, int lock);

enum {
    LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL
};

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define RAPIDS_FILE (__FILENAME__)
#define log_trace(...) log_log(LOG_TRACE, RAPIDS_FILE, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, RAPIDS_FILE, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  RAPIDS_FILE, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  RAPIDS_FILE, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, RAPIDS_FILE, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, RAPIDS_FILE, __LINE__, __VA_ARGS__)

void log_set_udata(void *udata);

void log_set_lock(log_LockFn fn);

void log_set_fp(FILE *fp);

void log_set_level(int level);

void log_set_quiet(int enable);

void log_log(int level, const char *file, int line, const char *fmt, ...);

#else //use log
#define log_trace(...)
#define log_debug(...)
#define log_info(...)
#define log_warn(...)
#define log_error(...)
#define log_fatal(...)
#endif //use log
