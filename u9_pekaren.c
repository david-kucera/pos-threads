#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int n, k;

typedef struct pult {
    int akt_pocet;
    pthread_mutex_t mutex;
    pthread_cond_t pridaj_chleba;
    pthread_cond_t odober_chleba;
} PULT;

typedef struct pekar {
    PULT * pult;
} PEKAR;

typedef struct zakaznik {
    int id;
    int cas_prichodu;
    PULT * pult;
} ZAKAZNIK;

void init_pult(PULT * pult) {
    pult->akt_pocet = 0;
    pthread_mutex_init(&pult->mutex, NULL);
    pthread_cond_init(&pult->pridaj_chleba, NULL);
    pthread_cond_init(&pult->odober_chleba, NULL);
}

void destroy_pult(PULT * pult) {
    pult->akt_pocet = 0;
    pthread_cond_destroy(&pult->pridaj_chleba);
    pthread_cond_destroy(&pult->odober_chleba);
    pthread_mutex_destroy(&pult->mutex);
}

void * pekar_fun(void * arg) {
    PEKAR * data = (PEKAR *)arg;
    printf("Pekar zacina piect chleby!\n");
    for (int i = 1; i <= n; ++i) {
        printf("Zacinam piect %i. chleba, aktualny pocet na stole %i!\n", i, data->pult->akt_pocet);
        sleep(k);
        pthread_mutex_lock(&data->pult->mutex);
        while (data->pult->akt_pocet >= 2) {
            printf("Pekar caka na odobranie chleba!\n");
            pthread_cond_wait(&data->pult->odober_chleba, &data->pult->mutex);
        }
        data->pult->akt_pocet++;
        printf("Pekar napiekol %i, chleba, aktualny pocet na stole %i\n", i, data->pult->akt_pocet);
        pthread_cond_signal(&data->pult->pridaj_chleba);
        pthread_mutex_unlock(&data->pult->mutex);
    }
    printf("Pekar napiekol co mal a konci!\n");
    return NULL;
}

void * zak_fun(void * arg) {
    ZAKAZNIK * data = (ZAKAZNIK *)arg;
    printf("Zakaznik %i PRICHADZA do pekarne!\n", data->id);
    sleep(data->cas_prichodu);
    printf("Zakaznik %i PRISIEL do pekarne!\n", data->id);
    pthread_mutex_lock(&data->pult->mutex);
    while (data->pult->akt_pocet <= 0) {
        printf("Zakaznik %i caka na pridanie chleba!\n", data->id);
        pthread_cond_wait(&data->pult->pridaj_chleba, &data->pult->mutex);
    }
    data->pult->akt_pocet--;
    printf("Zakaznik %i si berie chleba a odchadza, aktualny pocet %i.\n", data->id, data->pult->akt_pocet);
    pthread_cond_signal(&data->pult->odober_chleba);
    pthread_mutex_unlock(&data->pult->mutex);
    return NULL;
}

int main(int argc, char * argv[]) {
    srand(time(NULL));

    if (argc != 3) {
        n = 10;
        k = 4;
    } else {
        n = atoi(argv[1]);
        k = atoi(argv[2]);
    }

    PULT pult;
    init_pult(&pult);

    PEKAR pekar_data;
    pekar_data.pult = &pult;
    pthread_t pekar;
    pthread_create(&pekar, NULL, pekar_fun, &pekar_data);

    ZAKAZNIK zakaznik_data[n];
    pthread_t zakaznici[n];
    for (int i = 0; i < n; ++i) {
        zakaznik_data[i].pult = &pult;
        zakaznik_data[i].id = i + 1;
        zakaznik_data[i].cas_prichodu = 2 + rand() % 6;
        pthread_create(&zakaznici[i], NULL, zak_fun, &zakaznik_data[i]);
    }

    printf("Pekaren je otvorena!\n");
    pthread_join(pekar, NULL);
    for (int i = 0; i < n; ++i) {
        pthread_join(zakaznici[i], NULL);
    }

    destroy_pult(&pult);
    printf("Pekaren je zatvorena!\n");
    return 0;
}
