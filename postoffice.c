#include "postoffice.h"

Belt intake_belt,destination_belts[3];
int finish_employees=0;
int total_deliveries;

int initialize(int capacity) {

    /*αρχικοποιηση intake belt*/
    intake_belt.size=capacity;
    intake_belt.head=0;
    intake_belt.tail=0;
    intake_belt.parse_count=0;
    intake_belt.buffer=malloc(sizeof(Parsel)*capacity);

    if (!intake_belt.buffer) {
        fprintf(stderr,"Memory Error");
        return 1;
    }
    pthread_mutex_init(&intake_belt.mutex, NULL);
    sem_init(&intake_belt.slots, 0, capacity);
    sem_init(&intake_belt.items, 0, 0);

    /*αρχικοποιηση destination belts*/
    for(int i=0;i<3;i++) {
        destination_belts[i].size=capacity;
        destination_belts[i].head=0;
        destination_belts[i].tail=0;
        destination_belts[i].parse_count=0;
        destination_belts[i].buffer=malloc(sizeof(Parsel)*capacity);

        if (!destination_belts[i].buffer) {
            fprintf(stderr,"Memory Error ");
            return 1;
        }
        pthread_mutex_init(&destination_belts[i].mutex, NULL);
        sem_init(&destination_belts[i].slots, 0, capacity);   /* αρχικά ολες οι θεσεις ελευθερες */
        sem_init(&destination_belts[i].items, 0, 0);
    }
    return 0;
}

int put_intake_belt(Parsel *p) {
    sem_wait(&intake_belt.slots);
    /* κλειδωνει ωστε να μην μπορουν να το τρεχουν πανω απο 1 thread*/
    pthread_mutex_lock(&intake_belt.mutex);
   /* παει το tails ενα μπροστα και βλεπει αν κανει κυκλο να παει απο την αρχη*/
    intake_belt.buffer[intake_belt.tail]=*p;
    intake_belt.parse_count++;
    intake_belt.tail=(intake_belt.tail+1)%intake_belt.size;

     printf("[CUSTOMER %d] Placed parcel on intake belt (%d/%d)\n",p->customers_number,intake_belt.parse_count,intake_belt.size);

    pthread_mutex_unlock(&intake_belt.mutex);

    sem_post(&intake_belt.items);
    return 0;
}

int put_destination_belt(Parsel *p,int employee_id) {


    int position=p->dest;


    sem_wait(&destination_belts[position].slots);

   destination_belts[position].parse_count++;
    destination_belts[position].buffer[destination_belts[position].tail]=*p;
    destination_belts[position].tail=( destination_belts[position].tail+1)% destination_belts[position].size;

    if (p->dest == LOCAL) {
        printf("[EMPLOYEE %d] Placed parcel on LOCAL belt (%d/%d)\n"
            ,employee_id
            ,destination_belts[position].parse_count
            ,destination_belts[position].size);
    }else if (p->dest == NATIONAL) {
        printf("[EMPLOYEE %d] Placed parcel on NATIONAL belt (%d/%d)\n"
            ,employee_id
            ,destination_belts[position].parse_count
            , destination_belts[position].size);
    }else if (p->dest == INTERNATIONAL) {
        printf("[EMPLOYEE %d] Placed parcel on INTERNATIONAL belt (%d/%d)\n"
             ,employee_id
             ,destination_belts[position].parse_count
             ,destination_belts[position].size);
    }
    sem_post(&destination_belts[position].items);
    return 0;


}

void* customer_behavior(void *arg) {
    sem_t delivery_signal;
    int id=*((int*)arg);

    /* κοιματε τυχαια για 1-5 s*/
    int cooldown=(rand()%5)+1;
    sleep(cooldown);

    /* τυχαια κατηγορια*/
    int random_parsel=rand()%3;
    Parsel p;


    sem_init(&delivery_signal, 0, 0);

    p.customers_number=id;
    p.dest=(DestinationCategories)random_parsel;
    p.delivery_signal=&delivery_signal;

    /* εκτυπωνει στην οθονη οτι δημιουργηθηκε το parcel*/
    if (p.dest==0) {
        printf("[CUSTOMER %d]Created parcel (LOCAL)\n",id);
    }else if (p.dest==1) {
        printf("[CUSTOMER %d] Created parcel (NATIONAL)\n",id);
    }else if (p.dest==2) {
        printf("[CUSTOMER %d] Created parcel (INTERNATIONAL)\n",id);
    }

    /*το βαζει στο in belt*/
     put_intake_belt(&p);

    /* περιμενει να παραδωθει απο τον postman */
    sem_wait(&delivery_signal);
    printf("[CUSTOMER %d] Notified: parcel delivered.\n",id);

    /* καταστρεφει καθε νημα customers*/
    sem_destroy(&delivery_signal);

    return NULL;
}



