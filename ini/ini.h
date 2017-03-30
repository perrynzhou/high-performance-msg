/*************************************************************************
	> File Name: ini.h
	> Author: perrynzhou
	> Mail: 715169549@qq.com
	> Created Time: Mon 28 Nov 2016 12:49:08 PM HKT
 ************************************************************************/

#ifndef _INI_H
#define _INI_H
#include <stdint.h>
#include <stdbool.h>
typedef struct ini_s ini;
static ini *g_ini;
ini *ini_create (const char *file);
void ini_destroy (ini * in);
bool ini_contain_section (ini * in, const char *sec_name);
char *ini_val (ini * in, const char *sec_name, const char *key);
#endif
