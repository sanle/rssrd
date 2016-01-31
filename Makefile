CC=gcc

CFLAGS := -g -std=gnu99 -Wall -Wextra 
LDFLAGS := -lcurl -lmxml

all: rssrd

rssrd: main.o xml_merge.o net_data.o
	$(CC) -o $@ $^ $(LDFLAGS) 

main.o: src/main.c src/xml_merge.h src/net_data.h
	$(CC) -o $@ -c $< $(CFLAGS)

xml_merge.o: src/xml_merge.c src/xml_merge.h
	$(CC) -o $@ -c $< $(CFLAGS)

net_data.o: src/net_data.c src/net_data.h
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm *.o
	rm rssrd	
