#ifndef POSTOFFICE_H
#define POSTOFFICE_H

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <time.h>

/* ολες οι κατηγοριες destination parcel*/
extern int finish_employees;
extern int total_deliveries;
typedef enum {
    LOCAL,
    NATIONAL,
    INTERNATIONAL,
}DestinationCategories;

typedef struct {
    int customers_number;
    DestinationCategories dest;
    sem_t *delivery_signal;
}Parsel;

/* δημιουργει thread για customers employees postmen */
void* customer_behavior(void *arg);
void* employee_behavior(void *arg);
void* postman_behavior(void *arg);
typedef struct {
    Parsel *buffer;

    int parse_count;
    int size;

    int head;
    int tail;

    pthread_mutex_t mutex;
    sem_t slots;
    sem_t items;
} Belt;

int put_intake_belt(Parsel *p);
int put_destination_belt(Parsel *p,int employee_id);
int initialize(int capacity);
#endif