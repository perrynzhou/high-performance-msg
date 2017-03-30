/*************************************************************************
	> File Name: g_util.c
	> Author: perrynzhou
	> Mail: perrynzhou@gmail.com
	> Created Time: Fri 23 Dec 2016 09:16:06 AM EST
 ************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <regex.h>
#include <time.h>
#include "g_util.h"
#include "g_core.h"
bool is_ip (const char *ip)
{
	int cflags = REG_EXTENDED;
	regmatch_t pmatch[1];
	const size_t nmatch = 1;
	regex_t reg;
	const char *pattern = "^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$";
	regcomp (&reg, pattern, cflags);
	bool ret = true;
	if (regexec (&reg, ip, nmatch, pmatch, 0) == REG_NOMATCH)
	{
		ret = false;
	}
	regfree (&reg);
	return ret;
}

bool is_digit (char *s)
{
	if (s == NULL)
		return false;
	uint32_t len = strlen (s);
	uint32_t i = 0;
	for (; i < len; i++)
	{
		if (isdigit (s[i]) == 0)
		{
			return false;
		}
	}
	return true;
}

//convert client fd to ip address
char *get_client_ip (int fd, char *ip_buf)
{
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof (struct sockaddr_in);
	int res = getpeername (fd, (struct sockaddr *) &addr, &addr_size);
	if (res == -1)
	{
		return NULL;
	}
	strcpy (ip_buf, inet_ntoa (addr.sin_addr));
	return ip_buf;
}

char *trim_string (const char *src, int len, char *target)
{
	if (src == NULL)
	{
		return NULL;
	}
	if (target == NULL)
	{
		target = (char *) calloc (1, len + 1);
		if (target == NULL)
			return NULL;

	}
	int i = 0;
	while (*src != '\0')
	{
		if (*src != ' ')
		{
			target[i] = *src;
			i++;
		}
		src++;
	}
	return target;
}

#ifdef UTIL_TEST
int main ()
{
	char *s[5] = { "127.0.0.1", "localhost", "192.1.1.9", "167.aaa.d.12", "104.99.68.35" };
	int i = 0;
	for (; i < 5; i++)
	{
		fprintf (stdout, "%s = %d\n", s[i], is_ip (s[i]));
	}
	const char *src = "    --- port =   ";
	char buf[64] = { '\0' };
	char *target = trim_string (src, strlen (src), buf);
	fprintf (stdout, " target =[%s],src=%s\n", target, src);
	return 0;
}
#endif
