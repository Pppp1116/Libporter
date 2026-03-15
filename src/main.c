#include "libs/types.h"
#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include <uuid/uuid.h>

int download(const char* name);
int add_library(const char* name, const char* source_url, const char* description);

void help(void)
{
    printf("-help\n");
    printf("-S <name>\n");
    printf("-Syu\n");
    printf("-A <name> <source_url> <description>\n");
}

char *new_id(package *pkg)
{
    uuid_t id;
    uuid_generate_random(id);
    uuid_unparse_lower(id, pkg->id);
    return pkg->id;
}

int main(int argc, char *argv[])
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    if (argc == 2 && strcmp(argv[1], "-help") == 0)
    {
        help();
        curl_global_cleanup();
        return 0;
    }

    if (argc == 3 && strcmp(argv[1], "-S") == 0)
    {
        int rc = download(argv[2]);
        curl_global_cleanup();
        return rc;
    }

    if (argc == 5 && strcmp(argv[1], "-A") == 0)
    {
        int rc = add_library(argv[2], argv[3], argv[4]);
        curl_global_cleanup();
        return rc;
    }

    if (argc == 3 && strcmp(argv[1], "-Syu") == 0)
    {
        
    }

    help();
    curl_global_cleanup();
    return 1;
}