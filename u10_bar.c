#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    bool pult;
    pthread_mutex_t mutex;
    pthread_cond_t odober;
    pthread_cond_t pridaj;
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
        while(data->pult->pult) {
            pthread_cond_wait(&data->pult->pridaj,&data->pult->mutex);
        }
        data->pult->pult=true;
        pthread_cond_signal(&data->pult->odober);
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
    while(!data->pult->pult) {
        pthread_cond_wait(&data->pult->odober,&data->pult->mutex);
    }
    data->pult->pult=false;
    pthread_cond_signal(&data->pult->pridaj);
    pthread_mutex_unlock(&data->pult->mutex);
    printf("Zakaznik %u si dava svoj drink a konci!\n",data->id);
    return NULL;
}

int main(int argc, char * argv[]) {
    srand(time(NULL));

    PULT pult;
    pult.pult=false;
    pthread_mutex_init(&pult.mutex, NULL);
    pthread_cond_init(&pult.pridaj,NULL);
    pthread_cond_init(&pult.odober,NULL);

    pthread_t barman;
    BARMAN dataB = {&pult, 10};
    if(argc > 1 ) {
        dataB.pocet_drinkov = atoi(argv[1]);
    }
    pthread_create(&barman, NULL, barman_fun, &dataB);

    pthread_t zakaznici[dataB.pocet_drinkov];
    ZAKAZNIK dataZ[dataB.pocet_drinkov];
    for (unsigned int i = 0; i < dataB.pocet_drinkov; ++i) {
        dataZ[i].id=i+1;
        dataZ[i].pult=&pult;
        dataZ[i].cas_prichodu= rand() % 8 + 1;
        pthread_create(&zakaznici[i], NULL, zakaznik_fun, &dataZ[i]);
    }

    printf("Bar otvoreny!\n");
    pthread_join(barman,NULL);
    for (int i = 0; i < dataB.pocet_drinkov; ++i) {
        pthread_join(zakaznici[i],NULL);
    }
    printf("Bar zatvoreny!\n");

    pthread_mutex_destroy(&pult.mutex);
    pthread_cond_destroy(&pult.pridaj);
    pthread_cond_destroy(&pult.odober);

    return 0;
}