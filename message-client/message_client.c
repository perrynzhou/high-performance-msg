/*************************************************************************
	> File Name: message_client.c
	> Author: perrynzhou
	> Mail: 715169549@qq.com
	> Created Time: Tue 27 Dec 2016 05:21:06 PM HKT
 ************************************************************************/

#include <regex.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "message_client.h"
#include "../message/msg.h"
#include "../ini/ini.h"
#include "../common/g_util.h"
#include "../common/g_core.h"
static void usage (char *s)
{
	fprintf (stdout, "usage: %s [config_file]\n", s);
	fprintf (stdout, "    {message-client.ini}:\n");
	fprintf (stdout, "       [message-server]\n");
	fprintf (stdout, "        message-server-ip         = 127.0.0.1\n");
	fprintf (stdout, "        message-server-port       = 8080\n");
	fprintf (stdout, "        message-type              = [queue] or [topic]\n");
	fprintf (stdout, "        message-partition-name    = [name]\n");
	fprintf (stdout, "        message-publish           = [0]_(write) || [1]_(read)\n");
}

int main (int argc, char *argv[])
{

	ini *cli_ini = NULL;
	int sockfd = -1;
	if (argc != 2)
	{
		goto _FAILED;
	}
	cli_ini = ini_create (argv[1]);
	if (cli_ini == NULL)
	{
		goto _FAILED;
	}
	uint8_t type = 0;
	char *server_ip = ini_val (cli_ini, MESSAGE_SERVER_CONF[0], MESSAGE_SERVER_CONF[1]);
	char *server_port = ini_val (cli_ini, MESSAGE_SERVER_CONF[0], MESSAGE_SERVER_CONF[2]);
	char *message_type = ini_val (cli_ini, MESSAGE_SERVER_CONF[0], MESSAGE_SERVER_CONF[3]);
	char *message_parition_name = ini_val (cli_ini, MESSAGE_SERVER_CONF[0], MESSAGE_SERVER_CONF[4]);
	char *message_publish = ini_val (cli_ini, MESSAGE_SERVER_CONF[0], MESSAGE_SERVER_CONF[5]);
	fprintf (stdout, "message_parition_name  %s\n", message_parition_name);
	if (message_parition_name == NULL || message_publish == NULL || !is_digit (message_publish))
	{
		goto _FAILED;
	}
	int mp = atoi (message_publish);
	if (mp > 1 || mp < 0)
	{
		goto _FAILED;
	}
	if (!is_digit (server_port) || !is_ip (server_ip))
	{
		goto _FAILED;
	}
	if (!strncmp (message_type, MESSAGE_TYPE[0], strlen (MESSAGE_TYPE[0])) || !strncmp (message_type, MESSAGE_TYPE[1], strlen (MESSAGE_TYPE[1])))
	{
		int sockfd, n;
		char recvline[MSG_ONCE_SIZE] = { '\0' };
		char sendline[MSG_ONCE_SIZE] = { '\0' };
		struct sockaddr_in srvaddr, cliaddr;

		if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf ("create socket error: %s(errno: %d)\n", strerror (errno), errno);
			goto _FAILED;
		}

		memset (&srvaddr, 0, sizeof (srvaddr));
		srvaddr.sin_family = AF_INET;
		srvaddr.sin_port = htons (atoi (server_port));
		if (inet_pton (AF_INET, server_ip, &srvaddr.sin_addr) <= 0)
		{
			goto _FAILED;
		}
		if (connect (sockfd, (struct sockaddr *) &srvaddr, sizeof (srvaddr)) < 0)
		{
			printf ("connect error: %s(errno: %d)\n", strerror (errno), errno);
			goto _FAILED;
		}
		/* client config info to server */
		init_pkg ig;
		memset (ig.parition_name, '\0', PARITION_NAME_MAX_SIZE);
		memset (ig.type, '\0', TYPE_MAX_LEN);
		ig.is_publish = mp;
		strncpy (ig.parition_name, message_parition_name, strlen (message_parition_name));
		strncpy (ig.type, message_type, strlen (message_type));
		time_t now;
		ig.timestamp = time (&now);
		/* send first init package to server */
		char ret_buf[32] = { '\0' };
		if (send (sockfd, &ig, sizeof (ig), 0) < 0)
		{
			printf ("send error:%s\n", strerror (errno));
			goto _FAILED;
		}
		if (recv (sockfd, ret_buf, 32, 0) < 0)
		{
			printf ("recv error:%s\n", strerror (errno));
			goto _FAILED;
		}
		printf ("*********************************Handshake Success******************************\n");
		printf ("            Version    :1.0\n");
		printf ("            Author     :perrynzhou\n");
		printf ("            PartionName:%s\n", ig.parition_name);
		printf ("            Type:%s\n", ig.type);
		if (mp == 0)
		{
			printf ("            Writer     :Yes\n");
		}
		else
		{
			printf ("        Reader     :Yes\n");
		}
		printf ("*********************************Start    Request******************************\n\n");
		//begin send message to server
		switch (mp)
		{
		case 0:
			while (1)
			{
				char send_buf[MSG_ONCE_SIZE] = { '\0' };
				char recv_buf[MSG_ONCE_SIZE / 8] = { '\0' };
				fgets (send_buf, MSG_ONCE_SIZE, stdin);
				uint32_t len = strlen (send_buf);
				send_buf[len - 1] = '\0';
				len = strlen (send_buf);
				int ret = 0;
				if (len <= 0 || (ret = send (sockfd, send_buf, len, 0)) < 0)
				{
					printf ("send msg error: %s\n", strerror (errno));
					continue;
				}
				if (recv (sockfd, recv_buf, MSG_ONCE_SIZE / 8, 0) < 0)
				{
					fprintf (stdout, "recv msg error:%s", strerror (errno));
					break;
				}
				if (strncmp (recv_buf, RET_TYPE[w_n], strlen (RET_TYPE[w_n])) == 0)
				{
					fprintf (stdout, "recv message :%s ,will exit", RET_TYPE[w_n]);
					goto _FAILED;
				}

			}
		case 1:
			while (1)
			{
				msg m;
				memset (&m, '\0', sizeof (m));
				if (recv (sockfd, &m, sizeof (m), 0) < 0)
				{
					fprintf (stdout, "recv error:%s\n", strerror (errno));
					continue;
				}
				if (strlen (m.dat) > 0)
				{
					fprintf (stdout, "dat:%s,len:%d,timestamp:%d\n", m.dat, m.size, m.timestamp);
				}
				/*
				   if (strlen (m->dat) > 0)
				   {
				   fprintf (stdout, "recv data from server:%s", buf);
				   }
				 */
				/*
				   if (strlen (m.dat) > 0)
				   {
				   fprintf (stdout, "dat:%s,len:%d,timestamp:%d\n", m.dat, m.size, m.timestamp);
				   }
				   if (send (sockfd, &ig, sizeof (ig), 0) < 0)
				   {
				   fprintf (stdout, "send error:%s\n", strerror (errno));
				   }
				   sleep (1);
				 */
			}
		}
		close (sockfd);
	}
	else
	{
		goto _FAILED;
	}
	return 0;
  _FAILED:
	if (sockfd != -1)
	{
		close (sockfd);
	}
	if (cli_ini != NULL)
	{
		ini_destroy (cli_ini);
	}
	usage (argv[0]);
	return 0;
}
