all:
	rm -rf start
	gcc -g  ../common/g_core.h ../log/g_log.h ../log/g_log.c ../log/log.h \
		../common/g_util.h ../common/g_util.c ../ini/ini.h ../ini/ini.c\
		../list/list.h ../list/list.c ../dict/dict.h ../dict/dict.c\
		../socket_poll/socket_poll.h ../socket_poll/socket_poll.c\
		../message-handle/message_handle.h ../message-handle/message_handle.c\
		../message/msg.h ../message/msg.c message_server.h message_server.c -o start -lpthread
clean:
	rm -rf start
