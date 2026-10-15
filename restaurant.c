#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100

typedef struct {
    char id[32];
    int arrival_time;
    int priority;
    int service_time;
    char category[32];

    int remaining_time;
    int start_time;
    int completion_time;
    int started;
    int completed;
} Process;

int main (int argc, char *argv[]) {
    if (argc != 3){
        fprintf(
            stderr,
            "Usage: %s INPUT_FILE ALGORITHM\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *algorithm = argv[2];

    if (strcmp(algorithm, "FCFS") != 0 &&
        strcmp(algorithm, "PRIORITY") != 0 &&
        strcmp(algorithm, "SJF") != 0) {

        fprintf(stderr, "Error: Unknown algorithm '%s'\n",
                algorithm);
        return 1;
    }
    return 0;
}

