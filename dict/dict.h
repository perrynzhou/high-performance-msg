/*************************************************************************
	> File Name: dict.h
	> Author: zhoulin
	> Mail: 715169549@qq.com
	> Created Time: Sat 26 Nov 2016 09:57:56 AM EST
 ************************************************************************/

#ifndef _dict_H
#define _dict_H
#include <stdint.h>
#include <stdbool.h>
/* define entry of dict */
typedef struct entry_s entry;
/*define dict struct */
typedef struct dict_s dict;
static dict *g_dict;
/* entry operation */
entry *dict_insert_entry (dict * d, void *key, int32_t key_len, void *value);
bool dict_delete_entry (dict * d, void *key, uint32_t key_len, void (*free_key) (void *), void (*free_val) (void *));

/* dict operation */
dict *dict_new (int64_t size);
void *dict_find (dict * d, void *key, int32_t key_len);
void dict_free (dict * d, void (*free_key) (void *), void (*free_val) (void *));
void dict_print (dict * d);
#endif
