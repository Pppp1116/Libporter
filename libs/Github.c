#include "libs/cJSON.h"
#include <ctype.h>
#include <curl/curl.h>
#include <pwd.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct
{
    char *data;
    size_t length;
} buffer;


static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    buffer *buf = (buffer *)userp;

    char *new_data = realloc(buf->data, buf->length + realsize + 1);
    if (!new_data)
        return 0;

    buf->data = new_data;
    memcpy(buf->data + buf->length, contents, realsize);
    buf->length += realsize;
    buf->data[buf->length] = '\0';

    return realsize;
}

static int http_get_to_buffer(const char *url, buffer *out)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return 1;

    out->data = NULL;
    out->length = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "lb/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);

    CURLcode res = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        free(out->data);
        out->data = NULL;
        out->length = 0;
        return 1;
    }

    if (http_code < 200 || http_code >= 300)
    {
        free(out->data);
        out->data = NULL;
        out->length = 0;
        return 1;
    }

    return 0;
}

static char *strip_newlines_copy(const char *src)
{
    if (!src)
        return NULL;

    size_t len = strlen(src);
    char *out = malloc(len + 1);
    if (!out)
        return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (src[i] != '\n' && src[i] != '\r')
        {
            out[j++] = src[i];
        }
    }
    out[j] = '\0';
    return out;
}

static int b64_value(unsigned char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 26;
    if (c >= '0' && c <= '9')
        return c - '0' + 52;
    if (c == '+')
        return 62;
    if (c == '/')
        return 63;
    return -1;
}

static unsigned char *base64_decode(const char *input, size_t *out_len)
{
    if (!input || !out_len)
        return NULL;

    size_t len = strlen(input);
    if (len % 4 != 0)
        return NULL;

    size_t alloc_len = (len / 4) * 3;
    if (len >= 1 && input[len - 1] == '=')
        alloc_len--;
    if (len >= 2 && input[len - 2] == '=')
        alloc_len--;

    unsigned char *out = malloc(alloc_len + 1);
    if (!out)
        return NULL;

    size_t j = 0;

    for (size_t i = 0; i < len; i += 4)
    {
        int a = b64_value((unsigned char)input[i]);
        int b = b64_value((unsigned char)input[i + 1]);
        int c = (input[i + 2] == '=') ? -2 : b64_value((unsigned char)input[i + 2]);
        int d = (input[i + 3] == '=') ? -2 : b64_value((unsigned char)input[i + 3]);

        if (a < 0 || b < 0 || (c < 0 && c != -2) || (d < 0 && d != -2))
        {
            free(out);
            return NULL;
        }

        uint32_t triple = 0;
        triple |= (uint32_t)a << 18;
        triple |= (uint32_t)b << 12;
        triple |= (uint32_t)((c >= 0) ? c : 0) << 6;
        triple |= (uint32_t)((d >= 0) ? d : 0);

        if (j < alloc_len)
            out[j++] = (triple >> 16) & 0xFF;
        if (c != -2 && j < alloc_len)
            out[j++] = (triple >> 8) & 0xFF;
        if (d != -2 && j < alloc_len)
            out[j++] = triple & 0xFF;
    }

    out[j] = '\0';
    *out_len = j;
    return out;
}

static int ensure_libported_dir(char *out_path, size_t out_path_size)
{
    struct passwd *pw = getpwuid(getuid());
    if (!pw || !pw->pw_dir)
        return 1;

    int n = snprintf(out_path, out_path_size, "%s/libported", pw->pw_dir);
    if (n < 0 || (size_t)n >= out_path_size)
        return 1;

    if (mkdir(out_path, 0755) != 0)
    {
        if (access(out_path, F_OK) != 0)
            return 1;
    }

    return 0;
}

