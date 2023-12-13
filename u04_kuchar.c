#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int max_pocet_stolik;

typedef struct stolik {
    int akt_pocet;

    pthread_mutex_t mutex;
    pthread_cond_t je_plny;
    pthread_cond_t je_prazdny;
} STOLIK;

typedef struct casnik {
    int id_casnika;
    STOLIK * stolik;
} CASNIK;

typedef struct kuchar {
    STOLIK * stolik;
} KUCHAR;

void * kuchar_fun(void * arg) {
    KUCHAR * data = (STOLIK *)arg;

    while (1) {
        printf("Kuchar pripravuje jedlo...\n");
        int cas_pripravy = (rand() % 2) + 1;
        sleep(cas_pripravy);
        pthread_mutex_lock(&data->stolik->mutex);
        while (data->stolik->akt_pocet == max_pocet_stolik) {
            printf("Kuchar caka na casnikov kym odnesu jedla!\n");
            pthread_cond_wait(&data->stolik->je_plny, &data->stolik->mutex);
        }
        data->stolik->akt_pocet++;
        printf("Kuchar pripravil jedlo. Celkovy pocet jedal na stole: %i\n", data->stolik->akt_pocet);
        pthread_cond_signal(&data->stolik->je_prazdny);
        pthread_mutex_unlock(&data->stolik->mutex);
    }

    return NULL;
}

void * casnik_fun(void * arg) {
    CASNIK * data = (CASNIK *)arg;
    printf("Casnik %i zacina pracovat!\n", data->id_casnika);

    while (1) {
        pthread_mutex_lock(&data->stolik->mutex);
        while (data->stolik->akt_pocet == 0) {
            pthread_cond_wait(&data->stolik->je_prazdny, &data->stolik->mutex);
        }
        data->stolik->akt_pocet--;
        pthread_cond_signal(&data->stolik->je_plny);
        pthread_mutex_unlock(&data->stolik->mutex);
        printf("Casnik %i odniesol jedlo, celkovy pocet jedal na stole je %i.\n", data->id_casnika, data->stolik->akt_pocet);
        int cas_roznosu = (rand() % 4) + 2;
        sleep(cas_roznosu);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    if (argc != 2) {
        max_pocet_stolik = 10;
    } else {
        max_pocet_stolik = atoi(argv[1]);
    }

    STOLIK stolik;
    stolik.akt_pocet = 0;
    pthread_mutex_init(&stolik.mutex, NULL);
    pthread_cond_init(&stolik.je_plny, NULL);
    pthread_cond_init(&stolik.je_prazdny, NULL);

    pthread_t kuchar;
    KUCHAR data_kuchar = {&stolik};
    pthread_create(&kuchar, NULL, kuchar_fun, &data_kuchar);

    pthread_t casnici[2];
    CASNIK data_casnik[2];
    for (int i = 0; i < 2; ++i) {
        data_casnik[i].stolik = &stolik;
        data_casnik[i].id_casnika = i + 1;
        pthread_create(&casnici[i], NULL, casnik_fun, &data_casnik[i]);
    }

    printf("Restauracia otvorena!\n");
    pthread_join(kuchar, NULL);
    pthread_join(casnici[0], NULL);
    pthread_join(casnici[1], NULL);


    printf("Restauracia zatvorena!\n");
    pthread_mutex_destroy(&stolik.mutex);
    pthread_cond_destroy(&stolik.je_prazdny);
    pthread_cond_destroy(&stolik.je_plny);
    return 1;
}