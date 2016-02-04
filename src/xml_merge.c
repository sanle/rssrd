#include "xml_merge.h"

#include <stdbool.h>
#include <stdio.h>

#include<mxml.h>

#define __USE_XOPEN
#include <time.h>

void xml_merge(const char *fname, void *new_data)
{
	FILE *fp;
	mxml_node_t *tree, *new_tree;
	mxml_node_t *node, *last, *child;
	mxml_index_t *index;
	char time[33]="";
	const char *timeformat;
	struct tm tm;
	time_t last_time,new_time;
	bool skip_date_parsing = false;

	fp = fopen(fname,"r");
	tree = mxmlLoadFile(NULL,fp,MXML_NO_CALLBACK);
	fclose(fp);

	new_tree = mxmlLoadString(NULL, new_data, MXML_NO_CALLBACK);
	if(tree == NULL && new_tree == NULL)
	{
		fprintf(stderr,"Do not have any data");
		exit(EXIT_FAILURE);
	}
	if(tree == NULL)
	{
		fp = fopen(fname,"w");
		mxmlSaveFile(new_tree,fp,MXML_NO_CALLBACK);
		fclose(fp);
		return;
	}
	if(mxmlFindElement(tree,tree,"pubDate",NULL,NULL,MXML_DESCEND) != NULL)
	{
		index = mxmlIndexNew(tree,"pubDate",NULL);
		timeformat ="%a, %d %b %Y %T %z";
	}
	else
	{
		index = mxmlIndexNew(tree,"updated",NULL);
		timeformat ="%Y-%m-%dT%TZ";
	}
	if ((node = mxmlIndexEnum(index))!= NULL)
	{
		last=node;
		child = mxmlGetFirstChild(node);
		while(child != NULL)
		{
			strcat(time,mxmlGetText(child,NULL));
			strcat(time," ");
			child = mxmlGetNextSibling(child);
		}
		strptime(time,timeformat,&tm);
		time[0]='\0';
	}
	else
	{
		fprintf(stderr,"Do not find pubDate node\n");
		exit(EXIT_FAILURE);
	}
	last_time = mktime(&tm);
	mxmlIndexDelete(index);
	if(mxmlFindElement(new_tree,new_tree,"pubDate",NULL,NULL,MXML_DESCEND) != NULL)
	{
		index = mxmlIndexNew(new_tree,"pubDate",NULL);
		timeformat ="%a, %d %b %Y %T %z";
	}
	else
	{
		index = mxmlIndexNew(new_tree,"updated",NULL);
		timeformat ="%Y-%m-%dT%TZ";
	}
	while ((node = mxmlIndexEnum(index)) != NULL)
	{
		child = mxmlGetFirstChild(node);
		while (child != NULL)
		{
			strcat(time,mxmlGetText(child,NULL));
			strcat(time," ");
			child = mxmlGetNextSibling(child);
		}
		strptime(time,timeformat,&tm);
		time[0]='\0';
		new_time = mktime(&tm);
		if(!skip_date_parsing && (difftime(last_time,new_time) < 0))
		{
			skip_date_parsing=true;
		}
		if(skip_date_parsing)
		{
			if(difftime(last_time,new_time) < 0)
			{
				mxmlAdd(mxmlGetParent(last), MXML_ADD_BEFORE, last, mxmlGetParent(node));
				last = mxmlGetNextSibling(last);
			}
			else
			{

				mxmlAdd(mxmlGetParent(last), MXML_ADD_AFTER, last, mxmlGetParent(node));
				last = mxmlGetPrevSibling(last);
			}
			last_time=new_time;
		}

	}
	mxmlIndexDelete(index);
	if(skip_date_parsing)
	{
		fp = fopen(fname,"w");
		mxmlSaveFile(tree,fp,MXML_NO_CALLBACK);
		fclose(fp);
	}
	mxmlDelete(tree);
	mxmlDelete(new_tree);
}
