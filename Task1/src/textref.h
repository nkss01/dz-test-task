#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

typedef struct {
    int position;
    char* ref;
} TextReference;

TextReference* textreference_extract(const char* text, int* count)
{
    TextReference* textrefs = NULL;
    *count = 0;

    char* textcpy = strdup(text);
    char* line = strtok(textcpy, "\n");
    int linenum = 0;

    while (line != NULL)
    {
        linenum++;
        
        char* refpos = strstr(line, TEXTREF_BEGIN);
        if (refpos != NULL)
        {
            char* begin = refpos + strlen(TEXTREF_BEGIN);
            char* end = strchr(begin, TEXTREF_END);
            
            if (end != NULL)
            {
                textrefs = realloc(textrefs, (*count + 1) * sizeof(TextReference));
                if (textrefs == NULL)
                {
                    free(textcpy);
                    return NULL;
                }
                
                textrefs[*count].position = linenum;
                size_t length = end - begin;
                textrefs[*count].ref = malloc(length + 1);
                if (textrefs[*count].ref == NULL)
                {
                    free(textcpy);
                    free(textrefs);
                    return NULL;
                }
                
                strncpy(textrefs[*count].ref, begin, length);
                textrefs[*count].ref[length] = '\0';
                (*count)++;
            }
        }
       
        line = strtok(NULL, "\n");
    }
    
    free(textcpy);
    return textrefs;
}