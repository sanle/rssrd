#ifndef NET_DATA_H
#define NET_DATA_H

#include <string.h>

struct MemoryStruct
{
	char *memory;
	size_t size;
};

size_t read_rss(char *ptr, size_t size, size_t nmeb, void *userdata);

int get_data(const char *str, struct MemoryStruct *mem);

long get_modify_time(const char *str);

#endif
