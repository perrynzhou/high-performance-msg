/*************************************************************************
	> File Name: log.h
	> Author: perrynzhou
	> Mail: 715169549@qq.com
	> Created Time: Tue 29 Nov 2016 07:49:27 PM HKT
 ************************************************************************/

#ifndef _LOG_H
#define _LOG_H
#include "g_log.h"
#define M_DEBUG   0
#define M_INFO    1
#define M_WARNING  2
#define M_ERROR  3
#define M_FATAL 4
#define log_destroy()    logger_destroy()
#define log_disable()    logger_disable()
#define log_enable()     logger_enable()
#define log_init(A)      logger_init(A)
#define log_fatal(fmt, ...) logger_write(M_FATAL, "[%s:%d %s] "fmt,__FILE__,__LINE__,__func__, ##__VA_ARGS__)
#define log_error(fmt, ...) logger_write(M_ERROR, "[%s:%d %s] "fmt,__FILE__,__LINE__,__func__, ##__VA_ARGS__)
#define log_warning(fmt, ...) logger_write(M_WARNING, "[%s:%d %s] "fmt,__FILE__,__LINE__,__func__, ##__VA_ARGS__)
#define log_info(fmt, ...) logger_write(M_INFO, "[%s:%d %s] "fmt,__FILE__,__LINE__,__func__, ##__VA_ARGS__)
#define log_debug(fmt, ...) logger_write(M_DEBUG, "[%s:%d %s] "fmt,__FILE__,__LINE__,__func__, ##__VA_ARGS__)
#endif
