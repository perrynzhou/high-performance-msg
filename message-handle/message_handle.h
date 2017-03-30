/*************************************************************************
	> File Name: message_handle.h
	> Author: perrynzhou
	> Mail:perrynzhou@gmail.com 
	> Created Time: Fri 10 Mar 2017 19:51:36 GMT
 ************************************************************************/

#ifndef _MESSAGE_SERVER_H
#define _MESSAGE_SERVER_H
#include <stdint.h>
#include "../dict/dict.h"
#include "../list/list.h"
#define MSG_QUEUE_MAX_SIZE 16384
#define MSG_HANDLE_SIZE 128
#define MSG_HANDLE_DICT_SIZE 16384

static const char *MESSAGE_HANDLE_CONF[3] = { "message-server", "server-port", "server-datadir" };

/*global message queue dict */
typedef struct message_handle_s
{
	uint32_t max_size;
	uint32_t port;
	dict *msg_queue_dict;		//key is parition name,value is index of msg_queue;
	uint32_t parition_size;
	char datadir[MSG_HANDLE_SIZE];	//load datadir from config
	dict *parition_dict;		//key as disk parition name ,value as disk file of parition data
	uint32_t connections;		//current connect to server
} message_handle;
/*  create message handle */
message_handle *message_handle_new (const char *server_conf);
/* destroy messsage handle */
void message_handle_free (message_handle * m);
#endif
