/*************************************************************************
    > File Name: message_server.c
    > Author: perrynzhou
    > Mail: perrynzhou@gmail.com
    > Created Time: Tue 27 Dec 2016 04:26:00 PM HKT
 ************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include "../log/log.h"
#include "../common/g_util.h"
#include "../message-handle/message_handle.h"
#include "../message/msg.h"
#include "message_server.h"
#define  MAX_CONNECTIONS_SIZE 4096
typedef struct thd_t_s
{
	pthread_t id;
	list_node *n;
	int fd;
	int64_t count;
	int64_t timestamp;
} thd_t;
/*
typedef struct thd_topic_s
{
	pthread_t id;
	list *lt;
	int fd;
	int64_t timestamp;
	int64_t count;
} thd_topic;
*/
static message_handle *g_m;
static dict *g_d_one;			//contain init_pkg for every fd
static dict *g_pos;				//contain unqiue fd,and fd write list_node postion
static dict *g_t;				//contain  do task thread for every fd
static inline char *fd_to_string (char *buf, int fd);
static void *start_func (thd_t * tq)
{
	char ip[32] = { '\0' };
	char *t_ip = get_client_ip (tq->fd, ip);
	char fd_str[8] = { '\0' };
	char *fd_ptr = fd_to_string (fd_str, tq->fd);
	list_node *cur = get_data (tq->n, tq->fd, &tq->count);
	log_info (" [Thread] %ld", pthread_self ());
	if (cur != NULL && dict_insert_entry (g_pos, fd_str, strlen (fd_str), cur) != NULL)
	{
		log_info ("****[first] send count %d,record current node =%d,val =%s*****\n", tq->count, cur, list_node_data (cur));
	}
	return NULL;
}

static void thread_write_to_client (thd_t * th)
{
	if (pthread_create (&th->id, NULL, (void *) &start_func, (void *) th) < 0)
	{
		log_error ("pthread_create:%s", strerror (errno));
	}
	if (pthread_join (th->id, NULL) < 0)
	{
		log_error ("pthread_join:%s", strerror (errno));
	}
}

/*
*/
static void message_server_init (const char *conf)
{
	if (conf == NULL || g_m == NULL)
	{
		g_m = message_handle_new (conf);
		g_d_one = dict_new (MAX_CONNECTIONS_SIZE);
		g_pos = dict_new (MAX_CONNECTIONS_SIZE);
		g_t = dict_new (MAX_CONNECTIONS_SIZE);
	}
}

static inline char *fd_to_string (char *buf, int fd)
{
	sprintf (buf, "%d", fd);
	return buf;
}

static thd_t *get_ready_thread (dict * t, char *key, int fd)
{
	uint32_t len = strlen (key);
	thd_t *tq = (thd_t *) dict_find (t, key, len);
	if (tq == NULL)
	{
		tq = (thd_t *) g_malloc (sizeof (*tq));
		if (dict_insert_entry (t, key, len, tq) != NULL)
		{
			log_info ("--first add queue thread-> fd %s to thread dict %p", key, tq);
		}
	}
	tq->fd = fd;
	tq->count = 0;
	tq->n = NULL;
	return tq;
}

list_node *get_data (list_node * n, int fd, int64_t * count)
{
	list_node *prev = NULL;
	while (n != NULL)
	{
		msg *m_ptr = list_node_data (n);
		int ret = write (fd, m_ptr, sizeof (*m_ptr));
		log_info ("get_data_for_queue ret = %d,err:%s", ret, strerror (errno));
		prev = n;
		__sync_fetch_and_add (count, 1);
		n = list_node_next (n);
	}
	return prev;
}

/*
*/
int add_queue_to_dict (char *parition_name, char *recv_data)
{
	if (g_m == NULL)
	{
		log_error ("g_m = %p", g_m);
		return -1;
	}
	msg *m = NULL;
	uint32_t len = strlen (parition_name);
	log_info ("find current parition name = %s,from client data = %s", parition_name, recv_data);
	list *val = (list *) dict_find (g_m->msg_queue_dict, parition_name, len);
	if (val == NULL)
	{
		val = list_create ();
		log_info ("not find parition_name=%s, list in dict ,create a message list %p", parition_name, val);

	}
	m = msg_create (recv_data, strlen (recv_data));
	if (m == NULL)
	{
		log_error ("create a message failed %p", m);
		return -1;
	}
	list_add_node_tail (val, m);
	if (dict_insert_entry (g_m->msg_queue_dict, parition_name, len, val) == NULL)
	{
		log_error ("add msg to list failed,msg =%p", m);
		list_destroy (val);
		msg_destroy (m);
		return -1;
	}
	log_info ("find parition_name =%s,list =%p,head=%p,tail=%p,size=%d", parition_name, val, val->head, val->tail, val->size);
	return 0;
}

