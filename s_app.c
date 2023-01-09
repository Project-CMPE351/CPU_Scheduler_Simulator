#include <stdio.h>
#include <stdlib.h>
// #include <getopt.h>
#include <string.h>

typedef struct process {
    int burst_time;
    int arrival_time;
    int title;
    int priority;
    int start_time;
    int wait_time;
    bool done;
    struct process *next;  
} Process;

static int num = 1; // Assign an id to a newly created process

void printNode(Process *node) {
    printf("P%d: Burst: %d, arrival: %d, priority: %d\n",node->title,node->burst_time,node->arrival_time,node->priority);
}

Process *createNode(int b_t, int a_t, int priority) {
    Process *node = (Process*)malloc(sizeof(Process));
    node->burst_time = b_t;
    node->arrival_time = a_t;
    node->priority = priority;
    node->done = false;
    node->start_time = -1;
    node->wait_time = 0;
    node->title = num; // assign title to each process
    num++; // increment num after each process is created
    node->next = NULL;
    return node;
}

void insertSorted(Process **head, int b_t, int a_t, int priority) {
    Process *node = createNode(b_t, a_t, priority);
    if (*head == NULL) {
        *head = node;
        return;
    }
    Process *current = *head;
    while (current->next != NULL && current->next->arrival_time <= a_t) {
        current = current->next;
    }
    if (current->next != NULL){
        Process *next = current->next;
        node->next = next;
    }
    current->next = node;
    return;
}

Process *findMinBurstNode(Process *head, int current_time) {
    Process *current = head;
    Process *min_node = current;
    int min_burst_time = current->burst_time;
    while (current != NULL && current->arrival_time <= current_time) {
        if (!current->done && current->burst_time < min_burst_time) {
            min_node = current;
            min_burst_time = current->burst_time;
        }
        if (current->done && min_node == current && current->next != NULL) min_node = current->next;
        current = current->next;
    }
    return min_node;
}

Process *removeNode(Process *head, Process *node) {
    if (head == NULL) return NULL;
    if (head == node) {
        Process *new_head = head->next;
        free(head);
        return new_head;
    }
    Process *current = head;
    while (current->next != NULL && current->next != node) {
        current = current->next;
    }
    if (current->next == node) {
        Process *next = current->next->next;
        free(current->next);
        current->next = next;
    }
    return head;
}

void display(Process *header, FILE *output_file, char *description) {
    Process *current = header;
    printf("Scheduling Method: %s\n", description);
    printf("Process Waiting Times:\n");
    int sum_wait_time = 0;
    int count = 0;
    while (current != NULL) {
        int wait_time = current->start_time - current->arrival_time + current->wait_time;
        sum_wait_time += wait_time;
        printf("P%d %d ms\n", current->title, wait_time);
        current = current->next; //print title
        count++;
    }
    printf("Average Waiting Time: %.1f ms", (double) sum_wait_time / count);
}

void DoFCFS(Process *head) {
    Process *current = head;
    int time_passed = 0;
    while (current != NULL) {
        if (current->arrival_time > time_passed) time_passed = current->arrival_time;
        current->start_time = time_passed;
        time_passed += current->burst_time;
        current = current->next;
    }
}

// Non preemptive
void DoSJF(Process *head) {
    int time_passed = 0;
    while (1) {
        Process *min_node = findMinBurstNode(head, time_passed);
        if (min_node->arrival_time > time_passed) time_passed = min_node->arrival_time;
        else min_node->start_time = time_passed;
        time_passed += min_node->burst_time;
        min_node->done = true;
        Process *node = head;
        bool done = true;
        while (node != NULL) {
            if (!node->done) done = false;
            node = node->next;
        }
        if (done) break;
    }
}

void DoSJFP(Process *head) {
    int time_passed = 0;
    while (1){
        // printf("Time passed: %d\n", time_passed);
        Process *min_node = findMinBurstNode(head, time_passed);
        if (!min_node->done) {
            if (min_node->start_time == -1) min_node->start_time = time_passed;
            if (min_node->burst_time <= 1) min_node->done = true;
            min_node->burst_time--;
        }
        time_passed++;
        // printNode(min_node);
        Process *node = head;
        bool done = true;
        while (node != NULL) {
            if (!node->done) {
                if (node != min_node && node->start_time > -1) node->wait_time++;
                done = false;
            }
            node = node->next;
        }
        if (done) return;
    }
}
void doRoundRobin(Process *head) {
    // Create a queue to hold the processes
    Process *queue = NULL;
    // Add all the processes to the queue
    Process *current = head;
    while (current != NULL) {
        insertSorted(&queue, current->burst_time, current->arrival_time, current->priority);
        current = current->next;
    }
    // Set the time passed to 0
    int time_passed = 0;
    // Set the current process to the first process in the queue
    current = queue;
    // Run the scheduler loop until all processes are done
    while (current != NULL) {
        // If the current process has not started yet, set its start time
        if (current->start_time == -1) {
            current->start_time = time_passed;
        }
        // Run the current process for the time quantum
        current->burst_time -= 2;
        time_passed += 2;
        // If the current process has completed, mark it as done and remove it from the queue
        if (current->burst_time <= 0) {
            current->done = true;
            queue = removeNode(queue, current);
        }
        // If the current process has not completed, requeue it at the end of the queue
        else {
            insertSorted(&queue, current->burst_time, current->arrival_time, current->priority);
            queue = removeNode(queue, current);
        }
        // Set the current process to the next process in the queue
        current = queue;
    }
    // Print the results of the scheduling
}


int main(int argc, char *argv []) {
    //Open input file
    FILE *input_file = fopen("input.txt", "r");
    if (input_file == NULL) {
        printf("Error opening input file\n");
        return 1;
    }
   
    // Open output file
    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        printf("Error opening output file\n");
        return 1;
    }

    // Read data from file and parse into linked list
    Process *head = NULL;
    char *token;
    int burst,arrival,priority;

    while (fscanf(input_file, "%d:%d:%d\n", &burst, &arrival, &priority) != EOF) {
        insertSorted(&head, burst, arrival, priority);
    }

    // Test read processes
    Process *reader = head;
    while (reader != NULL) {
        printf("P%d: Burst: %d, arrival: %d, priority: %d\n",reader->title,reader->burst_time,reader->arrival_time,reader->priority);
        reader = reader->next;
    }
    // Close input file
    fclose(input_file);

    // algorithm goes here
    // DoFCFS(head);
    // DoSJF(head);
    //DoSJFP(head);
      doRoundRobin(head);

    // Iterate through linked list and write data to output file
    //display(head, output_file, "First come first serve - Non preemptive");
    //display(head, output_file, "Scheduling Method: Shortest Job First - Non-Preemptive");
    //display(head, output_file, "Scheduling Method: Shortest Job First  - Preemptive");
      display(head, output_file, "Scheduling Method: Round Robin ");
    // Close output file
    fclose(output_file);

    // Iterate through linked list and free memory
    Process *cleaningPtr = head;
    while (cleaningPtr != NULL) {
        Process *next = cleaningPtr->next;
        free(cleaningPtr);
        cleaningPtr = next;
    }
}
