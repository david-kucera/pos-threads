#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define MIN_REGAL 3

typedef struct regal {
    int aktualny_pocet;
    int max_pocet;

    pthread_mutex_t mutex;
    pthread_cond_t je_prazdny;
    pthread_cond_t je_plny;
} REGAL;

typedef struct zakaznik {
    int cas_prichodu;
    int poziadavka;
    int id;
    REGAL * regal;
} ZAKAZNIK;

typedef struct pracovnici {
    int cas_na_2_darceky;
    REGAL * regal;
} PRACOVNICI;

void init_regal(REGAL * regal, int max) {
    regal->max_pocet = max;
    regal->aktualny_pocet = regal->max_pocet/2;
    pthread_mutex_init(&regal->mutex, NULL);
    pthread_cond_init(&regal->je_plny, NULL);
    pthread_cond_init(&regal->je_prazdny, NULL);
}

void destroy_regal(REGAL * regal) {
    regal->max_pocet = 0;
    regal->aktualny_pocet = 0;
    pthread_mutex_destroy(&regal->mutex);
    pthread_cond_destroy(&regal->je_prazdny);
    pthread_cond_destroy(&regal->je_plny);
}

void * zakaznik_fun(void * arg) {
    srand(time(NULL));
    ZAKAZNIK * data = (ZAKAZNIK *)arg;

    printf("Zakaznik %i prisiel do obchodu!\n", data->id);
    data->poziadavka = rand() % 5 + 1;

    pthread_mutex_lock(&data->regal->mutex);
    if(data->regal->aktualny_pocet < data->poziadavka) {
        int berie = data->regal->aktualny_pocet;
        data->regal->aktualny_pocet -= berie;
        printf("Zakaznik %i je nespokojny, pretoze chcel %i darcekov a na regali ostalo %i!!!\n", data->id, data->poziadavka, data->regal->aktualny_pocet);
    } else {
        data->regal->aktualny_pocet -= data->poziadavka;
        printf("Zakaznik %i je spokojny, berie si so sebou %i darcekov, na regali ostalo %i.\n", data->id, data->poziadavka, data->regal->aktualny_pocet);
    }
    pthread_cond_signal(&data->regal->je_prazdny);
    pthread_mutex_unlock(&data->regal->mutex);

    pthread_exit(NULL);
    return NULL;
}

void * predavaci_fun(void * arg) {
    srand(time(NULL));
    PRACOVNICI * data = (PRACOVNICI *)arg;
    int kriticka_dolna_hranica = data->regal->aktualny_pocet/3;
    int kriticka_horna_hranica = (data->regal->aktualny_pocet/3)*2;

    while (1) {
        pthread_mutex_lock(&data->regal->mutex);

        while (data->regal->aktualny_pocet < kriticka_dolna_hranica && data->regal->aktualny_pocet < kriticka_horna_hranica) {
            printf("Zamestnanci vylozili dost a cakaju...\n");
            pthread_cond_wait(&data->regal->je_prazdny, &data->regal->mutex);
        }

        printf("Pracovnik vyklada darceky na regal...\n");
        sleep(data->cas_na_2_darceky);
        data->regal->aktualny_pocet += 2;
        printf("Aktualny pocet darcekov na regali je %i\n", data->regal->aktualny_pocet);

        if (data->regal->aktualny_pocet >= kriticka_horna_hranica) {
            pthread_cond_signal(&data->regal->je_plny);
        }

        pthread_mutex_unlock(&data->regal->mutex);
    }

    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char * argv[]) {
    int k;
    if (argc != 2) {
        k = MIN_REGAL;
    } else {
        int hodnota = atoi(argv[1]);
        if(hodnota < MIN_REGAL) {
            k = 3;
        } else {
            k = hodnota;
        }
    }

    REGAL regal;
    init_regal(&regal, k);
    printf("PROGRAM SA SPUSTA!\n");
    printf("REGAL MA KAPACITU %i DARCEKOV!\nAKTUALNE MA REGAL %i DARCEKOV\n", k, regal.aktualny_pocet);

    PRACOVNICI pracovnici_data;
    pracovnici_data.cas_na_2_darceky = 1;
    pracovnici_data.regal = &regal;

    pthread_t pracovnicit;
    pthread_create(&pracovnicit, NULL, predavaci_fun, &pracovnici_data);

    ZAKAZNIK zakaznik_data[24];
    pthread_t zakaznikt[24];
    for (int i = 0; i < 24; ++i) {
        zakaznik_data[i].id = i + 1;
        zakaznik_data[i].regal = &regal;
        zakaznik_data[i].cas_prichodu = 2 + rand() % 2;

        pthread_create(&zakaznikt[i], NULL, zakaznik_fun, &zakaznik_data[i]);

        // Wait for the current customer thread to finish before starting the next one
        pthread_join(zakaznikt[i], NULL);
    }

    pthread_join(pracovnicit, NULL);

    destroy_regal(&regal);
    return 0;
}