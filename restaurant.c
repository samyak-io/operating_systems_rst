#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100
#define NUM_CHEFS 2

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

int comes_before(const Process *a, const Process *b,
                 const char *algorithm) {
    if (strcmp(algorithm, "FCFS") == 0) {
        if (a->arrival_time != b->arrival_time)
            return a->arrival_time < b->arrival_time;
    } 
    
    else if (strcmp(algorithm, "PRIORITY") == 0) {
        if (a->priority != b->priority)
            return a->priority < b->priority;

        if (a->arrival_time != b->arrival_time)
            return a->arrival_time < b->arrival_time;
    } 
    
    else if (strcmp(algorithm, "SJF") == 0) {
        if (a->service_time != b->service_time)
            return a->service_time < b->service_time;

        if (a->arrival_time != b->arrival_time)
            return a->arrival_time < b->arrival_time;
    }
    
     return strcmp(a->id, b->id) < 0;
}

int find_next_process(Process orders[], int n, int current_time,
                      const char *algorithm) {
    int best = -1;

    for (int i = 0; i < n; i++) {
        if (orders[i].started ||
            orders[i].arrival_time > current_time) {
            continue;
        }

        if (best == -1 ||
            comes_before(&orders[i], &orders[best], algorithm)) {
            best = i;
        }
    }

    return best;
}

int find_next_arrival(Process orders[], int n) {
    int next = -1;

    for (int i = 0; i < n; i++) {
        if (orders[i].started)
            continue;

        if (next == -1 ||
            orders[i].arrival_time < orders[next].arrival_time) {
            next = i;
        }
    }

    return next;
}

int schedule_orders(Process orders[], int n, const char *algorithm) {
    int chef_available[NUM_CHEFS] = {0, 0};
    int scheduled = 0;

    while (scheduled < n) {
        int chef = 0;

        if (chef_available[1] < chef_available[0])
            chef = 1;

        int current_time = chef_available[chef];

        int next = find_next_process(
            orders, n, current_time, algorithm
        );

        if (next == -1) {
            int arrival_index = find_next_arrival(orders, n);

            if (arrival_index == -1)
                return 0;

            chef_available[chef] =
                orders[arrival_index].arrival_time;

            continue;
        }

        orders[next].start_time = current_time;
        orders[next].completion_time =
            current_time + orders[next].service_time;

        orders[next].remaining_time = 0;
        orders[next].started = 1;
        orders[next].completed = 1;

        chef_available[chef] = orders[next].completion_time;
        scheduled++;
    }

    return 1;
}

int compare_ids(const void *a, const void *b) {
    const Process *p1 = (const Process *)a;
    const Process *p2 = (const Process *)b;

    return strcmp(p1->id, p2->id);
}

// int find_next_fcfs(Process orders[], int n, int current_time) {
//     int best = -1;

//     for (int i = 0; i < n; i++) {
//         if (orders[i].started ||
//             orders[i].arrival_time > current_time) {
//             continue;
//         }

//         if (best == -1 ||
//             orders[i].arrival_time < orders[best].arrival_time ||
//             (orders[i].arrival_time == orders[best].arrival_time &&
//              strcmp(orders[i].id, orders[best].id) < 0)) {
//             best = i;
//         }
//     }
//     return best;
// }

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

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("fopen");
        return 1;
    }

    int n, quantum;

    if (fscanf(file, "%d", &n) != 1 ||
        fscanf(file, "%d", &quantum) != 1) {
        fprintf(stderr, "Error: Invalid input header\n");
        fclose(file);
        return 1;
    }

    if (n < 1 || n > MAX_PROCESSES || quantum < 1) {
        fprintf(stderr, "Error: Invalid input values\n");
        fclose(file);
        return 1;
    }

    Process orders[MAX_PROCESSES];

    for (int i = 0; i < n; i++) {
        int fields_read = fscanf(
            file, " %31[^,],%d,%d,%d,%31s",
            orders[i].id,
            &orders[i].arrival_time,
            &orders[i].priority,
            &orders[i].service_time,
            orders[i].category
        );

        if (fields_read != 5) {
            fprintf(stderr, "Error reading order %d\n", i + 1);
            fclose(file);
            return 1;
        }

        if (orders[i].arrival_time < 0 ||
            orders[i].priority < 1 ||
            orders[i].service_time < 1) {
            fprintf(stderr, "Error: Invalid values for order %s\n",
                    orders[i].id);
            fclose(file);
            return 1;
        }

        for (int j = 0; j < i; j++) {
            if (strcmp(orders[i].id, orders[j].id) == 0) {
                fprintf(stderr, "Error: Duplicate order ID '%s'\n",
                        orders[i].id);
                fclose(file);
                return 1;
            }
        }

        orders[i].remaining_time = orders[i].service_time;
        orders[i].start_time = -1;
        orders[i].completion_time = -1;
        orders[i].started = 0;
        orders[i].completed = 0;
    }

    fclose(file);

    if (!schedule_orders(orders, n, algorithm)) {
        fprintf(stderr, "Error: Scheduling failed\n");
        return 1;
    }

    double total_waiting = 0.0;
    double total_turnaround = 0.0;
    double total_response = 0.0;
    int total_service = 0;

    int first_arrival = orders[0].arrival_time;
    int last_completion = orders[0].completion_time;

    for (int i = 0; i < n; i++) {
        int turnaround =
            orders[i].completion_time - orders[i].arrival_time;

        int waiting = turnaround - orders[i].service_time;

        int response =
            orders[i].start_time - orders[i].arrival_time;

        total_waiting += waiting;
        total_turnaround += turnaround;
        total_response += response;
        total_service += orders[i].service_time;

        if (orders[i].arrival_time < first_arrival)
            first_arrival = orders[i].arrival_time;

        if (orders[i].completion_time > last_completion)
            last_completion = orders[i].completion_time;
    }

    int elapsed_time = last_completion - first_arrival;

    double throughput = 0.0;
    double utilization = 0.0;

    if (elapsed_time > 0) {
        throughput = (double)n / (double)elapsed_time;

        utilization =
            (double)total_service /
            ((double)NUM_CHEFS * (double)elapsed_time) * 100.0;
    }

    qsort(orders, n, sizeof(Process), compare_ids);

    printf("PROCESS_ID,ARRIVAL,PRIORITY,SERVICE,START,COMPLETION,WAITING,TURNAROUND,RESPONSE\n");

    for (int i = 0; i < n; i++) {
        int turnaround =
            orders[i].completion_time - orders[i].arrival_time;

        int waiting = turnaround - orders[i].service_time;

        int response =
            orders[i].start_time - orders[i].arrival_time;

        printf("%s,%d,%d,%d,%d,%d,%d,%d,%d\n",
               orders[i].id,
               orders[i].arrival_time,
               orders[i].priority,
               orders[i].service_time,
               orders[i].start_time,
               orders[i].completion_time,
               waiting,
               turnaround,
               response);
    }

    printf("SUMMARY\n");
    printf("AVERAGE_WAITING,%.2f\n", total_waiting / (double)n);
    printf("AVERAGE_TURNAROUND,%.2f\n", total_turnaround / (double)n);
    printf("AVERAGE_RESPONSE,%.2f\n", total_response / (double)n);
    printf("THROUGHPUT,%.4f\n", throughput);
    printf("UTILIZATION,%.2f%%\n", utilization);

    return 0;
}

