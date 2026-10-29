# Restaurant Order Scheduling

## About the Project
-The simulation models two chefs working simultaneously in a restaurant where orders are modelled like Operating Systems' processes. 
-Algorithms implemented for scheduling: First-Come First-Serve, Shortest Job First, Priority Scheduling
-The program uses suitable tie-breaking mechanisms

The program prints one CSV-style result row per order followed by aggregate scheduling statistics.

## How to run?

Compile with:

```sh
gcc -std=c11 -Wall -Wextra -o restaurant restaurant.c
```

The command creates an executable named `restaurant`.

Run with:

```sh
./restaurant INPUT_FILE ALGORITHM
```

`ALGORITHM` must be one of:

```text
FCFS
PRIORITY
SJF
```

Algorithm names are case sensitive.

## Input file format

The first two lines contain the number of orders and a quantum value:

```text
number_of_orders
quantum
```

Each following line describes one order using this format:

```text
ORDER_ID,ARRIVAL_TIME,PRIORITY,SERVICE_TIME,CATEGORY
```

Example:

```text
5
2
O1,0,2,8,NORMAL
O2,1,1,4,VIP
O3,2,2,5,NORMAL
O4,3,1,6,VIP
O5,4,2,3,NORMAL
```

### Input rules

- The number of orders must be between `1` and `100`.
- The quantum must be at least `1`.
- Arrival times must be non-negative.
- Priority values must be at least `1`.
- Service times must be at least `1`.
- Every order ID must be unique.
- Each order must contain all five fields.
- Order IDs and categories may contain up to 31 characters.

The current scheduler is non preemptive. The quantum is validated and read from the input, but it is not currently used by the FCFS, PRIORITY, or SJF scheduling logic.

## Scheduling behavior

The program models **two chefs**. Each chef initially becomes available at time `0`.

1. The chef with the earliest available time is selected.
2. If no waiting order has arrived yet, that chef advances to the earliest future arrival time.
3. Among orders that have arrived, the next order is selected according to the requested algorithm.
4. The selected order runs until completion; it is not interrupted.
5. The process repeats until all orders are scheduled.

### Tie-breaking rules

If two eligible orders have the same primary scheduling value, the following tie-breakers are used:

- **FCFS:** earlier arrival time, then lexicographically smaller order ID
- **PRIORITY:** lower priority value, earlier arrival time, then lexicographically smaller order ID
- **SJF:** shorter service time, earlier arrival time, then lexicographically smaller order ID

## Output format

The first output section is: 

```text
PROCESS_ID,ARRIVAL,PRIORITY,SERVICE,START,COMPLETION,WAITING,TURNAROUND,RESPONSE
```

The columns mean:

PROCESS_ID: Order identifier 
ARRIVAL: Time at which the order becomes available 
PRIORITY: Numeric priority; lower values have higher priority 
SERVICE: Required service duration 
START:Time service begins 
COMPLETION: Time service finishes 
WAITING: TURNAROUND - SERVICE
TURNAROUND: COMPLETION - ARRIVAL 
RESPONSE: START - ARRIVAL

Rows are sorted by `PROCESS_ID` in the final output, regardless of the order in which they were scheduled.

The summary section contains:

AVERAGE_WAITING : Average waiting time across all orders 
AVERAGE_TURNAROUND: Average turnaround time across all orders 
AVERAGE_RESPONSE : Average response time across all orders 
THROUGHPUT : Orders completed per unit of elapsed time 
UTILIZATION : Total service time divided by two-chef capacity over elapsed time

## Example

Command:

```sh
./restaurant input_file.txt FCFS
```

Output example:

```text
PROCESS_ID,ARRIVAL,PRIORITY,SERVICE,START,COMPLETION,WAITING,TURNAROUND,RESPONSE
O1,0,2,8,0,8,0,8,0
O2,1,1,4,1,5,0,4,0
O3,2,2,5,5,10,3,8,3
O4,3,1,6,8,14,5,11,5
O5,4,2,3,10,13,6,9,6
SUMMARY
AVERAGE_WAITING,2.80
AVERAGE_TURNAROUND,8.00
AVERAGE_RESPONSE,2.80
THROUGHPUT,0.3571
UTILIZATION,92.86%
```

## Description of functions
 
1. comes_before(): Compares two orders on the basis of a specific algorithm, and returns an integer which represents whether process a should be scheduled before process b.

Function signature:
int comes_before(const Process *a, const Process *b,
                const char *algorithm)

Requires:
- a and b must point to valid, initialized Process structures.
- algorithm must be "FCFS", "PRIORITY", or "SJF".

Returns:
- 1 if a should come before b.
- 0 otherwise.

2. find_next_process(): Finds the best process available for scheduling at the current time.

Function signature:
int find_next_process(Process orders[], int n, int current_time,
                      const char *algorithm)

Requires: 
- n must be a valid number of processes.
- current_time must represent the current scheduling time.
- algorithm must be a supported scheduling algorithm.

Returns:

- An integer index in the range 0 to n - 1 representing the selected process.
- -1 if no eligible process exists.

It follows the following logic:

1. Iterates through all processes.
2. Skips processes that have already started.
3. Skips processes whose arrival time is later than current_time.
4. Uses comes_before() to select the best remaining candidate.

3. find_next_arrival(): Finds the index of the earliest-arriving process that has not yet started. 

Function signature:
int find_next_arrival(Process orders[], int n)

Requires:
- orders must contain at least n valid process structures.
- n must be non-negative.

Returns:
- The index of a process which is yet ot start, with the earliest arrival time.
- -1 if every process has already started.

Logic:

1. Iterates through the process array.
2. Skips processes whose started flag is set.
3. Finds the process with the smallest arrival time.


4. schedule_orders(): Schedules all orders across the available chefs according to the selected scheduling algorithm.

Function signature:
int schedule_orders(Process orders[], int n, const char *algorithm)

Requires:

- orders must contain n initialized processes.
- Each process must have valid arrival and service times.
- n must be within the supported process limit.
- algorithm must be a supported algorithm.
- The process array's scheduling flags and timing fields must be initialized appropriately.

Returns:

- 1 if all processes are successfully scheduled.
- 0 if scheduling fails.

Logic:

1. Initializes each chef's availability time to 0.
2. Selects the chef who becomes available earliest.
3. Uses that chef's availability time as current_time.
4. Calls find_next_process() to select an eligible process.
5. If no process is available, calls find_next_arrival() and advances the selected chef's availability time to the next arrival.
6. Assigns the process's start and completion times.
7. Sets its remaining time to zero and marks it as started and completed.
8. Updates the chef's availability time.
9. Repeats until every process has been scheduled.

The implementation uses non-preemptive scheduling with two parallel chefs. Once an order starts, it runs until completion. The scheduling decision is made whenever a chef becomes available.

The function also implicitly assumes that chefs are interchangeable and that every order can be handled by either chef.

5. compare_ids(): Compares two processes by their IDs so that the process array can be sorted alphabetically.

Function signature:
int compare_ids(const void *a, const void *b)

Requires:

- Both arguments must point to valid Process structures, as expected by qsort().
- Their IDs must be valid null-terminated strings.

Returns:

- A negative value if the first process ID comes before the second.
- Zero if the IDs are equal.
- A positive value if the first process ID comes after the second.

This function allows the final output to be sorted by process ID.

## Known limitations

- The quantum input is currently validated but unused.
- The category field is parsed and stored but is not used to make scheduling decisions or included in the output rows.
- The program has a fixed maximum of 100 orders and exactly two chefs.