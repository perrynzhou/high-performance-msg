all:
	rm -rf m_test
	gcc -DHANDLE_TEST -g ../common/g_util.h ../common/g_util.c ../dict/dict.h ../dict/dict.c ../list/list.h ../list/list.c\
		../log/log.h ../log/g_log.c ../log/g_log.h ../common/g_core.h  ../ini/ini.h ../ini/ini.c message_handle.h message_handle.c -o m_test
clean:
	rm -rf m_test
