
compile: client server

client: client.cpp message.h socket_tools.cpp socket_tools.h
	g++ -o client client.cpp socket_tools.cpp

server: server.cpp message.h socket_tools.cpp socket_tools.h
	g++ -o server server.cpp socket_tools.cpp
