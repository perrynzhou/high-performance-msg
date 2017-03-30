/*************************************************************************
	> File Name: CORE.h
	> Author: zhoulin
	> Mail: 715169549@qq.com
	> Created Time: Fri 23 Dec 2016 08:36:11 AM EST
 ************************************************************************/

#ifndef _G_CORE_H
#define _G_CORE_H
#include <unistd.h>
#include <linux/limits.h>
#include <sys/time.h>
#include <syscall.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../log/log.h"
#include "../log/g_log.h"
#define shm_key 0x1024
#define g_free(X)    free(X)
#define g_malloc(X)  calloc(1,X)
#define g_calloc(X)  calloc(1,X)
#define BUF_MAX_SIZE 256
#define MAX_LISTEN_SIZE 1024
#define PARITION_NAME_MAX_SIZE 128
#define MESSAGE_MAX_LEN 256
#define TYPE_MAX_LEN 16
/* defination message type */
static char *MESSAGE_TYPE[2] = { "queue", "topic" };

/* recive messgae from server */
static char *RET_TYPE[4] = { "W_OK", "W_FAILED", "R_OK", "R_FAILED" };

enum RET
{ w_y = 0, w_n, r_y, r_n };
typedef struct init_pkg_s
{
	char parition_name[PARITION_NAME_MAX_SIZE];
	int8_t is_publish;
	int is_exit;
	char type[TYPE_MAX_LEN];
	int64_t timestamp;
	int8_t flag;
} init_pkg;
#endif
