#include "net_data.h"
#include <stdlib.h>
#include <stdio.h>
#include<time.h>

#include <curl/curl.h>


size_t read_rss(char *ptr, size_t size, size_t nmeb, void *userdata)
{
	size_t realsize = size * nmeb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userdata;

	void *p= realloc(mem->memory,mem->size + realsize + 1);
	if(p == NULL)
	{
		perror("realloc");
		return -1;
	}
	mem->memory =p;
	memcpy(&mem->memory[mem->size], ptr ,realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

int get_data(char *str, struct MemoryStruct *mem)
{
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if(!curl)
	{
		fprintf(stderr,"Cannot init curl");
		return -1;
	}
	curl_easy_setopt(curl, CURLOPT_URL,str);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)mem);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_rss);
	res = curl_easy_perform(curl);
 
	if(res != CURLE_OK) 
	{
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		return -1;
	}
	curl_easy_cleanup(curl);
	return 0;
}

long get_modify_time(char *str)
{
	CURL *curl;
	CURLcode res;

	long time = 0;	

	curl = curl_easy_init();
	if(!curl)
	{
		fprintf(stderr,"Cannot init curl");
		return -1;
	}
	curl_easy_setopt(curl, CURLOPT_URL,str);
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
	curl_easy_setopt(curl, CURLOPT_FILETIME, 1L);

	res = curl_easy_perform(curl);
 
	if(res != CURLE_OK) 
	{
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		return -1;
	}
	curl_easy_getinfo(curl, CURLINFO_FILETIME,&time);
	curl_easy_cleanup(curl);
	return time;
}

