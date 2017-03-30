/*************************************************************************
	> File Name: msg.h
	> Author: perrynzhou
	> Mail:perrynzhou@gmail.com 
	> Created Time: Fri 10 Mar 2017 15:02:33 GMT
 ************************************************************************/

#ifndef _MSG_H
#define _MSG_H
#include <stdint.h>
#include "../common/g_core.h"
/* defination  message node */
typedef struct msg_s
{
	char dat[MESSAGE_MAX_LEN];
	uint32_t size;				//the data length of dat
	int64_t timestamp;
} msg;
/* create a message node */
msg *msg_create (char *dat, uint32_t size);
/* destroy a message node */
void msg_destroy (msg * m);
#endif
