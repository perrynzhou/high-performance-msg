/*************************************************************************
	> File Name: ini.c
	> Author: perrynzhou
	> Mail: 715169549@qq.com
	> Created Time: Mon 28 Nov 2016 12:55:58 PM HKT
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "ini.h"
#define BUFFER_SIZE 2048
static const char *g_split = "=";
typedef struct kv
{
	char *k;
	char *v;
	struct kv *next;
} kv;
typedef struct item_s
{
	char *section_name;
	kv *head;
	struct item_s *next;
} item;
struct ini_s
{
	uint32_t size;
	item *head;
};
static char *trim_space (char *s)
{
	if (s != NULL)
	{
		char buffer[BUFFER_SIZE] = { '\0' };
		uint32_t len = 0;
		uint32_t i = 0;
		while (*s != '\0')
		{
			if (*s != ' ')
			{
				buffer[i] = *s;
				i++;
			}
			s++;
		}
		if ((len = strlen (buffer)) > 0)
		{
			memcpy (s, buffer, len);
			s[len - 1] = '\0';	//last of line content is '\n',must be remove
		}
	}
	return s;
}

inline static bool is_section_head (char *s)
{
	if (s != NULL)
	{
		uint32_t len = strlen (s);
		if (*s == '[' && *(s + len - 1) == ']')
		{
			return true;
		}
	}
	return false;
}

static item *item_link (item * im, char *section_name)
{
	item *rs = NULL;
	if (section_name == NULL || (rs = (item *) malloc (sizeof (*rs))) == NULL)
	{
		return rs;
	}
	if (im == NULL)
	{
		im = rs;
	}
	else
	{
		rs->next = im;
		im = rs;
	}
	uint32_t len = strlen (section_name) - 2;
	im->section_name = (char *) malloc (len + 1);
	memset (im->section_name, '\0', len + 1);
	memcpy (im->section_name, section_name + 1, len);
	return im;
}

static bool kv_link (item * im, char *key, char *val)
{
	char *k = NULL;
	char *v = NULL;
	if (im != NULL && key != NULL && val != NULL)
	{
		uint32_t k_len = strlen (key);
		uint32_t v_len = strlen (val);
		k = (char *) malloc (k_len + 1);
		v = (char *) malloc (v_len + 1);
		if (k == NULL || v == NULL)
		{
			goto _ERROR;
		}
		struct kv *p = (struct kv *) malloc (sizeof (*p));
		if (p == NULL)
		{
			goto _ERROR;
		}
		memset (k, '\0', k_len + 1);
		memset (v, '\0', v_len + 1);
		memcpy (k, key, k_len);
		memcpy (v, val, v_len);
		p->k = k;
		p->v = v;
		if (im->head != NULL)
		{
			p->next = im->head;
			im->head = p;
		}
		else
		{
			im->head = p;
		}
		return true;
	}
	return false;
  _ERROR:
	if (!k)
	{
		free (k);
	}
	if (!v)
	{
		free (v);
	}
	return false;
}

ini *ini_create (const char *file)
{
	ini *in = NULL;
	FILE *f = NULL;
	if ((f = fopen (file, "r")) == NULL)
	{
		return in;
	}
	char line[BUFFER_SIZE] = { '\0' };
	item *head = NULL;
	in = (ini *) malloc (sizeof (*in));
	if (in != NULL)
	{
		while (fgets (line, BUFFER_SIZE, f) != NULL)
		{
			char *s = trim_space (line);
			uint32_t s_len = strlen (s);
			if (s_len == 0 || *s == '#')
			{
				continue;
			}
			if (is_section_head (s))
			{
				head = item_link (head, s);
				__sync_fetch_and_add (&(in->size), 1);
			}
			else
			{
				char *tmp = strstr (s, g_split);
				*tmp = '\0';
				char *v = ++tmp;
				char *k = s;
				if (!kv_link (head, k, v))
				{
					fprintf (stdout, "kv linenk error\n");
				}
			}
		}
		in->head = head;
	}
	if (f != NULL)
	{
		fclose (f);
	}
	return in;
}

void ini_destroy (ini * in)
{
	if (in != NULL)
	{
		item *im = in->head;
		while (im != NULL)
		{
			kv *cur = im->head;
			while (cur != NULL)
			{
				kv *p = cur->next;
				cur->next = NULL;
				if (cur->k != NULL)
				{
					free (cur->k);
					cur->k = NULL;
				}
				if (cur->v)
				{
					free (cur->v);
					cur->v = NULL;
				}
				if (cur != NULL)
				{
					free (cur);
					cur = NULL;
				}
				cur = p;
			}
			im = im->next;
		}
		free (in);
		in = NULL;
	}
}

bool ini_contain_section (ini * in, const char *sec_name)
{
	if (in == NULL || sec_name == NULL || strlen (sec_name) == 0)
	{
		return false;
	}
	item *cur = in->head;
	while (cur != NULL)
	{
		if (memcmp (cur->section_name, sec_name, strlen (sec_name)) == 0)
		{
			return true;
		}
		cur = cur->next;
	}
	return false;
}

char *ini_val (ini * in, const char *sec_name, const char *key)
{
	if (in == NULL || sec_name == NULL || key == NULL)
	{
		return NULL;
	}
	if (in->size > 0)
	{
		item *im = in->head;
		uint32_t key_len = strlen (key);
		uint32_t sec_len = strlen (sec_name);
		while (im != NULL)
		{
			if (memcmp (im->section_name, sec_name, sec_len) == 0)
			{
				kv *t = im->head;
				while (t != NULL)
				{
					if (memcmp (t->k, key, key_len) == 0)
					{
						return t->v;
					}
					t = t->next;
				}
			}
			im = im->next;
		}
	}
	return NULL;
}

#ifdef INI_TEST
int main (void)
{
	ini *in = ini_create ("./1.cfg");
	if (in != NULL)
	{
		item *im = in->head;
		while (im != NULL)
		{
			kv *cur = im->head;
			while (cur != NULL)
			{
				kv *p = cur->next;
				fprintf (stdout, "-----key=%s,val=%s,current = %p,next=%p\n", cur->k, cur->v, cur, cur->next);
				cur = p;
			}
			im = im->next;
		}
	}
	fprintf (stdout, " ---find val:%s(key:%s)\n", ini_val (in, "a", "a1"), "a1");
	fprintf (stdout, " ---find val:%s(key:%s)\n", ini_val (in, "a", "axx"), "axx");
	fprintf (stdout, " ---find val:%s(key:%s)\n", ini_val (in, "a", "b2"), "b2");
	fprintf (stdout, " ---find val:%s(key:%s)\n", ini_val (in, "a", "a4"), "a4");
	fprintf (stdout, " ---find val:%s(key:%s)\n", ini_val (in, "mm", "ppp"), "ppp");
	fprintf (stdout, " ---find val:%s(key:%s)\n", ini_val (in, "mm", "yyy"), "yyy");
	fprintf (stdout, " ---find val:%s(key:%s)\n", ini_val (in, "asdfas", "yyy"), "yyy");
	ini_destroy (in);
	return 0;
}
#endif
