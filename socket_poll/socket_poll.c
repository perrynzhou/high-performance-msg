/*************************************************************************
	> File Name: poll.c
	> Author: perrynzhou
	> Mail: 715169549@qq.com
	> Created Time: Tue 27 Dec 2016 02:35:15 PM HKT
 ************************************************************************/

#include "../log/log.h"
#include "../log/g_log.h"
#include "socket_poll.h"
#include "uthash.h"
#define MAX_EVENTS_SIZE 1024
static poll_event_element *_nodes;

poll_event_element *poll_event_element_new (int fd, uint32_t events)
{
	poll_event_element *pem = (poll_event_element *) g_calloc (sizeof (*pem));
	if (pem == NULL)
	{
		log_error ("%s", "create a poll_event_element failed");
		return NULL;
	}
	pem->fd = fd;
	pem->events = events;
	log_info ("create poll_event_element %p success", pem);
	return pem;
}

void poll_event_element_delete (poll_event_element * elem)
{
	if (elem != NULL)
	{
		log_info ("free poll_event_element %p success", elem);
		g_free (elem);
		elem = NULL;
	}
}

poll_event *poll_event_new (int timeout)
{
	_nodes = NULL;
	poll_event *pet = (poll_event *) g_calloc (sizeof (*pet));
	if (!pet)
	{
		log_error ("new poll_event failed %s", strerror (errno));
		return NULL;
	}
	pet->timeout = timeout;
	pet->epoll_fd = epoll_create (MAX_EVENTS_SIZE);
	log_info ("new poll_event %p success", pet);
	return pet;
}

void poll_event_delete (poll_event * p_event)
{
	if (p_event != NULL)
	{
		close (p_event->epoll_fd);
		g_free (p_event);
		log_info ("%s", "free poll_event success");
	}
}

int poll_event_add (poll_event * p_event, int fd, uint32_t flags, poll_event_element ** p_element)
{
	poll_event_element *elem = NULL;
	HASH_FIND_INT (_nodes, &fd, elem);
	if (elem != NULL)
	{
		log_info ("fd(%d) already added updating flags", fd);
		elem->events |= flags;
		struct epoll_event ev;
		memset (&ev, 0, sizeof (ev));
		ev.data.fd = fd;
		ev.events = elem->events;
		*p_element = elem;
		return epoll_ctl (p_event->epoll_fd, EPOLL_CTL_MOD, fd, &ev);
	}
	else
	{
		elem = poll_event_element_new (fd, flags);
		HASH_ADD_INT (_nodes, fd, elem);
		log_info ("added fd(%d) ", fd);
		struct epoll_event ev;
		memset (&ev, 0, sizeof (ev));
		ev.data.fd = fd;
		ev.events = elem->events;
		*p_element = elem;
		return epoll_ctl (p_event->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
	}
}

int poll_event_remove (poll_event * p_event, int fd)
{

	poll_event_element *elem = NULL;
	HASH_FIND_INT (_nodes, &fd, elem);
	if (elem != NULL)
	{
		log_info ("poll_event_remove fd(%d)", fd);
		HASH_DEL (_nodes, elem);
		close (fd);
		epoll_ctl (p_event->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	}
	return 0;
}

int poll_event_process (poll_event * p_event)
{
	struct epoll_event events[MAX_EVENTS_SIZE];
	int fds = epoll_wait (p_event->epoll_fd, events, MAX_EVENTS_SIZE, p_event->timeout);
	if (fds == 0)
	{
		//log_info ("%s", "event loop timeout");
		if (p_event->timeout_callback != NULL)
		{
			if (p_event->timeout_callback (p_event) != 0)
			{
				return -1;
			}
		}
	}
	int i = 0;
	for (; i < fds; i++)
	{
		poll_event_element *val = NULL;
		HASH_FIND_INT (_nodes, &events[i].data.fd, val);
		if (val != NULL)
		{
			if ((events[i].events & EPOLLIN) || (events[i].events & EPOLLPRI))
			{
				if (events[i].events & EPOLLIN)
				{
					//log_info ("found EPOLLIN for event i(%d) and sock(%d)", i, events[i].data.fd);
					val->cur_event &= EPOLLIN;
				}
				else
				{
					//log_info ("found EPOLLPRI for event i(%d) and sock(%d)", i, events[i].data.fd);
					val->cur_event &= EPOLLPRI;
				}
				if ((val->cb_flags & ACCEPT_CB) && (val->accept_callback))
				{

					val->accept_callback (p_event, val, events[i]);
				}
				if ((val->cb_flags & CONNECT_CB) && (val->connect_callback))
				{
					val->connect_callback (p_event, val, events[i]);
				}
				if (val->read_callback != NULL)
				{

					val->read_callback (p_event, val, events[i]);
				}
			}
			//write possible
			if (events[i].events & EPOLLOUT)
			{
				//log_info ("found EPOLLOUT for event i(%d) and sock(%d)", i, events[i].data.fd);
				val->cur_event &= EPOLLOUT;
				if (val->write_callback != NULL)
				{

					val->write_callback (p_event, val, events[i]);
				}
			}
			//shutdown or error
			if ((events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))
			{
				if (events[i].events & EPOLLRDHUP)
				{
					log_info ("found EPOLLRDHUP for event i(%d) and sock(%d)", i, events[i].data.fd);
					val->cur_event &= EPOLLRDHUP;
				}
				else
				{
					log_info ("found EPOLLRERR for event i(%d) and sock(%d)", i, events[i].data.fd);
					val->cur_event &= EPOLLERR;
				}
				if (val->close_callback != NULL)
				{

					val->close_callback (p_event, val, events[i]);
				}
			}
		}
		else
		{
			log_warning ("not found in hashmap for event i(%d) and sock(%d)", i, events[i].data.fd);
		}
	}
	return 0;
}

void poll_event_loop (poll_event * p_event)
{
	while (!poll_event_process (p_event)) ;
}
