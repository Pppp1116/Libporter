#include "libs/cJSON.h"
#include "libs/types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // Essencial para malloc e free
#include <string.h> // Essencial para strcmp

typedef struct
{
      char name[70];
      u8 id;
      char description[500];

} package;

// fucntion is responsible by getting a struct and and returning a path to the saved file
char* create_JSON(const package* pkg)
{
      cJSON* root = cJSON_CreateObject();
      cJSON_AddStringToObject(root, "name", pkg->name);
      cJSON_AddNumberToObject(root, "id", pkg->id);
      cJSON_AddStringToObject(root, "description", pkg->description);
      char* path = cJSON_Print(root);
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
