all:
	@rm -rf ./bin
	@mkdir ./bin
	gcc -g  common/g_core.h common/g_util.h common/g_util.c ini/ini.h \
		ini/ini.c message-client/message_client.h  message-client/message_client.c -O2 -o bin/client
	gcc -g  common/g_core.h log/g_log.h  log/g_log.c log/log.h \
        common/g_util.h  common/g_util.c ini/ini.h ini/ini.c\
        list/list.h list/list.c dict/dict.h dict/dict.c\
        socket_poll/socket_poll.h socket_poll/socket_poll.c\
        message-handle/message_handle.h message-handle/message_handle.c\
        message/msg.h  message/msg.c message-server/message_server.h message-server/message_server.c -O2 -o bin/server -lpthread
	@cp *.ini bin/
	@echo "build server and client success"
	@cd bin
