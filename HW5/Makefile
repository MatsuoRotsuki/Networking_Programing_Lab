CC = gcc

SRCCLIENT1 = client1.c

SRCCLIENT2 = client2.c

SRCSERVER = server.c

OUTCLIENT1 = client1

OUTCLIENT2 = client2

OUTSERVER = server

all: $(OUTCLIENT1)

$(OUTCLIENT1): $(SRCCLIENT1)
	$(CC) -o $(OUTCLIENT1) $(SRCCLIENT1) 
	$(CC) -o $(OUTCLIENT2) $(SRCCLIENT2)
	$(CC) -o $(OUTSERVER) $(SRCSERVER) -lssl -lcrypto

clean:
	rm -f $(OUTCLIENT1)
	rm -f $(OUTCLIENT2)
	rm -f $(OUTSERVER)

.PHONY: all clean
