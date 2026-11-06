#include "postoffice.h"

/* Γιώργος Μαραγκάκης csd5694 */

int main(const int argc, char *argv[]) {
    /* να τα βαγζει κατευθειαν στην οθονη */
    setvbuf(stdout, NULL, _IONBF, 0);

    int num_customers;
    int num_employees;
    int belt_capacity;

    if (argc !=4) {
        fprintf(stderr,"needs 4 arguments \n");
        exit(1);
    }

    srand(time(NULL));

    /* κανει απο string σε int ωστε να εχω ολα τα ορισματα σε μεταβλητες */
    num_customers = atoi(argv[1]);
    num_employees = atoi(argv[2]);
    belt_capacity = atoi(argv[3]);


    /* να ξερουν ποτε να σταματανε οι postmen*/
    total_deliveries = num_customers;

    if ((num_customers <= 0 || num_employees <= 0 || belt_capacity <= 0)) {
        fprintf(stderr,"wrong arguments");
        exit(1);
    }

    /*αρχικοποιηση belts*/
    initialize(belt_capacity);



    int random_customers[num_customers];
    for (int i=0;i<num_customers;i++) {
        random_customers[i] = i;
    }

    /* για να επιλεγονται τυχαιοι customers*/
    for (int i = num_customers - 1; i > 0; i--) {
        int j = rand() % (i + 1);

        int temp = random_customers[i];
        random_customers[i] = random_customers[j];
        random_customers[j] = temp;
    }

    /* δημιουργια υπαλληλων*/
    pthread_t employees_thread[num_employees];
    int employees[num_employees];
    for (int i = 0; i < num_employees; i++) {
        employees[i] = i;
        if (pthread_create(&employees_thread[i], NULL, employee_behavior, &employees[i])!=0) {
            fprintf(stderr,"couldn't create employee thread");
            exit(1);
        }
    }

    /* δημιουργια πελατωνν*/
    pthread_t customers_thread[num_customers];
    for (int i = 0; i < num_customers; i++) {
       if ( pthread_create(&customers_thread[i], NULL, customer_behavior, &random_customers[i])!=0) {
           fprintf(stderr,"couldn't create customers thread");
           exit(1);
       }

    }



    /* δημιουργια postmen*/
    pthread_t postmen_thread[3];
    int postmen[3] ={LOCAL, NATIONAL, INTERNATIONAL};
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&postmen_thread[i], NULL, postman_behavior, &postmen[i])!=0) {
            fprintf(stderr,"couldn't create postmen thread");
            exit(1);
        }
    }


    /*περιμενει ολα τα threads να τελειωσουν*/
    for (int i = 0; i < num_customers; i++) {
        pthread_join(customers_thread[i], NULL);
    }



    for (int i=0;i<num_employees;i++) {
        pthread_join(employees_thread[i], NULL);
    }

    for (int i=0;i<3;i++) {
        pthread_join(postmen_thread[i], NULL);
    }

    printf("All parcels delivered. Closing post office.\n");



    return 0;
}
