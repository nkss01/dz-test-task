#pragma once
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <locale.h>

#ifdef _WIN32
#include "dirent.h"
#define strdup _strdup
#else
#include <dirent.h>
#include <unistd.h>
#endif

#include "textref.h"

typedef struct FileData {
    TextReference** textrefs;
    struct FileData** filerefs;
    int refcount;
    char* filepath;
    char* content;
    size_t size;
} FileData;

void filedata_free(FileData* files, int count)
{
    for (int i = 0; i < count; i++)
    {
        free(files[i].filepath);
        free(files[i].content);
        free(files[i].textrefs);
    }
    free(files);
}

char* filedata_read(const char* filepath, size_t* filesize)
{
    FILE* file = fopen(filepath, "rb");
    if (!file)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(*filesize + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, *filesize, file);
    if (bytesRead != *filesize) {
        free(buffer);
        fclose(file);
        return NULL;
    }
    buffer[*filesize] = '\0';
    fclose(file);
    return buffer;
}


int filedata_find(const char* dirpath, FileData** files, int* count, int* capacity) {
    DIR* dir = opendir(dirpath);
    if (!dir)
    {
        return EXIT_NOT_A_FOLDER;
    }

    struct dirent* entry;
    struct stat pathStat;
    char filepath[MAX_FOLDER_PATH];

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, entry->d_name);

        if (stat(filepath, &pathStat) == -1)
        {
            continue;
        }

        if (S_ISDIR(pathStat.st_mode))
        {
            if (filedata_find(filepath, files, count, capacity) != EXIT_SUCCESS)
            {
                closedir(dir);
                return EXIT_NOT_A_FOLDER;
            }
        }
        else if (S_ISREG(pathStat.st_mode))
        {
            size_t len = strlen(entry->d_name);
            if (len > 4 && strcmp(entry->d_name + len - 4, FILE_EXTENSION) == 0)
            {
                if (*count >= *capacity)
                {
                    *capacity *= 2;
                    *files = (FileData*)realloc(*files, *capacity * sizeof(FileData));
                    if (!*files)
                    {
                        closedir(dir);
                        return EXIT_MALLOC_FAILED;
                    }
                }

                (*files)[*count].filepath = strdup(filepath);
                if (!(*files)[*count].filepath)
                {
                    closedir(dir);
                    return EXIT_MALLOC_FAILED;
                }

                (*files)[*count].content = filedata_read(filepath, &(*files)[*count].size);
                if (!(*files)[*count].content) 
                {
                    free((*files)[*count].filepath);
                    (*files)[*count].filepath = NULL;
                    continue;
                }
                (*count)++;
            }
        }
    }

    closedir(dir);
    return 0;
}

FileData* filedata_get(const char* dirpath, int* count)
{
    setlocale(LC_ALL, LOCALE);
    FileData* files = NULL;
    int capacity = FILE_CAPACITY;
    files = (FileData*)malloc(capacity * sizeof(FileData));

    if (!files)
    {
        return NULL;
    }

    if (filedata_find(dirpath, &files, count, &capacity) != EXIT_SUCCESS)
    {
        free(files);
        return NULL;
    }

    return files;
}