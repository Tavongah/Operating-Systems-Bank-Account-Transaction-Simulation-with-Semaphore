#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>

// Transaction and process structures
struct transaction {
    short startTime;
    short duration;
    short amount;
};

struct procStruct {
    short count;
    struct transaction arr[1000];
};

// Global shared variables
struct procStruct *processes;
int *balance;
sem_t *account_sem;
short *current_time;

// Process function
void run_process(int process_id) {
    struct procStruct *proc = &processes[process_id];
    
    for (int i = 0; i < proc->count; i++) {
        struct transaction *t = &proc->arr[i];
        
        // Wait until transaction start time
        while (*current_time < t->startTime) {
            usleep(500);
        }
        
        // Acquire semaphore for critical section
        sem_wait(account_sem);
        
        short acquire_time = *current_time;
        
        // Check if transaction can be completed (withdrawal only if sufficient funds)
        int success = 1;
        if (t->amount < 0 && (*balance + t->amount) < 0) {
            success = 0; // Insufficient funds for withdrawal
        }
        
        // If transaction is successful, update balance
        if (success) {
            *balance += t->amount;
        }
        
        // Simulate transaction processing time by holding the lock
        short target = acquire_time + t->duration;
        while (*current_time < target) {
            usleep(500);
        }
        
        // Calculate finish time
        short finish_time = acquire_time + t->duration - 1;
        
        // Output result in correct format
        printf("%d %d %d %d %d %d\n", 
               t->startTime, process_id + 1, finish_time, success, 
               t->amount, *balance);
        
        // Release semaphore
        sem_post(account_sem);
    }
    
    exit(0);
}

int main() {
    int num_processes, num_transactions;
    
    // Read first line: number of processes and transactions
    if (scanf("%d %d", &num_processes, &num_transactions) != 2) {
        fprintf(stderr, "Error reading input\n");
        return 1;
    }
    
    // Allocate shared memory for processes array
    processes = mmap(NULL, num_processes * sizeof(struct procStruct),
                    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    // Allocate shared memory for balance
    balance = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, 
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *balance = 0;
    
    // Allocate shared memory for semaphore
    account_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, 
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    // Allocate shared memory for current_time
    current_time = mmap(NULL, sizeof(short), PROT_READ | PROT_WRITE, 
                       MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *current_time = 0;
    
    // Initialize semaphore
    if (sem_init(account_sem, 1, 1) == -1) {
        perror("sem_init");
        return 1;
    }
    
    // Initialize process counts to zero
    for (int i = 0; i < num_processes; i++) {
        processes[i].count = 0;
    }
    
    // Read transaction data
    for (int i = 0; i < num_transactions; i++) {
        int start_time, process_id, duration, amount;
        
        if (scanf("%d %d %d %d", &start_time, &process_id, &duration, &amount) != 4) {
            fprintf(stderr, "Error reading transaction %d\n", i);
            return 1;
        }
        
        // Convert to 0-based indexing for process_id
        process_id--;
        
        // Add transaction to appropriate process
        int idx = processes[process_id].count;
        processes[process_id].arr[idx].startTime = start_time;
        processes[process_id].arr[idx].duration = duration;
        processes[process_id].arr[idx].amount = amount;
        processes[process_id].count++;
    }
    
    // Create child processes
    pid_t pids[num_processes];
    
    for (int i = 0; i < num_processes; i++) {
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Child process
            run_process(i);
        } else if (pids[i] < 0) {
            perror("fork");
            return 1;
        }
    }
    
    // Parent drives the clock
    for (int t = 0; t <= 101; t++) {
        if (t > 0) {
            usleep(1000);
        }
        *current_time = t;
    }
    
    // Parent process waits for all children to complete
    for (int i = 0; i < num_processes; i++) {
        waitpid(pids[i], NULL, 0);
    }
    
    // Cleanup
    sem_destroy(account_sem);
    munmap(processes, num_processes * sizeof(struct procStruct));
    munmap(balance, sizeof(int));
    munmap(account_sem, sizeof(sem_t));
    munmap(current_time, sizeof(short));
    
    return 0;
}