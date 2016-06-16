#include "net_data.h"
#include "xml_merge.h"

#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <linux/fs.h>

#define TIMEOUT 10


struct SourceStruct
{
	char *url;
	char *fname;
	long time;
};

void make_daemon(const char* path)
{
	pid_t pid;
	char fpath[PATH_MAX];
	pid = fork();
	if (pid == -1)
	{
		exit(EXIT_FAILURE);
	}
	else 
	{
		if (pid !=0)
		{
			exit(EXIT_SUCCESS);
		}
	}
	if(setsid() == -1)
	{
		exit(EXIT_FAILURE);
	}
	if(chdir("/") == -1)
	{
		exit(EXIT_FAILURE);
	}
	int max_fds = sysconf(_SC_OPEN_MAX);
	if (max_fds == -1)
	{
    		max_fds = 8192;
	}
	for (int i = 0; i< max_fds; i++)
	{
		close(i);
	}
	open("/dev/null",O_RDWR);
	dup(0);
	dup(0);
	strcpy(fpath,path);
	strcat(fpath,".error.log");
	freopen(fpath,"w+",stderr);

}

size_t read_conf(struct SourceStruct **list, char **d_path)
{
	FILE *stream;
	char buf[LINE_MAX];
	char *conf;
	char path[PATH_MAX];
	char dest_path[PATH_MAX];

	size_t source_col = 0;

	conf = getenv("HOME");
	if (conf == NULL)
	{
		perror("getenv");
		exit (EXIT_FAILURE);
	}
	strcpy(path,conf);
	strcat(path,"/.config/rssrd/destination");
	stream = fopen(path,"r");
	if (!stream)
	{
		perror("fopen");
		fprintf(stderr,"Cannot find config file (destination)\n");
		exit (EXIT_FAILURE);
	}
	if(fgets(dest_path,PATH_MAX,stream) == NULL)
	{
		perror("fgets");
		fprintf(stderr,"Cannot read config file\n");
		exit (EXIT_FAILURE);
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

	(*d_path) = strdup(dest_path);

	strcpy(path,conf);
	strcat(path,"/.config/rssrd/sources");
	stream = fopen(path,"r");
	if (!stream)
	{
		perror("fopen");
		fprintf(stderr,"Cannot find config file (sources)\n");
		exit (EXIT_FAILURE);
	}
	while(fgets(buf,LINE_MAX,stream)!=NULL)
	{
		++source_col;
	}
	if(source_col == 0)
	{
		fprintf(stderr,"Cannot find sources\n");
		exit (EXIT_FAILURE);
	}
	rewind(stream);
	
	*list = malloc(source_col * sizeof(struct SourceStruct));
	if(*list == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	for(size_t i = 0; i<source_col;i++)
	{
		char *indexs;
		char *indexe;
		fgets(buf,LINE_MAX,stream);
		if (buf[0] == '#')
		{
			--source_col;
			--i;
			continue;
		}
		if(buf[strlen(buf)-1] == '\n')
		{
			(*list)[i].url=strndup(buf,strlen(buf)-1);
		}
		else
		{
			(*list)[i].url=strndup(buf,strlen(buf));
		}
		(*list)[i].time=0;
		indexs = strchr((*list)[i].url,'/');
		indexs+=2;
		indexe = strchr(indexs, '/');
		(*list)[i].fname=strndup(dest_path,strlen(dest_path));
		(*list)[i].fname = realloc((*list)[i].fname,strlen((*list)[i].fname)+indexe-indexs);
		if((*list)[i].fname == NULL)
		{
			perror("realloc");
			exit (EXIT_FAILURE);
		}
		(*list)[i].fname=strncat((*list)[i].fname, indexs,indexe-indexs);
		(*list)[i].fname = realloc((*list)[i].fname, strlen((*list)[i].fname)+5);
		if((*list)[i].fname == NULL)
		{
			perror("realloc");
			exit (EXIT_FAILURE);
		}
		(*list)[i].fname=strcat((*list)[i].fname,".rss");
	}
	fclose(stream);
	return source_col;

}

int main(int argc, char **argv)
{
	int fd;
	int opt;

	size_t source_col;

	struct SourceStruct *list;
	struct MemoryStruct chunk;

	bool daemon = true;

	char *dest_path;

	while((opt = getopt(argc,argv,"n")) != -1)
	{
		switch (opt)
		{
			case 'n':
				daemon = false;
				break;
			default:
				fprintf(stderr,"Undefined option \n");
		}
	}

	chunk.memory = NULL;
	chunk.size = 0;
	
	source_col = read_conf(&list,&dest_path);
	if(daemon)
	{
		make_daemon(dest_path);
	}

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
