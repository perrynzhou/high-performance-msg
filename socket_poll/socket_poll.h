/*************************************************************************
	> File Name: pool.h
	> Author: perrynzhou
	> Mail: 715169549@qq.com
	> Created Time: Tue 27 Dec 2016 12:59:20 PM HKT
 ************************************************************************/

#ifndef _POLL_H
#define _POLL_H
#include "../common/g_core.h"
#include "uthash.h"
#define ACCEPT_CB 0x01
#define CONNECT_CB 0x02
typedef struct poll_event_element_s poll_event_element;
typedef struct poll_event_s poll_event;
struct poll_event_element_s
{
	int fd;
	void (*close_callback) (poll_event *, poll_event_element *, struct epoll_event);
	void (*read_callback) (poll_event *, poll_event_element *, struct epoll_event);
	void (*write_callback) (poll_event *, poll_event_element *, struct epoll_event);
	void (*accept_callback) (poll_event *, poll_event_element *, struct epoll_event);
	void (*connect_callback) (poll_event *, poll_event_element *, struct epoll_event);
	void *data;
	uint32_t events;
	uint32_t cur_event;
	uint8_t cb_flags;
	//
	UT_hash_handle hh;

};
struct poll_event_s
{
	int (*timeout_callback) (poll_event *);
	size_t timeout;
	int epoll_fd;
	void *data;
};

poll_event_element *poll_event_element_new (int fd, uint32_t events);
void poll_event_element_delete (poll_event_element * elem);
poll_event *poll_event_new (int timeout);
int poll_event_add (poll_event * p_event, int fd, uint32_t flags, poll_event_element ** p_element);
void poll_event_delete (poll_event * p_event);
int poll_event_remove (poll_event * p_event, int fd);
int poll_event_process (poll_event * p_event);
void poll_event_loop (poll_event * p_event);
#endif
