#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 10000

// Function to execute the hello program and capture output
char* run_hello() {
    FILE *fp;
    char buffer[128];
    char *output = malloc(128);
    if (!output) return NULL;

    fp = popen("./hello", "r");  // Run compiled hello program
    if (fp == NULL) {
        snprintf(output, 128, "Error running hello program");
        return output;
    }

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strncpy(output, buffer, 128);
    } else {
        snprintf(output, 128, "No output from hello program");
    }

    pclose(fp);
    return output;
}

// Store query param value into a string
static enum MHD_Result get_param_cb(void *cls,
                                    enum MHD_ValueKind kind,
                                    const char *key,
                                    const char *value) {
    if (strcmp(key, "extra") == 0 && value != NULL) {
        char **extra_ptr = (char **)cls;
        *extra_ptr = strdup(value);
    }
    return MHD_YES;
}

// Function to handle GET requests
enum MHD_Result answer_to_connection(void *cls,
                                     struct MHD_Connection *connection,
                                     const char *url,
                                     const char *method,
                                     const char *version,
                                     const char *upload_data,
                                     size_t *upload_data_size,
                                     void **con_cls) {
    if (strcmp(method, "GET") != 0)
        return MHD_NO;

    char *hello_output = run_hello();
    if (!hello_output) hello_output = strdup("Error running hello program");

    // Get query parameter `extra`
    char *extra = NULL;
    MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, get_param_cb, &extra);

    // Build final output
    size_t final_len = strlen(hello_output) + (extra ? strlen(extra) : 0) + 2;
    char *final_output = malloc(final_len);
    snprintf(final_output, final_len, "%s%s", hello_output, extra ? extra : "");

    free(hello_output);
    if (extra) free(extra);

    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(final_output),
        (void*) final_output,
        MHD_RESPMEM_MUST_FREE
    );

    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

int main() {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD,
                              PORT,
                              NULL,
                              NULL,
                              &answer_to_connection,
                              NULL,
                              MHD_OPTION_END);

    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Server running on http://localhost:%d\n", PORT);
    getchar();  // Keep running until Enter is pressed

    MHD_stop_daemon(daemon);
    return 0;
}
