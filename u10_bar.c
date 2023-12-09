#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    bool je_na_pulte_drink;
    pthread_mutex_t mutex;
    pthread_cond_t odober_drink;
    pthread_cond_t pridaj_drink;
} PULT;

typedef struct {
    signed char cas_prichodu;
    unsigned int id;
    PULT * pult;
} ZAKAZNIK;

typedef struct {
    PULT * pult;
    unsigned int pocet_drinkov;
} BARMAN;

void * barman_fun(void * arg) {
    BARMAN * data = arg;
    printf("Barman zacina sichtu! Bude robit %u drinkov!\n", data->pocet_drinkov);
    for (int i = 1; i <= data->pocet_drinkov; ++i) {
        printf("Pripravujem %d. drink typu",i);
        if(rand()%2) {
            printf(" 1 s cakanim 2s\n");
            sleep(2);
        } else {
            printf(" 2 s cakanim 1s\n");
            sleep(1);
        }
        pthread_mutex_lock(&data->pult->mutex);
        while(data->pult->je_na_pulte_drink) {
            pthread_cond_wait(&data->pult->pridaj_drink, &data->pult->mutex);
        }
        data->pult->je_na_pulte_drink=true;
        pthread_cond_signal(&data->pult->odober_drink);
        pthread_mutex_unlock(&data->pult->mutex);
    }
    printf("Barman skoncil sichtu!\n");
    return NULL;
}

void * zakaznik_fun(void * arg) {
    ZAKAZNIK * data = arg;
    printf("Zakaznik %u ide do baru! Bude mu to trvat %hhd sekund!\n",data->id,data->cas_prichodu);
    sleep(data->cas_prichodu);
    printf("Zakaznik %u je v bare!\n",data->id);
    pthread_mutex_lock(&data->pult->mutex);
    while(!data->pult->je_na_pulte_drink) {
        pthread_cond_wait(&data->pult->odober_drink, &data->pult->mutex);
    }
    data->pult->je_na_pulte_drink=false;
    pthread_cond_signal(&data->pult->pridaj_drink);
    pthread_mutex_unlock(&data->pult->mutex);
    printf("Zakaznik %u si dava svoj drink a konci!\n",data->id);
    return NULL;
}

int main(int argc, char * argv[]) {
    srand(time(NULL));

    // Inicializacia pultu
    PULT pult;
    pult.je_na_pulte_drink = false;
    pthread_mutex_init(&pult.mutex, NULL);
    pthread_cond_init(&pult.pridaj_drink, NULL);
    pthread_cond_init(&pult.odober_drink, NULL);

    // Inicializacia barmana spolu s jeho vlaknom
    pthread_t barman;
    BARMAN dataB = {&pult, 10};
    if(argc > 1 ) {
        dataB.pocet_drinkov = atoi(argv[1]);
    }
    pthread_create(&barman, NULL, barman_fun, &dataB);

    // Inicializacia zakaznikov spolu s ich vlaknami
    pthread_t zakaznici[dataB.pocet_drinkov];
    ZAKAZNIK dataZ[dataB.pocet_drinkov];
    for (unsigned int i = 0; i < dataB.pocet_drinkov; ++i) {
        dataZ[i].id=i+1;
        dataZ[i].pult=&pult;
        dataZ[i].cas_prichodu= rand() % 8 + 1;
        pthread_create(&zakaznici[i], NULL, zakaznik_fun, &dataZ[i]);
    }

    // Spustenie simulacie a spojenie vlakien
    printf("Bar otvoreny!\n");
    pthread_join(barman,NULL);
    for (int i = 0; i < dataB.pocet_drinkov; ++i) {
        pthread_join(zakaznici[i],NULL);
    }

    // Skoncenie simulacie, zrusenie vlakien
    printf("Bar zatvoreny!\n");
    pthread_mutex_destroy(&pult.mutex);
    pthread_cond_destroy(&pult.pridaj_drink);
    pthread_cond_destroy(&pult.odober_drink);

    return 0;
}