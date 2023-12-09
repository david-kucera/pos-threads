#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_PLANTAZ 500
#define KAPACITA_SKLADU 10

typedef struct sklad {
    int aktualny_pocet;

    pthread_mutex_t mutex;
    pthread_cond_t je_plny;
    pthread_cond_t je_prazdny;
} SKLAD;

typedef struct plantaznici {
    SKLAD * sklad;
} PLANTAZNICI;

typedef struct praziaren {
    SKLAD * sklad;
    int zisk;
    int pocet_kg_spracovanych;
} PRAZIAREN;

void init_sklad(SKLAD * sklad) {
    sklad->aktualny_pocet = 0;
    pthread_mutex_init(&sklad->mutex, NULL);
    pthread_cond_init(&sklad->je_plny, NULL);
    pthread_cond_init(&sklad->je_prazdny, NULL);
}

void destroy_sklad(SKLAD * sklad) {
    sklad->aktualny_pocet = 0;
    pthread_mutex_destroy(&sklad->mutex);
    pthread_cond_destroy(&sklad->je_prazdny);
    pthread_cond_destroy(&sklad->je_plny);
}

void * plantaznik_fun(void * arg) {
    PLANTAZNICI * data = (PLANTAZNICI *)arg;

    for (int i = 0; i < MAX_PLANTAZ; ++i) {
        if (data->sklad->aktualny_pocet == KAPACITA_SKLADU) {
            printf("Sklad je plny, plantaznici idu na oddych...\n");
            sleep(6);
        }
        int cas_na_kg = 1 + rand() % 3;
        sleep(cas_na_kg);
        pthread_mutex_lock(&data->sklad->mutex);
        data->sklad->aktualny_pocet++;
        printf("Plantaznici vypestovali a pozbierali kg kavy. Aktualny pocet kg v sklade je %i.\n", data->sklad->aktualny_pocet);
        pthread_mutex_unlock(&data->sklad->mutex);
        if (data->sklad->aktualny_pocet >= 5) {
            pthread_cond_signal(&data->sklad->je_plny);
        }
    }

    printf("Plantaznici skoncili svoju pracu a koncia.\n");
    return NULL;
}

void * praziaren_fun(void * arg) {
    PRAZIAREN * data = (PRAZIAREN *)arg;

    while (data->pocet_kg_spracovanych != MAX_PLANTAZ) {

        pthread_mutex_lock(&data->sklad->mutex);
        while (data->sklad->aktualny_pocet < 5) {
            printf("Praziaren caka kym bude v sklade aspon 5 kg kavy.\n");
            pthread_cond_wait(&data->sklad->je_plny, &data->sklad->mutex);
        }

        int spracovavane_mnozstvo = 5;
        data->sklad->aktualny_pocet -= spracovavane_mnozstvo;
        printf("Praziaren zobrala zo skladu %i kg a ide ich spracovat!\n", spracovavane_mnozstvo);

        pthread_mutex_unlock(&data->sklad->mutex);
        pthread_cond_broadcast(&data->sklad->je_prazdny);

        int cas_prazenia = 1 + rand() % 5;
        sleep(cas_prazenia);
        int cena_za_kg;

        if (cas_prazenia < 2) {
            cena_za_kg = 15;
        } else if (cas_prazenia >= 2 && cas_prazenia < 3) {
            cena_za_kg = 20;
        } else if (cas_prazenia >= 3 && cas_prazenia < 4) {
            cena_za_kg = 30;
        } else {
            cena_za_kg = 5;
        }

        data->pocet_kg_spracovanych += spracovavane_mnozstvo;
        int zisk = spracovavane_mnozstvo * cena_za_kg;
        data->zisk += zisk;

        printf("Praziaren spracovala kavu a ide si oddychnut...\n");
        sleep(2);
        printf("Praziaren zatial spracovala %i kg kavy\n",data->pocet_kg_spracovanych);
    }

    printf("Praziaren skoncila svoju cinnost a konci!\n");
    return NULL;
}

int main(int argc, char * argv[]) {
    srand(time(NULL));

    printf("Zacina sa pracovat!\n");

    SKLAD sklad;
    init_sklad(&sklad);

    PRAZIAREN praziaren;
    praziaren.sklad = &sklad;
    praziaren.zisk = 0;
    praziaren.pocet_kg_spracovanych = 0;

    PLANTAZNICI plantaznici;
    plantaznici.sklad = &sklad;

    pthread_t praziarent, plantaznicit;
    pthread_create(&praziarent, NULL, praziaren_fun, &praziaren);
    pthread_create(&plantaznicit, NULL, plantaznik_fun, &plantaznici);

    pthread_join(plantaznicit, NULL);
    pthread_join(praziarent, NULL);

    printf("Ziskalo sa vsetko mnozstvo kavy. Priemerny zisk praziarne je %i EUR.", praziaren.zisk);
    destroy_sklad(&sklad);
    return 0;
}