int download(const char name[])
{
    if (!name || !name[0])
        return 1;

    char index_url[256];
    char letter = (char)tolower((unsigned char)name[0]);

    int n = snprintf(index_url,
                     sizeof(index_url),
                     "https://api.github.com/repos/Pppp1116/lb/contents/%c.json",
                     letter);
    if (n < 0 || (size_t)n >= sizeof(index_url))
        return 1;

    buffer meta_buf = {0};
    if (http_get_to_buffer(index_url, &meta_buf) != 0)
    {
        fprintf(stderr, "failed to fetch metadata index\n");
        return 1;
    }

    cJSON *github_root = cJSON_Parse(meta_buf.data);
    if (!github_root)
    {
        fprintf(stderr, "failed to parse GitHub wrapper JSON\n");
        free(meta_buf.data);
        return 1;
    }

    cJSON *content = cJSON_GetObjectItemCaseSensitive(github_root, "content");
    if (!cJSON_IsString(content) || !content->valuestring)
    {
        fprintf(stderr, "missing GitHub content field\n");
        cJSON_Delete(github_root);
        free(meta_buf.data);
        return 1;
    }

    char *base64_clean = strip_newlines_copy(content->valuestring);
    if (!base64_clean)
    {
        fprintf(stderr, "failed to normalize base64\n");
        cJSON_Delete(github_root);
        free(meta_buf.data);
        return 1;
    }

    size_t decoded_len = 0;
    unsigned char *decoded = base64_decode(base64_clean, &decoded_len);
    free(base64_clean);
    cJSON_Delete(github_root);
    free(meta_buf.data);

    if (!decoded)
    {
        fprintf(stderr, "failed to decode base64 content\n");
        return 1;
    }

    cJSON *db_root = cJSON_ParseWithLength((const char *)decoded, decoded_len);
    if (!db_root)
    {
        fprintf(stderr, "failed to parse decoded package database JSON\n");
        free(decoded);
        return 1;
    }

    cJSON *pkg = cJSON_GetObjectItemCaseSensitive(db_root, name);
    if (!pkg)
    {
        fprintf(stderr, "package '%s' not found\n", name);
        cJSON_Delete(db_root);
        free(decoded);
        return 1;
    }

    cJSON *source_url = cJSON_GetObjectItemCaseSensitive(pkg, "source_url");
    if (!cJSON_IsString(source_url) || !source_url->valuestring)
    {
        fprintf(stderr, "package '%s' has no valid source_url\n", name);
        cJSON_Delete(db_root);
        free(decoded);
        return 1;
    }

    buffer file_buf = {0};
    if (http_get_to_buffer(source_url->valuestring, &file_buf) != 0)
    {
        fprintf(stderr, "failed to download source_url: %s\n", source_url->valuestring);
        cJSON_Delete(db_root);
        free(decoded);
        return 1;
    }

    char dir_path[512];
    if (ensure_libported_dir(dir_path, sizeof(dir_path)) != 0)
    {
        fprintf(stderr, "failed to create ~/libported\n");
        free(file_buf.data);
        cJSON_Delete(db_root);
        free(decoded);
        return 1;
    }

    char file_path[1024];
    n = snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, name);
    if (n < 0 || (size_t)n >= sizeof(file_path))
    {
        fprintf(stderr, "output path too long\n");
        free(file_buf.data);
        cJSON_Delete(db_root);
        free(decoded);
        return 1;
    }

    FILE *f = fopen(file_path, "wb");
    if (!f)
    {
        perror("fopen");
        free(file_buf.data);
        cJSON_Delete(db_root);
        free(decoded);
        return 1;
    }

    size_t written = fwrite(file_buf.data, 1, file_buf.length, f);
    fclose(f);

    if (written != file_buf.length)
    {
        fprintf(stderr, "failed to write complete file\n");
        free(file_buf.data);
        cJSON_Delete(db_root);
        free(decoded);
        return 1;
    }

    printf("saved '%s' to %s\n", name, file_path);

    free(file_buf.data);
    cJSON_Delete(db_root);
    free(decoded);
    return 0;
}

