/*************************************************************************
	> File Name: list.c
	> Author: perrynzhou
	> Mail:perrynzhou@gmail.com 
	> Created Time: Fri 10 Mar 2017 13:17:51 GMT
 ************************************************************************/

#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#define g_malloc(T) calloc(1,T)
#define g_free free
struct list_node_s
{
	struct list_node_s *prev;
	struct list_node_s *next;
	void *dat;
};

inline int32_t list_size (list * g)
{
	return g->size;
}

inline void *list_node_data (list_node * ln)
{
	return ln->dat;
}

inline list_node *list_node_prev (list_node * n)
{
	return n->prev;
}

inline list_node *list_node_next (list_node * n)
{
	return n->next;
}

static list_node *list_create_node (void *dat)
{
	if (dat == NULL)
	{
		return NULL;
	}
	list_node *node = (list_node *) g_malloc (sizeof (*node));
	if (node != NULL)
	{
		node->dat = dat;
	}
	return node;
}

inline static void list_do_node_alone (list_node * n)
{
	if (n != NULL)
	{
		n->prev = n->next = NULL;
	}
}

inline static void list_destroy_node (list * g, list_node * n)
{
	if (n != NULL)
	{
		if (n->dat != NULL && g->data_free != NULL)
		{
			(*(g->data_free)) (n->dat);
		}
		g_free (n);
	}
}

inline list *list_create ()
{
	list *g = g_malloc (sizeof (*g));
	if (g != NULL)
	{
		g->head = g->tail = NULL;
		g->size = 0;
	}
	return g;
}

void list_destroy (list * g)
{
	if (g != NULL)
	{
		list_node *cur = g->head;
		while (cur != NULL)
		{
			list_node *next = cur->next;
			list_destroy_node (g, cur);
			cur = next;
		}
	}
}

list_node *list_add_node_head (list * g, void *dat)
{
	list_node *n = list_create_node (dat);
	if (n == NULL)
	{
		return NULL;
	}
	do
	{
		list_do_node_alone (n);
		if (g->size == 0)
		{
			g->head = g->tail = n;
		}
		else
		{
			n->next = g->head;
			g->head->prev = n;
			g->head = n;
		}
	}
	while (0);
	__sync_add_and_fetch (&(g->size), 1);
	return n;
}

list_node *list_add_node_tail (list * g, void *dat)
{
	list_node *n = list_create_node (dat);
	if (n == NULL)
	{
		return NULL;
	}
	list_do_node_alone (n);
	do
	{
		if (g->size == 0)
		{
			g->head = g->tail = n;
		}
		else
		{
			g->tail->next = n;
			n->prev = g->tail;
			g->tail = n;
		}
	}
	while (0);
	__sync_add_and_fetch (&(g->size), 1);
	return n;
}

inline list_node *list_remove_node_tail (list * g)
{
	if (g->tail != NULL)
	{
		list_remove_node (g, g->tail);
	}
	return g->tail;
}

inline list_node *list_remove_node_head (list * g)
{
	if (g->head != NULL)
	{
		list_remove_node (g, g->head);
	}
	return g->head;
}

void list_remove_node (list * g, list_node * n)
{
	if (g == NULL)
	{
		return;
	}
	if (n != NULL)
	{
		if (n->prev != NULL)
		{
			n->prev->next = n->next;
		}
		else
		{
			g->head = n->next;
		}
		if (n->next != NULL)
		{
			n->next->prev = n->prev;
		}
		else
		{
			g->tail = n->prev;
		}
		list_do_node_alone (n);
		__sync_sub_and_fetch (&(g->size), 1);
	}
}

#ifdef LIST_TEST
int main (void)
{
	int arr[30];
	int i = 0;
	list *g = list_create ();
	for (; i < 30; i++)
	{
		arr[i] = i;
		list_node *cur = list_add_node_head (g, (int *) &arr[i]);
		fprintf (stdout, "cur = %p,cur->prev=%p,cur->next=%p,list = %p,size=%d,head=%p,tail=%p\n", cur, cur->prev, cur->next, g, g->size, g->head, g->tail);
	}
	list_destroy (g);
}
#endif
