/*************************************************************************
	> File Name: log.h
	> Author: perrynzhou
	> Mail: 715169549@qq.com
	> Created Time: Tue 29 Nov 2016 05:27:01 PM HKT
 ************************************************************************/

#ifndef _G_LOG_H
#define _G_LOG_H
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#define MAX_SIZE 64
typedef struct logger_s
{
	FILE *file;
	bool start;					//this value stand of logger is ok.
	pthread_mutex_t lock;
	char cur_path[MAX_SIZE * 2];
} logger;
int logger_init (const char *path);
int logger_write (int16_t level, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
void logger_disable ();
void logger_enable ();
int logger_destroy ();
#endif
