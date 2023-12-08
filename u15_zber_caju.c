#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct sklad {
    int kapacita;
    int pocetVSklade;
    _Bool *data;
} SKLAD;

typedef struct data_plantaznici {
    int id;
    int trieda;
    SKLAD *sklad;
    int *plantaz;
    pthread_mutex_t *sklad_mutex;
    pthread_mutex_t *plantaz_mutex;
    pthread_cond_t *miesto_v_sklade;
    pthread_cond_t *nieco_v_sklade;
} PLANTAZNICI;

typedef struct data_kontrolor {
    SKLAD *sklad;
    pthread_mutex_t *sklad_mutex;
    pthread_cond_t *miesto_v_sklade;
    pthread_cond_t *nieco_v_sklade;
} KONTROLOR;

void *zbieraj_caj(void *data) {
    PLANTAZNICI *d = (PLANTAZNICI *)data;
    printf("Plantaznik %d triedy %d zacina cinnost\n", d->id, d->trieda);

    // Kym je nieco na plantazi budeme opakovat nasledujuce
    pthread_mutex_lock(d->plantaz_mutex);
    while (d->plantaz > 0) {
        pthread_mutex_unlock(d->plantaz_mutex);

        // Cesta z pristavu na plantaz
        printf("Plantaznik %d triedy %d ide na plantaz\n", d->id, d->trieda);
        if(d->trieda == 1) sleep(4);
        if(d->trieda == 2) sleep(6);
        if(d->trieda == 3) sleep(3);

        // Zber caju na plantazi
        printf("Plantaznik %d triedy %d prisiel na plantaz a ide zberat caj\n", d->id, d->trieda);
        pthread_mutex_lock(d->plantaz_mutex);
        if(d->plantaz <= 0) break;
        if(d->trieda == 1) usleep(1000);
        if(d->trieda == 2) usleep(2000);
        if(d->trieda == 3) usleep(1500);
        --d->plantaz;
        pthread_mutex_unlock(d->plantaz_mutex);

        // Cesta spat do pristavu
        printf("Plantaznik %d triedy %d zbieral caj a ide s cajom do pristavu\n", d->id, d->trieda);
        if(d->trieda == 1) sleep(2);
        if(d->trieda == 2) sleep(3);
        if(d->trieda == 3) sleep(3);
        pthread_mutex_lock(d->sklad_mutex);
        while (d->sklad->pocetVSklade >= d->sklad->kapacita) {
            pthread_cond_wait(d->miesto_v_sklade, d->sklad_mutex);
        }

        // Vylozime dodavku
        if(d->trieda == 3) d->sklad->data[d->sklad->pocetVSklade] = false;
        else d->sklad->data[d->sklad->pocetVSklade] = true;
        ++d->sklad->pocetVSklade;
        pthread_mutex_unlock(d->sklad_mutex);
        pthread_cond_signal(d->nieco_v_sklade);
        printf("Plantaznik %d triedy %d odovzdal dodavku\n", d->id, d->trieda);
        pthread_mutex_lock(d->plantaz_mutex);
    }
    pthread_mutex_unlock(d->plantaz_mutex);
}

void *kontroluj(void *data) {
    KONTROLOR *d = (KONTROLOR *)data;
    int pocet_vhodnych = 0, pocet_nevhodnych = 0;

    //kym neskontroloval vsetky dodavky opakuj
    for (int i = 0; i < 60; ++i) {
        //ak je sklad prazdny, caka
        printf("Kontrolor ide do skladu\n");
        pthread_mutex_lock(d->sklad_mutex);
        while(d->sklad->pocetVSklade <= 0) {
            pthread_cond_wait(d->nieco_v_sklade, d->sklad_mutex);
        }
        //vyberie dodavku zo skladu
        --d->sklad->pocetVSklade;
        _Bool dodavka = d->sklad->data[d->sklad->pocetVSklade];
        pthread_mutex_unlock(d->sklad_mutex);
        pthread_cond_broadcast(d->miesto_v_sklade);
        printf("Kontrolor prebral dodavku a ide kontrolovat\n");

        //skontroluje dodavku
        if(dodavka) {
            sleep(2);
            ++pocet_vhodnych;
        }
        else {
            sleep(4);
            ++pocet_nevhodnych;
        }

        //zapise si vysledok a vypise ho
        printf("Kontrolor spracoval dodavku %d, pomer vhodnych a nevhodnych: %lf\n", i, (double)pocet_nevhodnych/pocet_vhodnych);
    }
}

//int main(int args, char **argv) {
//    srand(time(NULL));
//    int pocet_plantaznikov = 10;
//    if(args > 1) {
//        pocet_plantaznikov = atoi(argv[1]);
//    }
//    pthread_mutex_t sklad_mutex, plantaz_mutex;
//    pthread_mutex_init(&sklad_mutex, NULL);
//    pthread_mutex_init(&plantaz_mutex, NULL);
//    pthread_cond_t miestoVSklade, niecoVSklade;
//    pthread_cond_init(&miestoVSklade, NULL);
//    pthread_cond_init(&niecoVSklade, NULL);
//    _Bool data[5];
//    SKLAD sklad = {5, 0, &data};
//    int plantaz = 60;
//    PLANTAZNICI plantaznikData[pocet_plantaznikov];
//    KONTROLOR kontrolorData = {
//            &sklad, &sklad_mutex, &miestoVSklade, &niecoVSklade
//    };
//    pthread_t plantaznik[pocet_plantaznikov];
//    pthread_t kontrolor;
//    for (int i = 0; i < pocet_plantaznikov; ++i) {
//        plantaznikData[i].id = i;
//        double r = (double)rand()/RAND_MAX;
//        if(r < 0.4) plantaznikData[i].trieda = 1;
//        else if(r < 0.75) plantaznikData[i].trieda = 2;
//        else plantaznikData[i].trieda = 3;
//        plantaznikData[i].sklad = &sklad;
//        plantaznikData[i].plantaz = &plantaz;
//        plantaznikData[i].sklad_mutex = &sklad_mutex;
//        plantaznikData[i].plantaz_mutex = &plantaz_mutex;
//        plantaznikData[i].miesto_v_sklade = &miestoVSklade;
//        plantaznikData[i].nieco_v_sklade = &niecoVSklade;
//        pthread_create(&plantaznik[i], NULL, zbieraj_caj, &plantaznikData[i]);
//    }
//    pthread_create(&kontrolor, NULL, kontroluj, &kontrolorData);
//    pthread_join(kontrolor, NULL);
//    for (int i = 0; i < pocet_plantaznikov; ++i) {
//        pthread_join(plantaznik[i], NULL);
//    }
//    pthread_mutex_destroy(&sklad_mutex);
//    pthread_mutex_destroy(&plantaz_mutex);
//    pthread_cond_destroy(&miestoVSklade);
//    pthread_cond_destroy(&niecoVSklade);
//    return 0;
//}