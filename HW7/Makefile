make:
	gcc server.c -w -lssl -lcrypto -o server
	gcc client.c -w -lssl -lcrypto -o client

start-server:
	./server 5500

start-client:
	./client 127.0.0.1 5500

clean:
	rm -rf server client1 client2