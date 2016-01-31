#include "net_data.h"
#include "xml_merge.h"

#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#define TIMEOUT 10


struct SourceStruct
{
	char *url;
	char *fname;
	long time;
};

int main(int argc, char **argv)
{
	FILE *stream;
	int fd;
	char buf[LINE_MAX];
	char *conf;
	char path[PATH_MAX];
	char dest_path[PATH_MAX];

	size_t source_col = 0;

	struct MemoryStruct chunk;
	
	struct SourceStruct *list;
	
	(void)argc;
	(void)argv;

	chunk.memory = NULL;
	chunk.size = 0;
	
	conf = getenv("HOME");
	if (conf == NULL)
	{
		perror("getenv");
		return EXIT_FAILURE;
	}
	strcpy(path,conf);
	strcat(path,"/.config/rssrd/destination");
	stream = fopen(path,"r");
	if(fgets(dest_path,PATH_MAX,stream) == NULL)
	{
		perror("fgets");
		fprintf(stderr,"Cannot read config file\n");
		return EXIT_FAILURE;
	}
	fclose(stream);
	if(dest_path[strlen(dest_path)-1] == '\n')
	{
		if(dest_path[strlen(dest_path)-2] == '/')
		{
			dest_path[strlen(dest_path)-1] ='\0';
		}
		else
		{
			dest_path[strlen(dest_path)-1] ='/';
		}
	}
	else
	{
		if(dest_path[strlen(dest_path)-1] != '/')
		{
			strcat(dest_path, "/");
		}

	}
	strcpy(path,conf);
	strcat(path,"/.config/rssrd/sources");
	stream = fopen(path,"r");
	if (!stream)
	{
		perror("fopen");
		return EXIT_FAILURE;
	}
	while(fgets(buf,LINE_MAX,stream)!=NULL)
	{
		++source_col;
	}
	
	rewind(stream);
	
	list = malloc(source_col * sizeof(*list));
	for(size_t i = 0; i<source_col;i++)
	{
		char *indexs;
		char *indexe;
		fgets(buf,LINE_MAX,stream);
		if(buf[strlen(buf)-1] == '\n')
		{
			list[i].url=strndup(buf,strlen(buf)-1);
		}
		else
		{
			list[i].url=strndup(buf,strlen(buf));
		}
		list[i].time=0;
		indexs = strchr(list[i].url,'/');
		indexs+=2;
		indexe = strchr(indexs, '/');
		list[i].fname=strndup(dest_path,strlen(dest_path));
		list[i].fname = realloc(list[i].fname,strlen(list[i].fname)+indexe-indexs);
		if(list[i].fname == NULL)
		{
			perror("realloc");
			return EXIT_FAILURE;
		}
		list[i].fname=strncat(list[i].fname, indexs,indexe-indexs);
		list[i].fname = realloc(list[i].fname, strlen(list[i].fname)+5);
		if(list[i].fname == NULL)
		{
			perror("realloc");
			return EXIT_FAILURE;
		}
		list[i].fname=strcat(list[i].fname,".rss");
	}
	fclose(stream);
	while(1)
	{
		for(size_t i = 0; i<source_col; i++)
		{
			long ctime = get_modify_time(list[i].url);
			if(ctime > list[i].time || ctime == -1)
			{
				list[i].time=ctime;
				chunk.size = 0;
				get_data(list[i].url,&chunk);
				if(!access(list[i].fname,F_OK))
				{
					xml_merge(list[i].fname,chunk.memory);
				}
				else
				{
					fd = open(list[i].fname,O_APPEND|O_CREAT|O_WRONLY,0644);
					if(fd == -1)
					{
						perror("open");
						return EXIT_FAILURE;
					}
					write(fd,chunk.memory,chunk.size);
					close(fd);
				}
			}
		}
		int t = TIMEOUT * 60;
		do
		{
			t = sleep(t);
		} while (t!=0);
	}
	return EXIT_SUCCESS;
}
