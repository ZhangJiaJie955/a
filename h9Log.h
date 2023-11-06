
#ifndef H9_LOG_H
#define H9_LOG_H

#include <stdio.h>
#include <stdarg.h>

#define LOG_VERSION "0.1.0"

#ifdef __cplusplus
extern "C" {

#endif
	
typedef void (*log_LockFn)(void *udata, int lock);
   
enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_LOG, LOG_WARN, LOG_ERROR, LOG_FATAL};

#if defined(H9_ALGO_TIMER) || defined(SCOPE_TIMER_ON)
#define H9_INFO(...)  log_log(LOG_INFO,  __VA_ARGS__)
#else
#define H9_INFO(...) 
#endif
  
#define H9_TRACE(...) timed_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define H9_DEBUG(...) log_log(LOG_DEBUG, __VA_ARGS__)
#define H9_LOG(...)   log_log(LOG_LOG,  __VA_ARGS__)  
#define H9_WARN(...)  log_log(LOG_WARN,  __VA_ARGS__)
#define H9_ERROR(...) timed_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define H9_FATAL(...) timed_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#define H9_TIMED_DEBUG(...) timed_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define H9_TIMED_INFO(...)  timed_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define H9_TIMED_WARN(...)  timed_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)

#define H9_LOG_INIT_FP(file, level)  log_init_fp(file, level)
#define H9_LOG_INIT(file, level)     log_init(file, level)
#define H9_READY()                   log_has_fp()
#define H9_CLOSE_LOG()               log_close()
#define H9_SET_REPOT_NAME_LENGTH(p)  log_set_filename_path_length(p)
	
void log_set_udata(void *udata);
void log_set_lock(log_LockFn fn);
void log_set_fp(FILE *fp);
int  log_has_fp();
void log_set_level(int level);
void log_set_quiet(int enable);
void log_init(const char* file, int level);
void log_init_fp(FILE* fp, int level);
void log_close();
void log_set_filename_path_length(int p);
	
void timed_log(int level, const char *file, int line, const char *fmt, ...);
void log_log(int level, const char *fmt, ...);


	
#ifdef __cplusplus
}
#endif
	
#endif
