#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
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
void display(Process *header, FILE *output_file, char *description, bool preemptive) {
    Process *current = header;
    printf("Scheduling Method: %s %s\n", description, preemptive ? "Preemptive" : "Non preemptive");
    printf("Process Waiting Times:\n");
    int sum_wait_time = 0;
    int count = 0;
    while (current != NULL) {
        int wait_time = current->start_time - current->arrival_time + current->wait_time;
        sum_wait_time += wait_time;
        printf("P%d %d ms\n", current->title, wait_time);
        fprintf(output_file, "P%d %d ms\n", current->title, wait_time);
        current = current->next; //print title
        count++;
    }
    printf("Average Waiting Time: %.1f ms\n", (double) sum_wait_time / count);
}
struct process * getdata(char *in_txt, struct process *hdr);
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
void DoRoundRobin(Process *head, int time_quantum) {
    Process *current = head;
    int time_passed = 0;
    while (1) {
        if (current == NULL) current = head;
        while (current->done || (current->arrival_time > time_passed)) current = current->next; //
        int extra_time = 0;
        if (current->start_time == -1) current->start_time = time_passed; // -__-
        if (current->burst_time <= time_quantum) {
            current->done = true;
            extra_time = time_quantum - current->burst_time;
        }
        current->burst_time -= time_quantum;    // |P1 3|P2 2|P3 1|P4 2|P5 1|P1 1|P2 0|P3 0|P4 0|P5 0|P1 0|
        time_passed += time_quantum - extra_time;
        // Check for done
        Process *node = head;
        bool done = true;
        while (node != NULL) {
            if (!node->done) { 
                if (node != current && node->start_time > -1 && node->arrival_time <= time_passed) node->wait_time += time_quantum - extra_time;
                done = false;
            }
            node = node->next;
        }
        if (done) return;
        current = current->next;
    }
}
Process *findPriorityNode(Process *head, int current_time) {
    Process *current = head;
    Process *priority_node = current;
    int min_priority = current->priority;
    while (current != NULL && current->arrival_time <= current_time) {
        if (!current->done && current->priority < min_priority) {
            priority_node = current;
            min_priority = current->priority;
        }
        if (current->done && priority_node == current && current->next != NULL) priority_node = current->next;
        current = current->next;
    }
    return priority_node;
}
void DoPriority(Process *head) {
    int time_passed = 0;
    Process *current = head;
    while (true) {
        current = findPriorityNode(head,time_passed);
        if (current->arrival_time > time_passed) time_passed = current->arrival_time;
        current->start_time = time_passed;
        time_passed += current->burst_time;
        current->done = true;
        Process *node = head;
        bool done = true;
        while (node != NULL) {
            if (!node->done) { 
                done = false;
            }
            node = node->next;
        }
        if (done) return;
    }
}
void DoPriorityP(Process *head) {
    int time_passed = 0;
    Process *current = head;
    while (true) {
        current = findPriorityNode(head,time_passed);
        if (!current->done) {
            if (current->start_time == -1) current->start_time = time_passed;
            if (current->burst_time <= 1) current->done = true;
            current->burst_time--;
        }
        time_passed++;
        Process *node = head;
        bool done = true;
        while (node != NULL) {
            if (!node->done) {
                if (node != current && node->start_time > -1) node->wait_time++;
                done = false;
            }
            node = node->next;
        }
        if (done) return;
    }
}

char *input_file_name, *output_file_name; //input and output file

int main(int argc, char *argv []) {

    if(argc < 4){
        exit(1);
    }
    
    int optionInput;//Getopt var. holder

    while((optionInput = getopt(argc, argv, "f:o:")) != -1){//prompt the user for the input and output file
        switch (optionInput)
        {
        case 'f':
            input_file_name = optarg;
            break;
        case 'o':
            output_file_name = optarg;
            break;
        default:
            exit(1);
            break;
        }
    }

    printf("Input: %s, output: %s\n", input_file_name, output_file_name);

    

    // Read data from file and parse into linked list
    Process *head = NULL;
    char *token;
    int burst,arrival,priority;

    

    bool preemptive = false;
    
    char *method;
    char fcfs[] = "first come first serve";
    char sjf[] = "shortest job first";
    char rr[] = "round robin";
    char pr[] = "priority";
    method = fcfs;

    while (1) {
        printf("1) Scheduling Method (%s)\n", method);
        printf("2) Preemptive Mode (%s)\n", preemptive ? "On" : "Off");
        printf("3) Show Result\n");
        printf("4) End Program\n");
        printf("Option > ");
        int option;
        scanf("%d", &option);
        printf("You have selected %d\n", option);
        if (option == 1){
            while (1) {
                int choice;
                printf("1) Select first come first serve\n");
                printf("2) Select shortest job first\n");
                printf("3) Select round robin\n");
                printf("4) Select priority\n");
                printf("Option > ");
                scanf("%d", &choice);
                if (choice > 4 || choice < 1) continue;
                if (choice == 1) method = fcfs;
                if (choice == 2) method = sjf;
                if (choice == 3) method = rr;
                if (choice == 4) method = pr;
                break;
            }
        }
        if (option == 2) preemptive = !preemptive;
        if (option == 3) {
            FILE *input_file = fopen(input_file_name, "r");
            if (input_file == NULL) {
                printf("Error opening input file\n");
                return 1;
            }
        
            // Open output file
            FILE *output_file = fopen(output_file_name, "w");
            if (output_file == NULL) {
                printf("Error opening output file\n");
                return 1;
            }

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

            if (method == fcfs) {
                preemptive = false;
                DoFCFS(head);
            }
            if (method == sjf) {
                if (preemptive) DoSJFP(head);
                else DoSJF(head);
            }
            if (method == rr) {
                preemptive = false;
                int time_quantum;
                printf("Enter your time quantum > ");
                scanf("%d", &time_quantum);
                DoRoundRobin(head,time_quantum);
            }
            if (method == pr) {
                if (preemptive) DoPriorityP(head);
                else DoPriority(head);
            }
            display(head, output_file, method, preemptive);

            fclose(output_file);
      
            exit(1);
        }
        if (option == 4) exit(1);
    }

}
