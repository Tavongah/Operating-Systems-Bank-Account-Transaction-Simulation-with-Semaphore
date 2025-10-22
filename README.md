# CSC520 - Operating Systems

[![C Language](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![OS](https://img.shields.io/badge/OS-Linux-orange.svg)](https://ubuntu.com/)

A collection of operating systems concepts implemented in C, featuring process management, synchronization, and resource allocation simulations.


### [Assignment 2: Bank Account Simulation with Semaphores](./assignment2/)
**Keywords:** `Process Synchronization` `Semaphores` `Critical Section` `Race Conditions`

A multi-process bank account system demonstrating:
- Process synchronization using POSIX semaphores
- Critical section protection
- Race condition prevention
- Transaction serialization

**Key Implementation:**
```c
// Shared memory for account balance
int *balance = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, 
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);

// Semaphore for critical section protection
sem_t *account_sem = sem_open("/account_sem", O_CREAT | O_EXCL, 0644, 1);
