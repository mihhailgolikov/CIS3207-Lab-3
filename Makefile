server: server.c open_listenfd.c server.h
	gcc -o mishaserver server.c open_listenfd.c -lpthread