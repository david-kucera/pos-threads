#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

int n, k;

typedef struct pult {
    int aktualny_pocet;

    pthread_mutex_t mutex;
    pthread_cond_t je_plny;
    pthread_cond_t je_prazdny;
} PULT;

typedef struct hrdina {
    int id;
    int zameranie;
    int cas_trvania_liecitel;
    int cas_trvania_zadavatel;
    bool dostal_talizman;
    PULT * pult;
} HRDINA;

typedef struct liecitel {
    int cas_pripravy;
    PULT * pult;
} LIECITEL;

void * init_pult(PULT * pult) {
    pult->aktualny_pocet = 0;
    pthread_mutex_init(&pult->mutex, NULL);
    pthread_cond_init(&pult->je_plny, NULL);
    pthread_cond_init(&pult->je_prazdny, NULL);
}

void * destroy_pult(PULT * pult) {
    pult->aktualny_pocet = 0;
    pthread_mutex_destroy(&pult->mutex);
    pthread_cond_destroy(&pult->je_prazdny);
    pthread_cond_destroy(&pult->je_plny);
}

void * hrdinovia_fun(void * arg) {
    HRDINA * data = (HRDINA *)arg;

    sleep(data->cas_trvania_liecitel);
    printf("Hrdina %i. prisiel ku liecitelovi.\n", data->id);

    pthread_mutex_lock(&data->pult->mutex);
    while (data->pult->aktualny_pocet == 0) {
        printf("Hrdina %i caka, lebo na pulte nic nie je!\n", data->id);
        pthread_cond_wait(&data->pult->je_prazdny, &data->pult->mutex);
    }

    printf("Hrdina %i si kupil lektvar a odchadza...\n", data->id);
    pthread_cond_signal(&data->pult->je_plny);
    data->pult->aktualny_pocet--;
    pthread_mutex_unlock(&data->pult->mutex);
    sleep(data->cas_trvania_zadavatel);

    int perc_talizman = rand() % 100;
    if (data->zameranie == 1) {
        if (perc_talizman < 10) {
            data->dostal_talizman = true;
        }
    } else if (data->zameranie == 2) {
        if (perc_talizman < 30) {
            data->dostal_talizman = true;
        }
    } else {
        if (perc_talizman < 20) {
            data->dostal_talizman = true;
        }
    }

    printf("Hrdina %i skoncil svoju put. Talisman: %i.\n", data->id, data->dostal_talizman);
    return NULL;
}

void * liecitel_fun(void * arg) {
    LIECITEL * data = (LIECITEL *)arg;

    pthread_mutex_lock(&data->pult->mutex);
    for (int i = 0; i < n; ++i) {

        while (data->pult->aktualny_pocet == 3) {
            printf("Liecitel caka, lebo pult je plny!\n");
            pthread_cond_wait(&data->pult->je_plny, &data->pult->mutex);
        }
        pthread_cond_signal(&data->pult->je_prazdny);
        sleep(data->cas_pripravy);
        data->pult->aktualny_pocet++;
        printf("Liecitel pripravil lektvar. Aktualny pocet je %i.\n", data->pult->aktualny_pocet);
    }
    pthread_mutex_unlock(&data->pult->mutex);

    printf("Liecitel vylozil vsetky lektvary a jeho cinnost sa konci.\n");
    return NULL;
}

int main(int argc, char * argv[]) {
    srand(time(NULL));
    if(argc != 3) {
        n = 12;
        k = 5;
    } else {
        n = atoi(argv[1]);
        k = atoi(argv[2]);
    }
    printf("Get the resurrection potion! Quest is now ACTIVE!\n");
    PULT pult;
    init_pult(&pult);

    HRDINA hrdinovia_data[n];
    pthread_t hrdinoviat[n];
    for (int i = 0; i < n; ++i) {
        hrdinovia_data[i].id = i + 1;
        hrdinovia_data[i].pult = &pult;

        int perc_rozdelenie = rand() % 100;
        int cas_liecitel;
        int cas_zadavatel;
        int zameranie;
        if (perc_rozdelenie < 50) {
            // BOJOVNIK
            zameranie = 1;
            cas_liecitel = rand() % 4 + 1;
            cas_zadavatel = rand() % 4 + 1;
        } else if (perc_rozdelenie > 50 && perc_rozdelenie < 70) {
            // MAG
            zameranie = 2;
            cas_liecitel = rand() % 6 + 3;
            cas_zadavatel = rand() % 8 + 4;
        } else {
            // VRAH
            zameranie = 3;
            cas_liecitel = rand() % 6 + 1;
            cas_zadavatel = rand() % 8 + 1;
        }
        hrdinovia_data[i].zameranie = zameranie;
        hrdinovia_data[i].cas_trvania_liecitel = cas_liecitel;
        hrdinovia_data[i].cas_trvania_zadavatel = cas_zadavatel;
        hrdinovia_data[i].dostal_talizman = false;
        pthread_create(&hrdinoviat[i], NULL, hrdinovia_fun, &hrdinovia_data[i]);
    }

    LIECITEL liecitel_data;
    liecitel_data.pult = &pult;
    liecitel_data.cas_pripravy = k;
    pthread_t liecitelt;
    pthread_create(&liecitelt, NULL, liecitel_fun, &liecitel_data);

    pthread_join(liecitelt, NULL);

    for (int i = 0; i < n; ++i) {
        pthread_join(hrdinoviat[i], NULL);
    }

    printf("Get the resurrection potion! Quest is now FINISHED!\n");

    destroy_pult(&pult);
    return 0;
}