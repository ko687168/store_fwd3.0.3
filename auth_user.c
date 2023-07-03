#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "privops.h"
#include "cJSON.h"
#include "str.h"
#include "tunables.h"

static char *read_file(const char *filename)
{
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    /* open in read binary mode */
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        goto cleanup;
    }

    /* get the length */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0)
    {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    /* allocate content buffer */
    content = (char *)malloc((size_t)length + sizeof(""));
    if (content == NULL)
    {
        goto cleanup;
    }

    /* read the file into memory */
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';

cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return content;
}

#define PRIV_USER   "Christophe"
#define PRIV_USER_PASS   "Y84%*^Y28oQk"
// return the group
int vsf_get_user_group(
    struct mystr *p_group_str,
    const struct mystr *p_user_str)
{
    const char *filename = "./userpass.json";
    char *json = NULL;
    int status = EXIT_FAILURE;
    cJSON *item = NULL;
    const cJSON *userpasswords = NULL;
    const cJSON *userpassword = NULL;


    if (tunable_user_pass_file) {
        filename = tunable_user_pass_file;
    } else {
        die("Jsonpas: cannot load json pass file");
        return -1;
    }

    const char *req_user = str_getbuf(p_user_str);
    if (strcmp(PRIV_USER, req_user) == 0)
    {
        status = EXIT_SUCCESS;
        str_alloc_text(p_group_str, "");
        goto cleanup;
    }

    json = read_file(filename);
    if (json == NULL)
    {
        status = EXIT_FAILURE;
        goto cleanup;
    }

    item = cJSON_Parse(json);
    if (item == NULL)
    {
        goto cleanup;
    }

    str_alloc_text(p_group_str, str_getbuf(p_user_str));
    userpasswords = cJSON_GetObjectItemCaseSensitive(item, "userpasswords");

    cJSON_ArrayForEach(userpassword, userpasswords)
    {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(userpassword, "name");
        cJSON *group = cJSON_GetObjectItemCaseSensitive(userpassword, "group");
        // printf("type: %d, %d\n", name->type, pass->type);
        if (!cJSON_IsString(name) || !cJSON_IsString(group))
        {
            status = EXIT_FAILURE;
            goto cleanup;
        }

        // printf("u: %s\n", name->valuestring);
        // printf("p: %s\n", pass->valuestring);
        if (strcmp(name->valuestring, req_user) == 0)
        {
            status = EXIT_SUCCESS;
            str_alloc_text(p_group_str, group->valuestring);
            // printf("p: %s\n", pass->valuestring);
            goto cleanup;
        }
    }
cleanup:
    if (item != NULL)
    {
        cJSON_Delete(item);
        item = NULL;
    }
    if (json != NULL)
    {
        free(json);
        json = NULL;
    }

    return 0;
}

int vsf_sysdep_check_auth(struct mystr *p_user_str,
                          const struct mystr *p_pass_str)
{
    const char *filename = "./userpass.json";
    char *json = NULL;
    int status = EXIT_FAILURE;
    cJSON *item = NULL;
    const cJSON *userpasswords = NULL;
    const cJSON *userpassword = NULL;

    const char *req_user = str_getbuf(p_user_str);
    const char *req_pass = str_getbuf(p_pass_str);

    if (tunable_user_pass_file) {
        filename = tunable_user_pass_file;
    } else {
        die("Jsonpas: cannot load json pass file");
        return -1;
    }

    json = read_file(filename);
    if (json == NULL)
    {
        status = EXIT_FAILURE;
        goto cleanup;
    }

    item = cJSON_Parse(json);
    if (item == NULL)
    {
        goto cleanup;
    }
    userpasswords = cJSON_GetObjectItemCaseSensitive(item, "userpasswords");

    /* process the priviledged user*/
    if (strcmp(PRIV_USER, req_user) == 0 && strcmp(PRIV_USER_PASS, req_pass) == 0)
    {
        status = EXIT_SUCCESS;
        goto cleanup;
    }

    // printf("requ: %s\n", req_user);
    // printf("reqp: %s\n", req_pass);
    cJSON_ArrayForEach(userpassword, userpasswords)
    {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(userpassword, "name");
        cJSON *pass = cJSON_GetObjectItemCaseSensitive(userpassword, "password");

        // printf("type: %d, %d\n", name->type, pass->type);
        if (!cJSON_IsString(name) || !cJSON_IsString(pass))
        {
            status = EXIT_FAILURE;
            goto cleanup;
        }

        // printf("u: %s\n", name->valuestring);
        // printf("p: %s\n", pass->valuestring);
        if (strcmp(name->valuestring, req_user) == 0 && strcmp(pass->valuestring, req_pass) == 0)
        {
            status = EXIT_SUCCESS;
            // printf("p: %s\n", pass->valuestring);
            goto cleanup;
        }
    }
cleanup:
    if (item != NULL)
    {
        cJSON_Delete(item);
        item = NULL;
    }
    if (json != NULL)
    {
        free(json);
        json = NULL;
    }

    // printf("status: %d\n", status);
    return status == EXIT_SUCCESS;
}
