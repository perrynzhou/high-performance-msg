/*************************************************************************
	> File Name: message_client.h
	> Author: perrynzhou
	> Mail:perrynzhou@gmail.com 
	> Created Time: Fri 10 Mar 2017 19:41:59 GMT
 ************************************************************************/

#ifndef _MESSAGE_CLIENT_H
#define _MESSAGE_CLIENT_H
#include <stdint.h>

#define MSG_ONCE_SIZE 4096

#define RESULT_BUFFER_SIZE 128
/* conf file section info */
static char *MESSAGE_SERVER_CONF[6] = { "message-server", "message-server-ip", "message-server-port",
	"message-type", "message-parition-name", "message-publish"
};

#endif
