#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct clovek {
    bool ma_zlavu;
    float cena_jedla;
} CLOVEK;

typedef struct kassa {
    float trzba;
    unsigned int pocet_uspesnych_plavieb;
} KASSA;

typedef struct lod {
    CLOVEK * pasazieri;
    size_t kapacita;
    size_t aktualny_pocet;
    bool vyplavala;
    bool potopena;
    float cena_listka;
    pthread_mutex_t mutPas;
    pthread_mutex_t mutPot;
    pthread_cond_t nasadaj;
    pthread_cond_t plavaj;
} LOD;

void * producent(void * arg) {
    LOD * data = arg;
    int pocet;
    CLOVEK pasazier;
    pthread_mutex_lock(&data->mutPot);
    while(!data->potopena) {
        pthread_mutex_unlock(&data->mutPot);
        pocet = rand()%5+1;
        for (int i = 0; i < pocet; ++i) {
            pasazier.ma_zlavu= (rand() / (double)RAND_MAX) < 0.35;
            pasazier.cena_jedla= (rand() / (double)RAND_MAX) * 7 + 3;
            pthread_mutex_lock(&data->mutPas);
            while(data->aktualny_pocet == data->kapacita) {
                pthread_cond_wait(&data->nasadaj,&data->mutPas);
            }
            data->pasazieri[data->aktualny_pocet++]=pasazier;
            if(data->aktualny_pocet == data->kapacita) {
                data->vyplavala=true;
                pthread_cond_signal(&data->plavaj);
            }
            pthread_mutex_unlock(&data->mutPas);
        }
        sleep(5);
        pthread_mutex_lock(&data->mutPot);
    }
    pthread_mutex_unlock(&data->mutPot);
    return NULL;
}

void * konzument (void * arg) {
    LOD * data = arg;
    pthread_mutex_lock(&data->mutPot);
    KASSA pokladnicka={0, 0};
    while(!data->potopena) {
        pthread_mutex_unlock(&data->mutPot);
        pthread_mutex_lock(&data->mutPas);
        while(!data->vyplavala) {
            pthread_cond_wait(&data->plavaj,&data->mutPas);
        }
        sleep(2+rand()%4);
        pthread_mutex_lock(&data->mutPot);
        data->potopena=(rand()/(double)RAND_MAX)<0.15;
        if(data->potopena) {
            //pthread_cond_signal(&data->nasadaj);
            break;
        }
        pthread_mutex_unlock(&data->mutPot);
        for (int i = 0; i < data->kapacita; ++i) {
            pokladnicka.trzba+=data->pasazieri[i].cena_jedla;
            pokladnicka.trzba+=data->cena_listka;
            if(data->pasazieri[i].ma_zlavu) {
                pokladnicka.trzba-= data->cena_listka * 0.5;
            }
        }
        pthread_cond_signal(&data->nasadaj);
        pthread_mutex_unlock(&data->mutPas);
        pokladnicka.pocet_uspesnych_plavieb++;
        pthread_mutex_lock(&data->mutPot);
    }
    pthread_mutex_unlock(&data->mutPot);
}

int main() {
    // TODO
    printf("Hello, World!\n");
    return 0;
}