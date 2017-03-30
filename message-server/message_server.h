/*************************************************************************
	> File Name: message_server.h
	> Author: perrynzhou
	> Mail:perrynzhou@gmail.com 
	> Created Time: Sat 18 Mar 2017 11:40:47 GMT
 ************************************************************************/
#include "../socket_poll/socket_poll.h"
#include "../common/g_core.h"
#include "../list/list.h"
/* read callback */
void read_cb (poll_event * p_evt, poll_event_element * node, struct epoll_event ev);
/* close callback */
void close_cb (poll_event * p_evt, poll_event_element * node, struct epoll_event ev);
/* accept callback */
void accept_cb (poll_event * p_evt, poll_event_element * node, struct epoll_event ev);

/* timeout callback */
int timeout_cb (poll_event * p_evt);

/* add parition to system dict */
int add_queue_to_dict (char *parition_name, char *recv_data);

/* get data for queue mode */
list_node *get_data (list_node * cur, int fd, int64_t * count);
