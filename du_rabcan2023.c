#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

int pocet_autobusov, pocet_cestujucich;

typedef struct zastavka {
    int pocet_cestujucich;
    bool obsadena;

    pthread_mutex_t mutex;
    pthread_cond_t je_obsadena;
    pthread_cond_t je_prazdna;
} ZASTAVKA;

typedef struct autobus {
    int id;
    int kapacita;
    int pocet_cestujucich;
    ZASTAVKA * zastavka;
} AUTOBUS;

typedef struct opravar {
    ZASTAVKA * zastavka;
} OPRAVAR;

void init_zastavka(ZASTAVKA * zastavka) {
    zastavka->pocet_cestujucich = pocet_cestujucich;
    zastavka->obsadena = false;
    pthread_mutex_init(&zastavka->mutex, NULL);
    pthread_cond_init(&zastavka->je_obsadena, NULL);
    pthread_cond_init(&zastavka->je_prazdna, NULL);
}

void destroy_zastavka(ZASTAVKA * zastavka) {
    zastavka->pocet_cestujucich = 0;
    zastavka->obsadena = false;
    pthread_mutex_destroy(&zastavka->mutex);
    pthread_cond_destroy(&zastavka->je_prazdna);
    pthread_cond_destroy(&zastavka->je_obsadena);
}

void * opravar_fun(void * arg) {
    return NULL;
}

void * autobus_fun(void * arg) {
    AUTOBUS * data = (AUTOBUS *)arg;

    while (data->zastavka->pocet_cestujucich != 0) {
        data->pocet_cestujucich = 0;
        data->id++;

        pthread_mutex_lock(&data->zastavka->mutex);
        while (!data->zastavka->je_obsadena) {
            printf("Zastavka je obsadena! Autobus %i caka na uvolnenie zastavky.\n", data->id);
            pthread_cond_wait(&data->zastavka->je_obsadena, &data->zastavka->mutex);
        }


        printf("Autobus %i prichadza na zastavku. Jeho kapacita je %i osob.\n", data->id, data->kapacita);
        data->zastavka->je_obsadena = true;
        if (data->zastavka->pocet_cestujucich > data->kapacita) {
            data->zastavka->pocet_cestujucich -= data->kapacita;
            data->pocet_cestujucich = data->kapacita;
        } else {
            data->pocet_cestujucich = data->zastavka->pocet_cestujucich;
            data->zastavka->pocet_cestujucich = 0;
        }

        int sanca_na_pokazenie = rand() % 100;

        if (sanca_na_pokazenie < 50) {
            printf("Autobus %i sa pokazil! Opravar ho ide orpavit...\n", data->id);
            sleep(rand() % 5 + 1);
            printf("Opravar opravil autobus!\n");
        }

        // nastupovanie cestujucich
        for (int i = 0; i < data->pocet_cestujucich; ++i) {
            int cas = (rand() % 20 + 5)/10;
            printf("nastupuje %i. cestujuci...\n", i + 1);
            sleep(cas);
        }
        printf("Vsetci cestujuci nastupili do autobusu %i!\n", data->id);
        pthread_cond_signal(&data->zastavka->je_prazdna);
        pthread_mutex_unlock(&data->zastavka->mutex);

        // cesta autobusus na finalnu stanicu
        int cas = rand() % 12 + 8;
        sleep(cas);
        printf("Autobus %i prisiel do finalnej destinacie!\n", data->id);
    }

    printf("Autobusy odviezli vsetkych cestujucich!\n");
    return NULL;
}

int main(int argc, char * argv[]) {
    srand(time(NULL));
    // Pocet autobusov, ako aj pocet cestujucich na zastavke budu ako argumenty programu
    if (argc != 3) {
        pocet_autobusov = 3;
        pocet_cestujucich = 100;
    } else {
        pocet_autobusov = atoi(argv[1]);
        pocet_cestujucich = atoi(argv[2]);
    }

    printf("SPUSTA SA PROGRAM!\n");

    ZASTAVKA zastavka;
    init_zastavka(&zastavka);

    AUTOBUS autobus_data;
    pthread_t autobusy;
    autobus_data.id = 0;
    autobus_data.pocet_cestujucich = 0;
    autobus_data.kapacita = 40;
    autobus_data.zastavka = &zastavka;
    pthread_create(&autobusy, NULL, autobus_fun, &autobus_data);

//    OPRAVAR opravar;
//    pthread_t opravart;
//    pthread_create(&opravart, NULL, opravar_fun, &opravar);

//    pthread_join(opravart, NULL);
    pthread_join(autobusy, NULL);

    printf("Vsetci cestujuci boli odvezeni na zapas! Program konci.\n");
    destroy_zastavka(&zastavka);
    return 0;
}