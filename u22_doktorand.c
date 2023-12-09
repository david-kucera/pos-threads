#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

typedef struct kavy {
    int *zaznamy_horkosti_kav;
    int pocet_kav;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} KAVY;

typedef struct doktorand {
    int index_kavy;
    int priemerna_spokojnost;
    KAVY * kavy;
} DOKTORAND;

typedef struct barista {
    KAVY * kavy;
} BARISTA;

void init_kavy(KAVY * kavy) {
    kavy->pocet_kav = 0;
    pthread_mutex_init(&kavy->mutex, NULL);
    pthread_cond_init(&kavy->cond, NULL);
}

void destroy_kavy(KAVY * kavy) {
    kavy->pocet_kav = 0;
    pthread_mutex_destroy(&kavy->mutex);
    pthread_cond_destroy(&kavy->cond);
}

void * doktorand_fun(void * arg) {
    DOKTORAND * data = (DOKTORAND *)arg;

    while (data->priemerna_spokojnost > 40) {
        printf("Doktorand si berie kavu od baristu.\n");
        int cas_prace = 2 + rand() % 5;
        sleep(cas_prace);
        printf("Odpracoval %i hodin.\n", cas_prace);
        data->index_kavy++;
        int horkosti = 0;
        for (int i = 0; i < data->index_kavy; ++i) {
            horkosti += data->kavy->zaznamy_horkosti_kav[i];
        }
        data->priemerna_spokojnost = horkosti / data->index_kavy+1;
        printf("Zatial je jeho priemerna spokojnost %i\n", data->priemerna_spokojnost);
    }

    printf("SOM VELMI NESPOKOJNY!!!\n");
    printf("Doktorand je velmi nespokojny a prestal navstevovat kaviaren, ktora zbankrotovala.\n");
    return NULL;
}

void * barista_fun(void * arg) {
    BARISTA * data = (BARISTA *)arg;

    while (1) {
        pthread_mutex_lock(&data->kavy->mutex);
        printf("Barista pripravuje kavu.\n");
        data->kavy->zaznamy_horkosti_kav[data->kavy->pocet_kav] = 20 + rand() % 100;
        data->kavy->pocet_kav++;
        sleep(1);
        pthread_mutex_unlock(&data->kavy->mutex);
        pthread_cond_signal(&data->kavy->cond);
        printf("Barista si musi odpocinut...\n");
        sleep(3);
    }
    return NULL;
}

int main(int argc, char * argv[]) {
    srand(time(NULL));
    printf("ZACIATOK PROGRAMU!\n");

    KAVY kavy;
    init_kavy(&kavy);

    BARISTA barista;
    barista.kavy = &kavy;

    DOKTORAND doktorand;
    doktorand.kavy = &kavy;
    doktorand.index_kavy = 0;
    doktorand.priemerna_spokojnost = 100;

    pthread_t doktorandt, baristat;
    pthread_create(&doktorandt, NULL, doktorand_fun, &doktorand);
    pthread_create(&baristat, NULL, barista_fun, &barista);

    pthread_join(doktorandt, NULL);
    pthread_join(baristat, NULL);

    destroy_kavy(&kavy);
    printf("KONIEC PROGRAMU!\n");
    return 0;
}