void write_cb (poll_event * p_evt, poll_event_element * node, struct epoll_event ev)
{
	if (node->fd > 0)
	{
		char fd_str[8] = { '\0' };
		char *fd_ptr = fd_to_string (fd_str, node->fd);
		init_pkg *pg = (init_pkg *) dict_find (g_d_one, fd_ptr, strlen (fd_ptr));
		thd_t *tq = NULL;
		if (pg != NULL && pg->flag == 1)
		{
			if (pg->is_publish == 0)
			{
				log_info ("commit message server write to client  %d", write (node->fd, RET_TYPE[w_y], strlen (RET_TYPE[w_y])));
				pg->flag = 0;
			}
			else
			{
				int64_t count = 0;
				char cli_ip[32] = { '\0' };
				list *lt = dict_find (g_m->msg_queue_dict, pg->parition_name, strlen (pg->parition_name));
				if (lt != NULL)
				{

					if (strncmp (pg->type, MESSAGE_TYPE[0], strlen (MESSAGE_TYPE[0])) == 0)
					{
						tq = get_ready_thread (g_t, fd_str, node->fd);
						list_node *cur = NULL;
						list_node *tmp = dict_find (g_pos, fd_str, strlen (fd_str));
						if (tmp == NULL)
						{
							tq->n = lt->head;
							tq->count = 0;
							thread_write_to_client (tq);
							log_info ("****[queue-get first] record current node =%d,get message count = %d*****\n", tq->fd, tq->count);
						}
						else
						{
							if (tmp != lt->tail)
							{
								list_node *next = list_node_next (tmp);
								if (next != lt->tail && next != NULL)
								{
									tq->n = next;
									tq->count = 0;
									thread_write_to_client (tq);
									log_info ("****[queue-get not first] record current node =%d,get message count = %d*****\n", tq->fd, tq->count);
								}
							}
						}
					}
					else
					{
						tq = get_ready_thread (g_t, fd_str, node->fd);
						list_node *tmp = dict_find (g_pos, fd_str, strlen (fd_str));
						if (tmp == NULL)
						{
							tq->n = lt->tail;
							tq->count = 0;
							thread_write_to_client (tq);
						}
						else
						{
							if (tmp != lt->tail)
							{
								list_node *next = list_node_next (tmp);
								if (next != NULL)
								{
									tq->n = next;
									tq->count = 0;
									thread_write_to_client (tq);
								}
							}
						}
						//log_info ("****[topic-get] record current node =%d,get message count = %d*****\n", tq->fd, tq->count);
					}
				}
				pg->flag = 1;
			}
		}
	}

}

void read_cb (poll_event * p_evt, poll_event_element * node, struct epoll_event ev)
{
	if (node->fd > 0)
	{
		char buf[BUF_MAX_SIZE] = { '\0' };
		assert (g_d_one != NULL);
		if (read (node->fd, buf, BUF_MAX_SIZE) > 0)
		{
			char cli_ip[32] = { '\0' };
			char fd_str[8] = { '\0' };
			char *fd_ptr = fd_to_string (fd_str, node->fd);
			int len = strlen (fd_ptr);
			/*thread */
			thd_t *tq = NULL;
			init_pkg *value = (init_pkg *) dict_find (g_d_one, fd_ptr, len);
			if (value == NULL)
			{
				init_pkg *ig = (init_pkg *) buf;
				char *key = fd_str;
				init_pkg *val = g_malloc (sizeof (*val));
				val->is_exit = 1;
				strncpy (val->parition_name, ig->parition_name, strlen (ig->parition_name));
				strncpy (val->type, ig->type, strlen (ig->type));
				val->is_publish = ig->is_publish;
				if (dict_insert_entry (g_d_one, key, strlen (key), val) == NULL)
				{
					log_error ("add key %s,val %d failed", key, *val);
				}
				log_info ("new handshake to client  %s success,write size %d", get_client_ip (node->fd, cli_ip), write (node->fd, "init ok", 7));
				//get ready thread
				tq = get_ready_thread (g_t, fd_str, node->fd);
				val->flag = 0;
				if (ig->is_publish == 1)
				{

					val->flag = 1;
				}
				else
				{
					val->flag = 0;
				}
				return;
			}
			if (value->is_exit == 0)
			{
				init_pkg *ig = (init_pkg *) buf;
				init_pkg *pg = (init_pkg *) value;
				memset (pg->parition_name, '\0', PARITION_NAME_MAX_SIZE);
				strncpy (pg->parition_name, ig->parition_name, strlen (ig->parition_name));
				strncpy (pg->type, ig->type, strlen (ig->type));
				pg->is_publish = ig->is_publish;

				log_info ("parition name = %s,op_type =%d,is_online = %d,timestamp = %ld,recv data:%s", pg->parition_name, pg->is_publish, pg->is_exit, pg->timestamp, buf);
				log_info ("reused handshake to client  %s success,write size %d", get_client_ip (node->fd, cli_ip), write (node->fd, "init ok", 7));
				pg->is_exit = 1;

				//get ready thread
				tq = get_ready_thread (g_t, fd_str, node->fd);

				if (pg->is_publish == 1)
				{
					pg->flag = 1;
				}
				else
				{
					pg->flag = 0;

				}
				return;
			}
			else
			{
				if (value->is_publish == 0)
				{
					log_info ("----recv data :%s", buf);
					int ret = 0;

					if ((ret = add_queue_to_dict (value->parition_name, buf)) != -1)
					{
						log_info ("client  %s success,save pariton_name:%s,ret code:%d", get_client_ip (node->fd, cli_ip), value->parition_name, ret);
					}
					else
					{
						log_error ("client  %s success,save pariton_name:%s,ret code:%d", get_client_ip (node->fd, cli_ip), value->parition_name, ret);
					}
					value->flag = 1;
				}
				return;
			}

		}
	}
}

