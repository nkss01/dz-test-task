#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "textref.h"

typedef struct FileNode {
    int id;
    struct FileNode** dependencies;
    int numdep;
    int visited;
    FileData* filedata;
} FileNode;

typedef struct FileGraph {
    FileNode** nodes;
    int numnodes;
} FileGraph;


FileNode* filenode_new(int id)
{
    FileNode* node = (FileNode*)malloc(sizeof(FileNode));
    if (node == NULL)
    {
        return NULL;
    }
    node->id = id;
    node->dependencies = NULL;
    node->numdep = 0;
    node->visited = 0;
    node->filedata = NULL;
    return node;
}

void filenode_setdependency(FileNode* source, FileNode* target)
{
    source->dependencies = (FileNode**)realloc(source->dependencies, (source->numdep + 1) * sizeof(FileNode*));
    source->dependencies[source->numdep++] = target;
}

void filenode_setdata(FileNode* node, struct FileData* filedata)
{
    node->filedata = filedata;
}

FileGraph* filegraph_new()
{
    FileGraph* graph = (FileGraph*)malloc(sizeof(FileGraph));
    if (graph == NULL)
    {
        return NULL;
    }
    graph->nodes = NULL;
    graph->numnodes = 0;
    return graph;
}

void filegraph_addnode(FileGraph* graph, FileNode* node)
{
    graph->nodes = (FileNode**)realloc(graph->nodes, (graph->numnodes + 1) * sizeof(FileNode*));
    graph->nodes[graph->numnodes++] = node;
}

int filegraph_sort_recursive(FileNode* node, FileNode** stack, int* index) {
    if (node->visited == 1)
    {
        return EXIT_WRONG_DEPENDENCY;
    }
    if (node->visited == 2)
    {
        return 0;
    }

    node->visited = 1;

    for (int i = 0; i < node->numdep; i++)
    {
        if (filegraph_sort_recursive(node->dependencies[i], stack, index) == EXIT_WRONG_DEPENDENCY)
        {
            printf("Wrong dependency found for: %s\n", node->filedata->filepath);
            printf("Line: %d\n", node->dependencies[i]->filedata->textrefs[i]->position);
            printf("Dependency to: %s\n\n", node->dependencies[i]->filedata->filepath);
            return EXIT_WRONG_DEPENDENCY;
        }
    }

    node->visited = 2;
    stack[(*index)++] = node;
    return 0;
}

FileNode** filegraph_sort(FileGraph* graph, int* count) {
    if (graph == NULL || graph->numnodes == 0)
    {
        *count = 0;
        return NULL;
    }

    FileNode** stack = (FileNode**)malloc(graph->numnodes * sizeof(FileNode*));
    if (stack == NULL)
    {
        return NULL;
    }

    int index = 0;
    for (int i = 0; i < graph->numnodes; i++)
    {
        if (graph->nodes[i]->visited == 0)
        {
            if (filegraph_sort_recursive(graph->nodes[i], stack, &index) == EXIT_WRONG_DEPENDENCY)
            {
                free(stack);
                *count = 0;
                return NULL;
            }
        }
    }

    *count = index;
    return stack;
}

void filenode_free(FileNode* node)
{
    if (node)
    {
        free(node->dependencies);
        free(node);
    }
}

void filegraph_free(FileGraph* graph)
{
    if (graph)
    {
        for (int i = 0; i < graph->numnodes; i++)
        {
            filenode_free(graph->nodes[i]);
        }
        free(graph->nodes);
        free(graph);
    }
}

void filegraph_setdependencies(FileGraph* graph)
{
    for (int i = 0; i < graph->numnodes; i++)
    {
        int refcount;
        FileData* fd = graph->nodes[i]->filedata;
        TextReference* references = textreference_extract(fd->content, &refcount);

        fd->refcount = 0;

        fd->textrefs = malloc(refcount * sizeof(TextReference*));
        fd->filerefs = malloc(refcount * sizeof(FileData*));

        for (int j = 0; j < refcount; j++)
        {
            for (int k = 0; k < graph->numnodes; k++)
            {
                FileData* fd2 = graph->nodes[k]->filedata;
                if (strstr(fd2->filepath, references[j].ref) != NULL)
                {
                    fd->textrefs[fd->refcount] = malloc(sizeof(TextReference));
                    if (fd->textrefs[fd->refcount] != NULL)
                    {
                        fd->textrefs[fd->refcount] = &references[j];
                        fd->filerefs[fd->refcount] = fd2;
                        filenode_setdependency(graph->nodes[i], graph->nodes[k]);
                        fd->refcount++;
                    }
                    break;
                }
            }
        }
    }
}

char* filenode_replacetext(FileNode* node)
{
    if (!node || !node->filedata || !node->filedata->content)
    {
        return NULL;
    }

    FileData* filedata = node->filedata;
    TextReference** textrefs = filedata->textrefs;
    int refcount = filedata->refcount;

    char* newcontent = malloc(filedata->size + 1);
    if (!newcontent)
    {
        return NULL;
    }

    strcpy(newcontent, filedata->content);

    char** lines = NULL;
    int numlines = 0;

    char* line = strtok(newcontent, "\n");
    while (line != NULL)
    {
        lines = realloc(lines, sizeof(char*) * (numlines + 1));
        if (!lines)
        {
            free(newcontent);
            return NULL;
        }

        lines[numlines] = strdup(line);
        numlines++;
        line = strtok(NULL, "\n");
    }

    for (int i = 0; i < refcount; ++i)
    {
        int position = textrefs[i]->position;
        if (position > 0 && position <= numlines)
        {
            int depindex = i % node->numdep;
            if (depindex < node->numdep)
            {
                FileData* depfiledata = node->dependencies[depindex]->filedata;
                if (depfiledata && depfiledata->content)
                {
                    free(lines[position - 1]);
                    lines[position - 1] = strdup(depfiledata->content);
                }
            }
        }
    }

    char* result = NULL;
    size_t resultsize = 0;

    for (int i = 0; i < numlines; ++i)
    {
        size_t linelen = strlen(lines[i]);
        result = realloc(result, resultsize + linelen + 2);
        if (!result)
        {
            free(newcontent);
            for (int j = 0; j < numlines; ++j)
                free(lines[j]);
            free(lines);
            return NULL;
        }

        strcpy(result + resultsize, lines[i]);
        resultsize += linelen;
        result[resultsize++] = '\n';
        free(lines[i]);
    }
    free(lines);
    result[resultsize - 1] = '\0';

    free(newcontent);

    return result;
}

char* filenode_combinetext(FileNode** sortedNodes, int numnodes)
{
    for (int i = 0; i < numnodes; i++)
    {
        char* text = filenode_replacetext(sortedNodes[i]);
        if (i == numnodes - 1)
        {
            return text;
        }
        sortedNodes[i]->filedata->content = text;
    }
    return NULL;
}