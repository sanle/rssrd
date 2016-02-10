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
	const char *item_node_name;
	const char *timeformat;
	struct tm tm;
	time_t last_time,new_time;
	bool is_new_data = false;

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
		item_node_name = "item";
	}
	else
	{
		index = mxmlIndexNew(tree,"updated",NULL);
		timeformat ="%Y-%m-%dT%TZ";
		item_node_name = "entry";
	}
	while ((node = mxmlIndexEnum(index))!= NULL)
	{
		last=mxmlGetParent(node);
		child = mxmlGetFirstChild(node);
		while(child != NULL)
		{
			strcat(time,mxmlGetText(child,NULL));
			strcat(time," ");
			child = mxmlGetNextSibling(child);
		}
		strptime(time,timeformat,&tm);
		time[0]='\0';
		if(!strcmp(mxmlGetElement(last),item_node_name))
		{
			break;
		}
	}
	if(strcmp(mxmlGetElement(last),item_node_name))
	{
		fprintf(stderr,"Cannot find item node\n");
		exit(EXIT_FAILURE);
	}
	last_time = mktime(&tm);
	mxmlIndexDelete(index);
	if(mxmlFindElement(new_tree,new_tree,"pubDate",NULL,NULL,MXML_DESCEND) != NULL)
	{
		index = mxmlIndexNew(new_tree,"pubDate",NULL);
		timeformat ="%a, %d %b %Y %T %z";
		item_node_name = "item";
	}
	else
	{
		index = mxmlIndexNew(new_tree,"updated",NULL);
		timeformat ="%Y-%m-%dT%TZ";
		item_node_name = "entry";
	}
	while ((node = mxmlIndexEnum(index)) != NULL)
	{	
		if(strcmp(mxmlGetElement(mxmlGetParent(node)),item_node_name) != 0)
		{
			continue;
		}

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
		if(difftime(last_time,new_time) < 0)
		{
			is_new_data = true;
			mxmlAdd(mxmlGetParent(last), MXML_ADD_BEFORE, last, mxmlGetParent(node));
		}

	}
	mxmlIndexDelete(index);
	if(is_new_data)
	{
		fp = fopen(fname,"w");
		mxmlSaveFile(tree,fp,MXML_NO_CALLBACK);
		fclose(fp);
	}
	mxmlDelete(tree);
	mxmlDelete(new_tree);
}
