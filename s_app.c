#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

typedef struct process {
    
    int burst_time;
    int arrival_time;
    int title;
    int priority;
    int start_time;
    struct process *next;  
} Process;

static int num = 0; // Assign an id to a newly created process
Process *createNode(int b_t, int a_t, int priority) {
    Process *node = (Process*)malloc(sizeof(Process));
    node->burst_time = b_t;
    node->arrival_time = a_t;
    node->priority = priority;
    node->title = num; // assign title to each process
    num++; // increment num after each process is created
    node->next = NULL;
    return node;
}
Process *insertOrdered(Process *header, int b_t, int a_t, int priority) {
    Process *node = createNode(b_t, a_t, priority);
    if (header == NULL) {
        return node;
    }
    Process *current = header;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = node;
    return header;
}


void display(Process *header, FILE *output_file) {
    Process *current = header;
    printf("Scheduling Method: Shortest Job First –Preemptive\n");
    printf("Process Waiting Times:\n");
    int sum_wait_time = 0;
    int count = 0;
    while (current != NULL) {
        int wait_time = current->start_time - current->arrival_time;
        sum_wait_time += wait_time;
        printf("P%d %d ms\n", current->title, wait_time);
        current = current->next; //print title
        count++;
    }
    printf("Average Waiting Time: %.1f ms", (double) sum_wait_time / count);
}

void DoFCFS(Process *head){
    Process *current = head;
    int time_passed = 0;
    while (true) {
        time_passed += current->burst_time;
        current = current->next;
        if (current == NULL) break;
        current->start_time = time_passed - current->arrival_time;
    }
}

int main (int argc, char *argv []) 
{
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
    char line[MAX_LINE_LENGTH];
    char *token;



    while (fgets(line, MAX_LINE_LENGTH, input_file) != NULL) {
    // Create new node in linked list
        Process *node = (Process*)malloc(sizeof(Process));

        printf("%s", line);
        if (head == NULL) {
            head = node;
            //current = head;
            //
        } else {
            //current->next = node;
            //current = current->next;
        }


    }
    // Close input file
    fclose(input_file);

    // algorithm goes here
    DoFCFS(head);

    // Iterate through linked list and write data to output file
    display(head, output_file);


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




