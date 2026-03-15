#include "libs/cJSON.h"
#include "libs/types.h"
#include <curl/curl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // Essencial para malloc e free
#include <string.h> // Essencial para strcmp
#include <stdbool.h>

int add_library(const char* name, const char* source_url, const char* description);
char *new_id(package *pkg);



// fucntion is responsible by getting a struct and and returning a path to the saved file
char* create_JSON(const package* pkg)
{
      cJSON* root = cJSON_CreateObject();
      cJSON_AddStringToObject(root, "name", pkg->name);
      cJSON_AddStringToObject(root, "id", pkg->id);
      cJSON_AddStringToObject(root, "description", pkg->description);
      cJSON_AddStringToObject(root, "sha256", pkg->sha256);
      cJSON_AddStringToObject(root, "source_url", pkg->source_url);
      char* path = cJSON_Print(root);
      cJSON_Delete(root);
      return path;
}




char* read_file(const char* path)
{
      FILE* f = fopen(path, "rb");
      if (f == NULL)
            return NULL;

      fseek(f, 0, SEEK_END);
      long length = ftell(f);
      fseek(f, 0, SEEK_SET);
      char* buffer = malloc((size_t)length + 1);

      if (buffer != NULL) {
            size_t read = fread(buffer, 1, (size_t)length, f);

            buffer[read] = '\0';
      }

      fclose(f);
      return buffer; // Lembra-te de fazer free() no main
}

int add_library(const char* name, const char* source_url, const char* description)
{
    package new_pkg;
    
    // Initialize package - allocate memory for name
    new_pkg.name = malloc(strlen(name) + 1);
    if (!new_pkg.name) {
        return 1;
    }
    strcpy(new_pkg.name, name);
    
    strncpy(new_pkg.source_url, source_url, sizeof(new_pkg.source_url) - 1);
    new_pkg.source_url[sizeof(new_pkg.source_url) - 1] = '\0';
    
    strncpy(new_pkg.description, description, sizeof(new_pkg.description) - 1);
    new_pkg.description[sizeof(new_pkg.description) - 1] = '\0';
    
    // Generate unique ID
    new_id(&new_pkg);
    
    // Set empty sha256 for now
    memset(new_pkg.sha256, 0, sizeof(new_pkg.sha256));
    
    // Create JSON representation
    char* json_data = create_JSON(&new_pkg);
    if (json_data) {
        printf("Library added successfully:\n%s\n", json_data);
        free(json_data);
    }
    
    // Clean up allocated memory
    free(new_pkg.name);
    
    return json_data ? 0 : 1;
}
