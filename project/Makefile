server: server.o
	gcc -o server server.o -l pthread

client: client.o
	gcc -o client client.o -l pthread


server.o: server.c
	gcc -c server.c


client.o: client.c
	gcc -c client.c