void close_cb (poll_event * p_evt, poll_event_element * node, struct epoll_event ev)
{
	if (node->fd > 0)
	{
		char ip[32] = { '\0' };
		char *t_ip = get_client_ip (node->fd, ip);
		char fd_str[8] = { '\0' };
		char *fd_ptr = fd_to_string (fd_str, node->fd);
		uint32_t len = strlen (fd_ptr);
		if (dict_insert_entry (g_pos, fd_str, len, NULL) != NULL)
		{
			log_info ("--clear close fd %s read node info--", fd_str);
		}
		init_pkg *val = (init_pkg *) dict_find (g_d_one, fd_ptr, len);
		thd_t *tq = dict_find (g_t, fd_str, len);
		if (tq != NULL)
		{
			log_info ("---reused %s for thread_t %p,n=%p,fd=%d----", fd_str, tq, tq->n, tq->fd);
		}
		if (val != NULL)
		{
			int old_val = val->is_exit;
			val->is_exit = 0;
			log_info ("change %s del_flag from %d to %d", fd_ptr, old_val, val->is_exit);
		}
		poll_event_remove (p_evt, node->fd);
		__sync_fetch_and_sub (&g_m->connections, 1);
		log_info ("one connection leave,current connections %d", g_m->connections);
	}
}

void accept_cb (poll_event * p_evt, poll_event_element * node, struct epoll_event ev)
{
	if (node->fd > 0)
	{
		__sync_fetch_and_add (&g_m->connections, 1);
		log_info ("one connection enter,current connections %d", g_m->connections);
		struct sockaddr_in cli_addr;
		socklen_t cli_len = sizeof (cli_addr);
		int listenfd = accept (node->fd, (struct sockaddr *) &cli_addr, &cli_len);
		uint32_t flags = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLHUP;
		poll_event_element *p;
		poll_event_add (p_evt, listenfd, flags, &p);
		p->read_callback = read_cb;
		p->write_callback = write_cb;
		p->close_callback = close_cb;

	}
}

int timeout_cb (poll_event * p_evt)
{
	if (p_evt->data != NULL)
	{
		log_info ("in timeout_cb");
		p_evt->data = g_calloc (sizeof (int));
	}
	else
	{
		int *val = (int *) p_evt->data;
		*val++;
		//log_info("timeout times %d",*val);
	}
	return 0;
}

void usage (const char *p)
{
	fprintf (stdout, "usage:%s  {conf_file}\n", p);
	fprintf (stdout, "       {message-server.ini}\n");
	fprintf (stdout, "         [message-server]\n");
	fprintf (stdout, "          server-port    = 9890\n");
	fprintf (stdout, "          server-datadir    = /tmp\n");
}

int init_socket_thread (poll_event * g)
{
	pthread_t id;
	if (pthread_create (&id, 0, (void *) &poll_event_loop, (void *) g) < 0)
	{
		log_error ("pthread_create error:%s", strerror (errno));
		return -1;
	}
	else
	{
		log_info ("create pthread for poll_event_loop");
	}
	pthread_join (id, NULL);
	return 0;
}

int main (int argc, char *argv[])
{
	message_server_init (argv[1]);
	if (g_m == NULL)
	{
		usage (argv[0]);
		return 0;
	}
	log_init (NULL);
	//message-server info
	log_info ("message-server run at port =%d", g_m->port);
	log_info ("message-handle info,max message queue %d,datadir =%s", g_m->max_size, g_m->datadir);

	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction (SIGPIPE, &sa, 0);

	int sock = socket (AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in srv_addr;
	memset (&srv_addr, 0, sizeof (srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htons (INADDR_ANY);
	srv_addr.sin_port = htons (g_m->port);
	log_info ("bind socket %d", bind (sock, (struct sockaddr *) &srv_addr, sizeof (srv_addr)));
	log_info ("listen socket %d", listen (sock, MAX_LISTEN_SIZE));
	fcntl (sock, F_SETFL, O_NONBLOCK);

	//init poll
	poll_event *g_poll = poll_event_new (1000);

	g_poll->timeout_callback = timeout_cb;
	poll_event_element *p;
	poll_event_add (g_poll, sock, EPOLLIN || EPOLLOUT, &p);
	p->accept_callback = accept_cb;
	p->read_callback = read_cb;
	p->close_callback = close_cb;
	p->write_callback = write_cb;
	p->cb_flags |= ACCEPT_CB;
	init_socket_thread (g_poll);
	//poll_event_loop (g_poll);
	return 0;
}
