#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define VEL_MENO 11

typedef struct sprava SPRAVA;

typedef struct zasobnik {
    SPRAVA * spravy;
    unsigned int index;
    unsigned int pocet;
    pthread_mutex_t mut;
    pthread_cond_t odober;
    pthread_cond_t pridaj;
} ZASOBNIK;

typedef struct pouzivatel {
    char meno[VEL_MENO];
    unsigned int pocetSprav;
    unsigned int pocetNevhodnychSprav;
    double sancaPejorativ;
    ZASOBNIK * zasobnik;
} POUZIVATEL;

typedef struct kontrolor {
    unsigned int pocetSprav;
    ZASOBNIK * zasobnik;
} KONTROLOR;

struct sprava {
    POUZIVATEL * pouzivatel;
    bool jeVhodna;
};

void * kontroluj_fun(void * arg) {
    KONTROLOR * data = arg;
    unsigned int pocetSpracSprav = 0;
    unsigned int pocetAktSprav;
    SPRAVA spravy[5];
    sleep(5);
    printf("Kontrolor zacina pracovat!\n");
    while (pocetSpracSprav < data->pocetSprav) {
        pocetAktSprav=0;
        pthread_mutex_lock(&data->zasobnik->mut);
        while(data->zasobnik->index == 0) {
            pthread_cond_wait(&data->zasobnik->pridaj,&data->zasobnik->mut);
        }
        while(pocetAktSprav < 5 && data->zasobnik->index > 0) {
            spravy[pocetAktSprav++] = data->zasobnik->spravy[--data->zasobnik->index];
        }
        pthread_cond_signal(&data->zasobnik->odober);
        pthread_mutex_unlock(&data->zasobnik->mut);
        printf("Kontrolor ide spracovat %u sprav!\n",pocetAktSprav);
        for (int i = 0; i < pocetAktSprav; ++i) {
            if(!spravy[i].jeVhodna)
                spravy[i].pouzivatel->pocetNevhodnychSprav++;
            pocetSpracSprav++;
            sleep(1);
        }
    }
    printf("Kontrolor konci!\n");
    return NULL;
}

void * pis_spravy_fun(void * arg) {
    POUZIVATEL * data = arg;
    SPRAVA sprava;
    double nahoda;
    printf("Zacina pouzivatel %s!\n",data->meno);
    for (int i = 1; i <= data->pocetSprav; ++i) {
        sprava.pouzivatel = data;
        nahoda = rand()/(double)RAND_MAX;
        sprava.jeVhodna = data->sancaPejorativ<nahoda;
        printf("Pouzivatel %s vytvoril spravu a je %s!\n",data->meno,sprava.jeVhodna?"dobra":"nevhodna");
        pthread_mutex_lock(&data->zasobnik->mut);
        while(data->zasobnik->index == data->zasobnik->pocet) {
            pthread_cond_wait(&data->zasobnik->odober,&data->zasobnik->mut);
        }
        data->zasobnik->spravy[data->zasobnik->index++]=sprava;
        pthread_cond_signal(&data->zasobnik->pridaj);
        pthread_mutex_unlock(&data->zasobnik->mut);
    }
    printf("Konci pouzivatel %s!\n",data->meno);
    pthread_exit(NULL);
}

int main(int argc, char * argv[]) {
    srand(time(NULL));
    pthread_t kontrolor, cherry, dudy;
    ZASOBNIK zas;
    zas.index = 0;
    zas.pocet = 8;
    if(argc > 1) {
        zas.pocet = atoi(argv[1]);
    }
    zas.spravy = calloc(zas.pocet, sizeof(SPRAVA));

    pthread_mutex_init(&zas.mut,NULL);
    pthread_cond_init(&zas.pridaj,NULL);
    pthread_cond_init(&zas.odober,NULL);

    KONTROLOR dataK={50, &zas};
    POUZIVATEL dataC={"Cherry", 30, 0, 0.5, &zas};
    POUZIVATEL dataD={"Dudy", 20, 0, 0.3, &zas};

    printf("Zacina fungovat waitchat!\n");
    pthread_create(&kontrolor, NULL, kontroluj_fun, &dataK);
    pthread_create(&cherry, NULL, pis_spravy_fun, &dataC);
    pthread_create(&dudy, NULL, pis_spravy_fun, &dataD);
    pthread_join(kontrolor,NULL);
    pthread_join(cherry,NULL);
    pthread_join(dudy,NULL);

    printf("Konci waitchat!\n");
    printf("Vyhodnotenie komunikacie:\n");
    printf("P:%s, pocet sprav: %u, pocet zlych sprav %u, podiel %.3lf\n",dataC.meno,dataC.pocetSprav,dataC.pocetNevhodnychSprav,dataC.pocetNevhodnychSprav/(double)dataC.pocetSprav*100);
    printf("P:%s, pocet sprav: %u, pocet zlych sprav %u, podiel %.3lf\n",dataD.meno,dataD.pocetSprav,dataD.pocetNevhodnychSprav,dataD.pocetNevhodnychSprav/(double)dataD.pocetSprav*100);

    pthread_mutex_destroy(&zas.mut);
    pthread_cond_destroy(&zas.pridaj);
    pthread_cond_destroy(&zas.odober);
    free(zas.spravy);

    return 0;
}