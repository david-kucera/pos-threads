#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int n;

typedef struct pult {
    int aktualny_pocet;
    int zarobok;
    int *recenzie;
    pthread_mutex_t mutex;
    pthread_cond_t pridaj;
    pthread_cond_t odober;
} PULT;

typedef struct hrac {
    int cas_prichodu;
    int id;
    PULT * pult;
} HRAC;

typedef struct predajca {
    PULT * pult;
} PREDAJCA;

void init_pult(PULT * pult) {
    pult->aktualny_pocet = 0;
    pult->zarobok = 0;
    pult->recenzie = (int*)malloc(n * sizeof(int));
    pthread_mutex_init(&pult->mutex, NULL);
    pthread_cond_init(&pult->pridaj, NULL);
    pthread_cond_init(&pult->odober, NULL);
}

void destroy_pult(PULT * pult) {
    pult->aktualny_pocet = 0;
    pult->zarobok = 0;
    free(pult->recenzie);
    pthread_mutex_destroy(&pult->mutex);
    pthread_cond_destroy(&pult->pridaj);
    pthread_cond_destroy(&pult->odober);
}

void * predaj(void * arg) {
    PREDAJCA * data = (PREDAJCA *)arg;
    for (int i = 0; i < 6; ++i) {
        data->pult->aktualny_pocet++;
        printf("Predajca vyklada hru na pult. Aktualny pocet hier na pulte je %i\n", data->pult->aktualny_pocet);
    }
    for (int i = 0; i < n-6; ++i) {
        pthread_mutex_lock(&data->pult->mutex);
        printf("Predajca pridava hru na pult. Aktualny pocet hier na pulte je %i\n", data->pult->aktualny_pocet);
        while (data->pult->aktualny_pocet >= 3) {
            pthread_cond_wait(&data->pult->odober, &data->pult->mutex);
        }
        data->pult->aktualny_pocet++;
        pthread_cond_signal(&data->pult->pridaj);
        pthread_mutex_unlock(&data->pult->mutex);
    }
    printf("Praca predajcu skoncila. Vylozil vsetko co mal.\n");
    return NULL;
}

void * kup(void * arg) {
    HRAC * data = (HRAC *)arg;
    printf("Hrac %i prichadza do predajne, bude mu to trvat %i sekund.\n", data->id, data->cas_prichodu);
    sleep(data->cas_prichodu);
    printf("Hrac %i prisiel do predajne.\n", data->id);
    pthread_mutex_lock(&data->pult->mutex);
    while (data->pult->aktualny_pocet == 0) {
        printf("Hrac %i caka na dodanie hry na pult!\n", data->id);
        pthread_cond_wait(&data->pult->pridaj, &data->pult->mutex);
    }
    int sanca = 0 + rand() % 100;
    if(sanca < 65) {
        data->pult->zarobok += 60;
    } else {
        data->pult->zarobok += 75;
    }
    data->pult->aktualny_pocet--;
    pthread_cond_signal(&data->pult->odober);
    pthread_mutex_unlock(&data->pult->mutex);
    printf("Zakaznik %i odchadza so svojou hrou domov, bude mu to trvat %i sekund.\n", data->id, data->cas_prichodu);
    sleep(data->cas_prichodu);
    int cas_hrania = 1 + rand() % 3;
    printf("Zakaznik %i prisiel domov a ide si zahrat hru. Bude mu to trvat %i sekund.\n", data->id, cas_hrania);
    sleep(cas_hrania);
    pthread_mutex_lock(&data->pult->mutex);
    int hodnotenie = 1 + rand() % 10;
    data->pult->recenzie[data->id-1] = hodnotenie;
    pthread_mutex_unlock(&data->pult->mutex);
    printf("Zakaznik %i vykonal co bolo treba a konci!\n", data->id);
    return NULL;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc != 2) {
        n = 15;
    } else {
        n = atoi(argv[1]);
    }

    PULT pult;
    init_pult(&pult);

    printf("Predaj spusteny, kupte si svoje gca++!\n");

    pthread_t predajca;
    PREDAJCA predajca_data;
    predajca_data.pult = &pult;
    pthread_create(&predajca, NULL, predaj, &predajca_data);

    pthread_t hraci[n];
    HRAC hraci_data[n];
    for (int i = 0; i < n; ++i) {
        hraci_data[i].pult = &pult;
        hraci_data[i].id = i + 1;
        hraci_data[i].cas_prichodu = 1 + rand() % 5;
        pthread_create(&hraci[i], NULL, kup, &hraci_data[i]);
    }

    for (int i = 0; i < n; ++i) {
        pthread_join(hraci[i], NULL);
    }

    pthread_join(predajca, NULL);

    printf("Celkovy zarobok z predaja hier je: %i EUR\n", pult.zarobok);
    printf("Recenzie: ");
    for (int i = 0; i < n; ++i) {
        printf("%d ", pult.recenzie[i]);
    }
    printf("\n");

    printf("Predaj skonceny! No refunds!\n");
    destroy_pult(&pult);
    return 0;
}