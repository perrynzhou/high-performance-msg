/*************************************************************************
	> File Name: mlist.h
	> Author: perrynzhou
	> Mail:perrynzhou@gmail.com 
	> Created Time: Fri 10 Mar 2017 12:20:13 GMT
 ************************************************************************/

#ifndef _MLIST_H
#define _MLIST_H
#include <stdint.h>
#include <stdbool.h>
typedef struct list_node_s list_node;
struct list_s
{
	struct list_node_s *head;
	struct list_node_s *tail;
	uint32_t size;

	void (*data_free) (void *dat);	// the free function of list_node->data
	  bool (*data_cmp) (void *src_dat, void *dst_dat);	//compare the two list_node->dat
};
typedef struct list_s list;
/* create a list */
list *list_create ();
void list_destroy (list * g);

/* listNode operation */
list_node *list_add_node_head (list * g, void *dat);
list_node *list_add_node_tail (list * g, void *dat);
list_node *list_remove_node_head (list * g);
list_node *list_remove_node_tail (list * g);
void list_remove_node (list * g, list_node * node);
void *list_node_data (list_node * ln);
list_node *list_node_prev (list_node * n);
list_node *list_node_next (list_node * n);
int32_t list_size (list * g);
#endif
