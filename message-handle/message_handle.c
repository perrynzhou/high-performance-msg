/*************************************************************************
	> File Name: message_handle.c
	> Author: perrynzhou
	> Mail:perrynzhou@gmail.com 
	> Created Time: Fri 10 Mar 2017 20:31:12 GMT
 ************************************************************************/

#include<stdio.h>
#include <string.h>
#include <stdlib.h>
#include "message_handle.h"
#include "../ini/ini.h"
#include "../common/g_util.h"
#include "../common/g_core.h"
static ini *m_ini;
message_handle *message_handle_new (const char *server_conf)
{
	uint32_t stop = 0;
	message_handle *mh = NULL;
	dict *msg_queue_dict = NULL;
	dict *parition_dict = NULL;
	dict *connect_user_dict = NULL;
	if (server_conf == NULL)
	{
		goto _FAILED;
	}
	if ((m_ini = ini_create (server_conf)) == NULL)
	{
		goto _FAILED;
	}
	mh = (message_handle *) g_malloc (sizeof (*mh));
	msg_queue_dict = dict_new (MSG_HANDLE_DICT_SIZE);
	parition_dict = dict_new (MSG_HANDLE_DICT_SIZE);
	if (mh == NULL || parition_dict == NULL)
	{
		goto _FAILED;
	}
	mh->msg_queue_dict = msg_queue_dict;
	char *port_s = ini_val (m_ini, MESSAGE_HANDLE_CONF[0], MESSAGE_HANDLE_CONF[1]);
	char *datadir_s = ini_val (m_ini, MESSAGE_HANDLE_CONF[0], MESSAGE_HANDLE_CONF[2]);
	if (port_s == NULL || !is_digit (port_s) || datadir_s == NULL)
	{
		goto _FAILED;
	}
	mh->port = atoi (port_s);
	memset (mh->datadir, '\0', MSG_HANDLE_SIZE);
	strncpy (mh->datadir, datadir_s, strlen (datadir_s));
	return mh;
  _FAILED:
	if (mh == NULL)
	{
		g_free (mh);
		mh = NULL;
	}
	if (msg_queue_dict != NULL)
	{
		dict_free (msg_queue_dict, NULL, NULL);
	}
	if (parition_dict != NULL)
	{
		dict_free (parition_dict, NULL, NULL);
	}
	return NULL;
}

void message_handle_free (message_handle * m)
{
	return;
}

/*
#ifndef MHANDLE_TEST
int main ()
{
	message_handle *mh = message_handle_new ("../message-server.ini");
    fprintf(stdout,"datadir = %s,port = %d\n",mh->datadir,mh->port);
}
#endif
*/