void* employee_behavior(void *arg) {

    int employee_id=*((int*)arg);
    while (1) {
        Parsel p;
        sem_wait(&intake_belt.items);
        pthread_mutex_lock(&intake_belt.mutex);

        /* τελειωσαν οι customers και δεν εχει τιποτα μεσα το intake belt*/
        if (finish_employees==1) {
            pthread_mutex_unlock(&intake_belt.mutex);
            sem_post(&intake_belt.items);
            break;
        }

        intake_belt.parse_count--;
        p=intake_belt.buffer[intake_belt.head];



        intake_belt.head=(intake_belt.head+1)%intake_belt.size;
        /* μολις το παρει το επεξεργαζετε αρα κοιματε*/
        int wait=(rand()%5)+1;
        sleep(wait);

        if (p.dest ==  LOCAL) {
            printf("[EMPLOYEE %d] Picked up parcel from Customer %d (LOCAL)\n",employee_id,p.customers_number);
        }else if (p.dest == NATIONAL) {
            printf("[EMPLOYEE %d] Picked up parcel from Customer %d (NATIONAL)\n",employee_id,p.customers_number);
        }else if (p.dest == INTERNATIONAL) {
            printf("[EMPLOYEE %d] Picked up parcel from Customer %d (INTERNATIONAL)\n",employee_id,p.customers_number);
        }



        pthread_mutex_unlock(&intake_belt.mutex);

        put_destination_belt(&p,employee_id);




        sem_post(&intake_belt.slots);
    }
    return NULL;

}

void* postman_behavior(void *arg) {

    DestinationCategories dest=*((DestinationCategories*)arg);
    Parsel p;
    static int num_customers;
    int first_time=1;
    int packages_delivered;
    while (1) {
       /* αρχικοποιει μονο μια φορα για δει ποσους πελατες εχουμε*/
        if (first_time==1) {
            num_customers = total_deliveries;
            first_time = 0;
        }
        sem_wait(&destination_belts[dest].items);

        pthread_mutex_lock(&destination_belts[dest].mutex);
       /* το συναντανε οι 2 αλλοι postmen που δεν ειχαν το τελευταιο πακετο*/
        if (total_deliveries <= 0) {
            pthread_mutex_unlock(&destination_belts[dest].mutex);
            break;
        }

        /* βρισκει το πακετο το περνει και επισης υπολογιζει σε ποιο πακετο βρισκομαστε για να τυπωθει στην οθονη*/
        p=destination_belts[dest].buffer[destination_belts[dest].head];
        destination_belts[dest].head = (destination_belts[dest].head + 1) % destination_belts[dest].size;
        destination_belts[dest].parse_count--;
        total_deliveries--;
        packages_delivered=num_customers-total_deliveries;



        pthread_mutex_unlock(&destination_belts[dest].mutex);
        sem_post(&destination_belts[dest].slots);

        int wait=(rand()%5)+1;
        sleep(wait);

        if (dest == LOCAL) {
            printf("[POSTMAN 0] Delivered parcel from Customer %d (LOCAL). Total delivered: %d/%d\n"
                ,p.customers_number
                ,packages_delivered
                ,num_customers);
        } else if (dest == NATIONAL) {
            printf("[POSTMAN 1] Delivered parcel from Customer %d (NATIONAL). Total delivered: %d/%d\n"
                ,p.customers_number
                ,packages_delivered
                ,num_customers);
        } else if(dest == INTERNATIONAL) {
            printf("[POSTMAN 2] Delivered parcel from Customer %d (INTERNATIONAL). Total delivered: %d/%d\n"
                ,p.customers_number
                ,packages_delivered
                ,num_customers);
        }
        sem_post(p.delivery_signal);


       /* το συνανταει ο postman με το τελευταιο πακετο*/
        if (total_deliveries <= 0) {
            /* ξυναει ολους τους αλλους postman*/
            for (int i = 0; i < 3; i++) {
                sem_post(&destination_belts[i].items);
            }
            /* 1 για να τελειωσουν οι employees και να κλεισουν τα thread τους*/
            finish_employees=1;
            /* στελνει ψευτικο σημα οτι υπαρχει πακετο στο intake buffer*/
            sem_post(&intake_belt.items);
            break;
        }
    }

    return NULL;
}



