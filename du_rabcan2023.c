#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

int pocet_autobusov, pocet_cestujucich;

typedef struct zastavka {
    int pocet_cestujucich;
    bool obsadena;

    pthread_mutex_t mutex;
    pthread_cond_t je_obsadena;
    pthread_cond_t je_prazdna;
} ZASTAVKA;

typedef struct autobus {
    int kapacita;
    ZASTAVKA * zastavka;
} AUTOBUS;

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

void * zastavka_fun(void * arg) {
    return NULL;
}

void * autobus_fun(void * arg) {
    return NULL;
}

int main(int argc, char * argv[]) {
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
    pthread_t zastavkat;
    pthread_create(&zastavkat, NULL, zastavka_fun, &zastavka);

    pthread_join(zastavkat, NULL);

    AUTOBUS autobus_data[pocet_autobusov];
    for (int i = 0; i < pocet_autobusov; ++i) {
        autobus_data[i].kapacita = 40;
        autobus_data[i].zastavka = &zastavka;
        pthread_t autobusik;
        pthread_create(&autobusik, NULL, autobus_fun, &autobus_data[i]);
        pthread_join(autobusik, NULL);
    }

    printf("Vsetci cestujuci boli odvezeni na zapas! Program konci.\n");
    destroy_zastavka(&zastavka);
    return 0;
}