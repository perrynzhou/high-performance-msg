/*************************************************************************
	> File Name: util.h
	> Author: zhoulin
	> Mail: 715169549@qq.com
	> Created Time: Fri 23 Dec 2016 09:14:48 AM EST
 ************************************************************************/

#ifndef _UTIL_H
#define _UTIL_H
#include <stdbool.h>
bool is_digit (char *s);
bool is_ip (const char *ip);
char *trim_string (const char *src, int len, char *target);
char *get_client_ip (int fd, char *ip_buf);
#endif
