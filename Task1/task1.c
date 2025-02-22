#include "config.h"
#include "filegraph.h"
#include "file.h"

#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <sys/stat.h>
#include <string.h>

#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#endif

int handlestatus(int status);
int processdir(char* dirpath);
int inputdir(char** dirpath);
int isdir(const char* dirpath);

int main(int argc, char* argv[])
{
    char* dir = NULL;
    if (argc < 2)
    {
        int status = inputdir(&dir);
        if (status != EXIT_SUCCESS)
        {
            return handlestatus(status);
        }
    }
    else
    {
        dir = argv[1];
    }

    if (!isdir(dir))
    {
        return handlestatus(EXIT_NOT_A_FOLDER);
        if (argc < 2)
        {
            free(dir);
        }
    }

    int status = processdir(dir);

    if (argc < 2)
    {
        free(dir);
    }
    return handlestatus(status);
}

int handlestatus(int status)
{
    printf("\n\n");
    switch (status)
    {
        case EXIT_SUCCESS:
            printf("Done!");
            break;
        case EXIT_MALLOC_FAILED:
            printf("Memory allocation failed");
            break;
        case EXIT_NOT_A_FOLDER:
            printf("Not a folder");
            break;
        case EXIT_CONVERT_FAILED:
            printf("String convertion failed");
            break;
        case EXIT_FILEREAD_FAILED:
            printf("Failed to read text files");
            break;
        case EXIT_NO_TEXTFILES:
            printf("No text files found (searching for %s)", FILE_EXTENSION);
            break;
        case EXIT_WRONG_DEPENDENCY:
            printf("Wrong dependency detected!");
            break;
        default:
            printf("Unknown error");
            break;
    }

    printf("\nPress any key to continue...");
    getchar();
    return status;
}

int processdir(char* dirpath)
{
    int count = 0;
    FileData* files = filedata_get(dirpath, &count);
    if (files == NULL)
    {
        return EXIT_FILEREAD_FAILED;
    }
    if (count < 1)
    {
        return EXIT_NO_TEXTFILES;
    }

    FileGraph* graph = filegraph_new();
    if (graph == NULL)
    {
        return EXIT_MALLOC_FAILED;
    }

    for (int i = 0; i < count; i++)
    {
        FileNode* node = filenode_new(i);
        if (node == NULL)
        {
            filedata_free(files, count);
            filegraph_free(graph);
            return EXIT_MALLOC_FAILED;
        }
        filegraph_addnode(graph, node);
        filenode_setdata(node, &files[i]);
    }
    filegraph_setdependencies(graph);

    int numsorted;
    FileNode** snodes = filegraph_sort(graph, &numsorted);

    if (snodes)
    {
        for (int i = 0; i < numsorted; i++)
        {
            printf("%s\n", snodes[i]->filedata->filepath);
        }

        char* result = filenode_combinetext(snodes, numsorted);
        if (result)
        {
            printf("\nCOMBINED TEXT:\n%s", result);
        }
        else
        {
            return EXIT_MALLOC_FAILED;
        }
    }
    else
    {
        return EXIT_WRONG_DEPENDENCY;
    }

    free(snodes);
    filedata_free(files, count);
    filegraph_free(graph);
    return EXIT_SUCCESS;
}

int inputdir(char** dirpath)
{
    wchar_t buffer[MAX_FOLDER_PATH];
    wprintf(L"Enter folder name >> ");
    fgetws(buffer, MAX_FOLDER_PATH, stdin);

    size_t size = wcstombs(NULL, buffer, 0);
    if (size == (size_t)-1)
    {
        return EXIT_CONVERT_FAILED;
    }

    *dirpath = malloc(size + 1);
    if (*dirpath == NULL)
    {
        return EXIT_MALLOC_FAILED;
    }

    wcstombs(*dirpath, buffer, size + 1);
    size_t newlen = strlen(*dirpath);
    if (newlen > 0 && (*dirpath)[newlen - 1] == '\n')
    {
        (*dirpath)[newlen - 1] = '\0';
    }
    return EXIT_SUCCESS;
}

int isdir(const char* dirpath)
{
    struct stat pathstat;
    if (stat(dirpath, &pathstat) != 0)
    {
        return 0;
    }
    return S_ISDIR(pathstat.st_mode);
}