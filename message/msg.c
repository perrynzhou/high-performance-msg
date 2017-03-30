/*************************************************************************
	> File Name: msg.c
	> Author: perrynzhou
	> Mail:perrynzhou@gmail.com 
	> Created Time: Fri 10 Mar 2017 15:06:37 GMT
 ************************************************************************/

#include<stdio.h>
#include <stdlib.h>
#include "msg.h"
msg *msg_create (char *dat, uint32_t size)
{
	msg *m = (msg *) g_malloc (sizeof (*m));
	if (m == NULL)
	{
		return NULL;
	}
	memset (m->dat, '\0', MESSAGE_MAX_LEN);
	const char *d = (const char *) dat;
	strncpy ((char *) m->dat, (const char *) d, size);
	m->size = size;
	time_t now;
	m->timestamp = time (&now);
	return m;
}

void msg_destroy (msg * t)
{
	if (t != NULL)
	{
		g_free (t);
	}
}
