#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "h9Log.h"

static struct {
  void *udata;
  log_LockFn lock;
  FILE *fp;
  int level;
  int quiet;
  int own;
} L  = { 0, 0, NULL, -1, 1, 0};

int plen = 0;

static const char *level_names[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static void lock(void)   {
  if (L.lock) {
    L.lock(L.udata, 1);
  }
}


static void unlock(void) {
  if (L.lock) {
    L.lock(L.udata, 0);
  }
}


void log_set_udata(void *udata) {
  L.udata = udata;
}


void log_set_lock(log_LockFn fn) {
  L.lock = fn;
}


void log_set_fp(FILE *fp) {
  L.fp = fp;
  L.own = 0;
}

int log_has_fp() {
	return L.fp != NULL? 1 : 0;
}

void log_close() {
	if (L.own) {
		fclose(L.fp);
		L.fp = NULL;
	}
}

void log_set_level(int level) {
  L.level = level;
}


void log_set_quiet(int enable) {
  L.quiet = enable ? 1 : 0;
}

void log_init(const char* file, int level) {
	FILE* fp = fopen(file, "w");
	log_set_level(level);
	log_set_fp(fp);
	log_set_quiet(fp != NULL);
	L.own = 1;

}

void log_init_fp(FILE* fp, int level) {
	log_set_level(level);
	log_set_fp(fp);
	log_set_quiet(fp != NULL);
	L.own = 0;

}

void timed_log(int level, const char *file, int line, const char *fmt, ...) {
  if (level < L.level) {
    return;
  }

  /* Acquire lock */
  lock();

  /* Get current time */
  
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    char buf[32];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
  
  /*
  boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
  std::string pts = boost::posix_time::to_simple_string(pt);
  const char* buf = pts.c_str();
  */

  /* Log to stderr */
  if (!L.quiet) {
    va_list args;
    fprintf(stderr, "%s %-5s %s:%d: ", buf, level_names[level], file + plen, line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
  }

  /* Log to file */
  if (L.fp) {
    va_list args;
    fprintf(L.fp, "%s %-5s %s:%d: ", buf, level_names[level], file + plen, line);
    va_start(args, fmt);
    vfprintf(L.fp, fmt, args);
    va_end(args);
    fprintf(L.fp, "\n");
    fflush(L.fp);
  }

  /* Release lock */
  unlock();
}

void log_log(int level, const char *fmt, ...) {
	if (level < L.level) {
		return;
	}
	
	/* Acquire lock */
	lock();
	

	/* Log to stderr */
	if (!L.quiet) {
		va_list args;
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
		fprintf(stderr, "\n");
		fflush(stderr);
	}

	/* Log to file */
	if (L.fp) {
		va_list args;
		va_start(args, fmt);
		vfprintf(L.fp, fmt, args);
		va_end(args);
		fprintf(L.fp, "\n");
		fflush(L.fp);
	}
	
	/* Release lock */
	unlock();
}


void log_set_filename_path_length(int p)
{
	plen = p;
}
