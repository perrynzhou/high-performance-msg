/*************************************************************************
	> File Name: dict.c
	> Author: perryn
	> Mail: perrynzhou@gmail.com
	> Created Time: Sat 26 Nov 2016 10:15:06 AM EST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dict.h"
#define SUCCESS 0
#define FAILED -1
struct entry_s
{
	void *key;
	uint32_t len;				//length of key
	uint32_t hashval;			//hash code
	void *val;
	struct entry_s *next;
	struct entry_s *prev;
};
struct dict_s
{
	int64_t maxsize;
	int64_t oversize;			//
	int64_t usedsize;
	struct entry_s **data;
};
uint32_t dict_entry_index (const char *key, int32_t len, int32_t size)
{
	uint32_t b = 378551;
	uint32_t a = 63689;
	uint32_t hash = 0;
	const char *str = key;
	while (len-- > 0)
	{
		hash = hash * a + (*str++);
		a *= b;
	}
	return (uint32_t) (hash & 0x7FFFFFFF) % size;
}

uint32_t dict_gen_hash (const char *key, int32_t len)
{
	int32_t seed = 131;
	int32_t hash = 0;
	const char *str = key;
	while (len-- > 0)
	{
		hash = hash * seed + (*str++);
	}
	return (uint32_t) (hash & 0x7FFFFFFF);
}

static void dict_add_entry (dict * d, entry * t)
{
	entry **ptr = d->data + dict_entry_index (t->key, t->len, d->maxsize);
	if (((t)->next = *ptr) != NULL)
	{
		(*ptr)->prev = t;
	}
	__sync_fetch_and_add (&(d->usedsize), 1);
	*ptr = t;
}

static int dict_entry_init (dict * d, uint32_t size)
{
	entry **cur;
	d->data = cur = (entry **) malloc (size * sizeof (entry));
	if (cur == NULL)
	{
		return FAILED;
	}
	d->maxsize = size;
	d->oversize = (int64_t) (size * 0.80);
	d->usedsize = 0;
	return SUCCESS;
}

static void dict_expand (dict * d)
{
	entry *tmp = NULL;
	entry *next;
	int64_t old_size = d->maxsize;
	entry **cur_data = d->data;
	entry **old_data = cur_data;
	int64_t new_size = d->maxsize * 2;
	if (dict_entry_init (d, new_size) != FAILED)
	{
		d->maxsize = new_size;
		while (old_size-- > 0)
		{
			for (tmp = *cur_data++; tmp != NULL; tmp = next)
			{
				next = tmp->next;
				dict_add_entry (d, tmp);
			}
		}
	}
	if (old_data != NULL)
	{
		free (old_data);
		old_data = NULL;
	}
}

static entry *dict_find_entry (dict * d, void *key, int32_t key_len)
{
	entry *rs = NULL;
	entry *en = NULL;
	uint32_t idx = dict_entry_index (key, key_len, d->maxsize);
	uint32_t hv = dict_gen_hash (key, key_len);
	if (d != NULL)
	{
		for (rs = d->data[idx]; rs != NULL; rs = rs->next)
		{
			if (key_len == rs->len && hv == rs->hashval && (memcmp (rs->key, key, key_len) == 0))
			{
				en = rs;
				break;
			}
		}
	}
	return en;
}

entry *dict_insert_entry (dict * d, void *key, int32_t key_len, void *value)
{
	entry *en = dict_find_entry (d, key, key_len);
	if (en == NULL)
	{
		en = (entry *) malloc (sizeof (entry));
		en->key = malloc (key_len + 1);
		memset (en->key, '\0', key_len + 1);
		memcpy (en->key, key, key_len);
		en->len = key_len;
		en->hashval = dict_gen_hash (key, key_len);
		if (d->usedsize > d->oversize)
		{
			dict_expand (d);
		}
		dict_add_entry (d, en);
	}
	en->val = value;
	return en;
}

bool dict_delete_entry (dict * d, void *key, uint32_t key_len, void (*free_key) (void *), void (*free_val) (void *))
{
	if (d != NULL)
	{
		entry **cur = d->data + dict_entry_index (key, key_len, d->maxsize);
		entry *tmp = *cur;
		uint32_t hval = dict_gen_hash (key, key_len);
		for (; tmp; tmp = tmp->next)
		{
			if (key_len = tmp->len && hval == tmp->hashval && !memcmp (key, tmp->key, tmp->len))
			{
				if (tmp->next != NULL)
				{
					tmp->next->prev = tmp->prev;
				}
				if (tmp->prev != NULL)
				{
					tmp->prev->next = tmp->next;
				}
				else
				{
					*cur = tmp->next;
				}
				__sync_fetch_and_sub (&(d->usedsize), 1);
				if (free_key != NULL)
				{
					(*free_key) (tmp->key);
				}
				if (free_val != NULL)
				{
					(*free_val) (tmp->val);
				}
				return true;
			}
		}
	}
	return false;
}

dict *dict_new (int64_t size)
{
	dict *d = (dict *) malloc (sizeof (*d));
	if (d == NULL)
	{
		return NULL;
	}
	if (dict_entry_init (d, size) != SUCCESS)
	{
		free (d);
		d = NULL;
	}
	else
	{
		d->maxsize = size;
		d->usedsize = 0;
		d->oversize = (int64_t) (size * 0.8);
	}
	return d;
}

void *dict_find (dict * d, void *key, int32_t key_len)
{
	entry *rs = NULL;
	void *val = NULL;
	uint32_t idx = dict_entry_index (key, key_len, d->maxsize);
	uint32_t hv = dict_gen_hash (key, key_len);
	if (d != NULL)
	{
		for (rs = d->data[idx]; rs != NULL; rs = rs->next)
		{
			if (key_len == rs->len && hv == rs->hashval && (memcmp (rs->key, key, key_len) == 0))
			{
				val = rs->val;
				break;
			}
		}
	}
	return val;
}

void dict_free (dict * d, void (*free_key) (void *), void (*free_val) (void *))
{
	if (d != NULL)
	{
		int64_t i = d->maxsize;
		entry *cur;
		entry *next;
		entry **data = d->data;
		while (i-- > 0)
		{
			for (cur = *data++; cur != NULL; cur = next)
			{
				next = cur->next;
				if (free_key != NULL)
				{
					(*free_key) (cur->key);
					fprintf (stdout, " free key,");
				}
				if (free_val != NULL)
				{
					(*free_val) (cur->val);
					fprintf (stdout, "free val,");
				}
				if (cur != NULL)
				{
					free (cur);
					fprintf (stdout, "free entry\n");
				}
			}
		}
		if (data != NULL)
		{
			free (d->data);
		}
		if (d != NULL)
		{
			free (d);
		}
		d->data = NULL;
		d = NULL;
	}
}

void dict_print (dict * d)
{
	if (d != NULL)
	{
		fprintf (stdout, "dict=%p,maxsize=%d,usedsize=%d,oversize=%d,data=%p\n", d, d->maxsize, d->usedsize, d->oversize, d->data);
		entry **data = d->data;
		int64_t size = d->maxsize;
		int64_t i = 0;
		for (; i < size; i++)
		{
			entry *cur = data[i];
			while (cur != NULL)
			{
				fprintf (stdout, "entry = %p,key=%s,val=%s,hashval=%d,index=%d\n", cur, cur->key, cur->val, cur->hashval, dict_entry_index (cur->key, cur->len, d->maxsize));
				fprintf (stdout, "   ----prev=%p.next=%p\n", cur->prev, cur->next);
				cur = cur->next;
			}
		}
	}
}

#ifdef DICT_TEST
char *rand_str (int n)
{
	char *key = (char *) malloc (n + 1);
	memset (key, '\0', n + 1);
	int i = 0;
	for (; i < n; i++)
	{
		if (i % 2 == 0)
		{
			key[i] = 65 + rand () % 25;
		}
		else
		{
			key[i] = 97 + rand () % 25;
		}
	}
	return key;
}

int main (void)
{
	dict *d = dict_new (20);
	char *g_keys[20] = { NULL };
	char *g_val[20] = { NULL };
	char *g_f = NULL;
	int i = 0;
	for (; i < 20; i++)
	{
		g_keys[i] = rand_str (6);
		g_val[i] = rand_str (10);
		entry *n = dict_insert_entry (d, g_keys[i], strlen (g_keys[i]), g_val[i]);
		if (n != NULL)
		{
			fprintf (stdout, " dict_insert_entry entry = %p\n", n);
		}
		if (i == 8)
		{
			g_f = g_keys[i];
			fprintf (stdout, "--------------------find key=%p,key=%s-------------\n", g_f, g_f);
		}
	}
	fprintf (stdout, "dict = %p\n", d);
	fprintf (stdout, "  maxsize=%d,oversize=%d,usedsize=%d,data=%p\n", d->maxsize, d->oversize, d->usedsize, d->data);
	dict_print (d);
	fprintf (stdout, "------------------------------g_f =%p,g_f=%s\n", g_f, g_f);
	void *cur = dict_find (d, g_f, strlen (g_f));
	printf ("   key=%s,val=%s\n", g_f, (char *) cur);
	dict_free (d, &free, &free);
	return 0;
}
#endif
