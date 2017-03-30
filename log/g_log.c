/*************************************************************************
	> File Name: g_log.c
	> Author: perrynzhou
	> Mail: 715169549@qq.com
	> Created Time: Tue 29 Nov 2016 05:48:39 PM HKT
 ************************************************************************/

#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include "g_log.h"
#define BUF_SIZE 1024
#define LOG_LEVEL_SIZE 5
#define SUCCESS 0
#define FAILED -1
static logger log;

const char *g_level[LOG_LEVEL_SIZE] = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };

const char *w_fmts[3] = {
	"================start log at %d-%d-%d %.2d:%.2d:%.2d,pid=[%u]================",
	"%d-%d-%d %.2d:%.2d:%.2d [%u]",
	"================end   log at %d-%d-%d %.2d:%.2d:%.2d,pid=[%u]================"
};

static void _log_time (char *buf, uint32_t len, const char *fmt)
{
	if (buf != NULL)
	{
		time_t now;
		struct tm ctime;
		memset (&ctime, 0, sizeof (ctime));
		time (&now);
		localtime_r (&now, &ctime);
		snprintf (buf, len, fmt, ctime.tm_year + 1900, ctime.tm_mon, ctime.tm_mday, ctime.tm_hour, ctime.tm_min, ctime.tm_sec, getpid ());
	}
}

void logger_disable ()
{
	log.start = false;
}

void logger_enable ()
{
	log.start = true;
}

static void _init_logger_file_path (const char *path)
{
	memset (log.cur_path, '\0', (MAX_SIZE) * 2);
	strncpy (log.cur_path, path, strlen (path));
	uint32_t len = strlen (log.cur_path);
	log.cur_path[len] = '_';
	time_t now;
	struct tm *ntm;
	time (&now);
	ntm = localtime (&now);
	len = strlen (log.cur_path);
	strftime (log.cur_path + len, (MAX_SIZE) * 2 - len, "%Y%m%d", ntm);
	len = strlen (log.cur_path);
	strncpy (log.cur_path + len, ".log", 4);
}

static int _logger_init (const char *path)
{
	char buf[BUF_SIZE] = { '\0' };
	if (path != NULL)
	{
		_init_logger_file_path (path);
		log.file = fopen (log.cur_path, "a+");
		if (log.file == NULL)
		{
			return FAILED;
		}
	}
	else
	{
		log.file = stdout;
	}
	memset (buf, '\0', BUF_SIZE);
	_log_time (buf, BUF_SIZE, w_fmts[0]);
	uint32_t len = strlen (buf);
	buf[len++] = '\n';
	int32_t count = fwrite (buf, 1, len, log.file);
	if (count != len)
	{
		if (log.file != NULL)
		{
			fclose (log.file);
		}
		return FAILED;
	}
	if (!log.file)
		fflush (log.file);
	pthread_mutex_init (&log.lock, NULL);
	return SUCCESS;
}

int logger_init (const char *path)
{
	log.file = NULL;
	log.start = false;
	if (_logger_init (path) == FAILED)
	{
		return FAILED;
	}
	log.start = true;
	return SUCCESS;
}

int logger_write (int16_t level, const char *fmt, ...)
{

	if (log.start)
	{
		pthread_mutex_lock (&log.lock);
		char buf[BUF_SIZE] = { '\0' };
		va_list ap;
		uint16_t len = 0;
		_log_time (buf, BUF_SIZE, w_fmts[1]);
		len = strlen (buf);
		char *args[3] = { "[%s]%2s", "[%s]%1s", "[%s]%2s" };
		char *_ar = NULL;
		switch (level)
		{
		case 1:
			_ar = args[0];
			break;
		case 2:
			_ar = args[1];
			break;
		default:
			_ar = args[2];
			break;
		}
		snprintf (buf + len, BUF_SIZE - len, _ar, g_level[level], "");
		va_start (ap, fmt);
		len = strlen (buf);
		vsnprintf (buf + len, BUF_SIZE - len, fmt, ap);
		va_end (ap);
		len = strlen (buf);
		buf[len++] = '\n';
		if (fwrite (buf, 1, len, log.file) != len)
		{
			pthread_mutex_unlock (&log.lock);
			if (log.file != NULL)
			{
				fclose (log.file);
			}
			return FAILED;
		}
		if (log.file != NULL)
		{
			fflush (log.file);
		}
		pthread_mutex_unlock (&log.lock);
	}

	return SUCCESS;
}

int logger_destroy ()
{
	if (log.file != NULL)
	{
		char buf[BUF_SIZE] = { '\0' };
		memset (buf, '\0', BUF_SIZE);
		_log_time (buf, BUF_SIZE, w_fmts[2]);
		uint32_t len = strlen (buf);
		buf[len++] = '\n';
		if (fwrite (buf, 1, len, log.file) != len && fflush (log.file) == 0)
		{
			return FAILED;
		}
		fclose (log.file);
		log.file = NULL;
		log.start = true;
		pthread_mutex_destroy (&log.lock);
		return SUCCESS;
	}
	return FAILED;
}
