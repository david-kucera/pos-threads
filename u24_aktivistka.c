#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int n, k;

typedef struct pult {
    int aktualny_pocet;
    int pocet_predanych;
    int pocet_el;
    pthread_mutex_t mutex;
    pthread_cond_t je_plno;
    pthread_cond_t je_prazdno;
} PULT;

typedef struct aktivistka {
    int cas_vykladania;
    PULT * pult;
} AKTIVISTKA;

typedef struct zakaznik {
    int id;
    int cas_prichodu;
    PULT * pult;
} ZAKAZNIK;

void init_pult(PULT * pult) {
    pult->aktualny_pocet = 0;
    pult->pocet_predanych = 0;
    pult->pocet_el = 0;
    pthread_mutex_init(&pult->mutex, NULL);
    pthread_cond_init(&pult->je_plno, NULL);
    pthread_cond_init(&pult->je_prazdno, NULL);
}

void destroy_pult(PULT * pult) {
    pult->aktualny_pocet = 0;
    pult->pocet_predanych = 0;
    pult->pocet_el = 0;
    pthread_mutex_destroy(&pult->mutex);
    pthread_cond_destroy(&pult->je_prazdno);
    pthread_cond_destroy(&pult->je_plno);
}

void * zak_fun(void * arg) {
    ZAKAZNIK * data = (ZAKAZNIK *)arg;
    sleep(data->cas_prichodu);

    pthread_mutex_lock(&data->pult->mutex);
    while (data->pult->aktualny_pocet <= 0) {
        printf("Zakaznik %i prisiel a zistil, ze nie su noviny. Odchadza sklamany.\n", data->id);
        pthread_mutex_unlock(&data->pult->mutex);
        return NULL;
    }
    data->pult->aktualny_pocet--;
    data->pult->pocet_predanych++;
    if ((rand() % 100) > 35) {
        printf("Zakaznik %i kupuje tlacenu verziu. Aktualny pocet novin na pulte: %i\n", data->id, data->pult->aktualny_pocet);
        printf("HOW DARE YOU!!!\n");
    } else {
        printf("Zakaznik %i kupuje elektronicku verziu. Aktualny pocet novin na pulte: %i\n", data->id, data->pult->aktualny_pocet);
        data->pult->pocet_el++;
    }

    pthread_mutex_unlock(&data->pult->mutex);

    return NULL;;
}

void * akt_fun(void * arg) {
    AKTIVISTKA * data = (AKTIVISTKA *)arg;

    printf("Aktivistka vylozila na zaciatok vsetky noviny na pult.\n");
    pthread_mutex_lock(&data->pult->mutex);
    for (int i = 0; i < n; ++i) {
        data->pult->aktualny_pocet++;
    }
    printf("Aktualny pocet novin na pulte je %i\n", data->pult->aktualny_pocet);
    pthread_mutex_unlock(&data->pult->mutex);

    int kriticka_hranica = (n / 3);

    while (data->pult->pocet_predanych < k) {
        if (data->pult->aktualny_pocet <= kriticka_hranica) {
            sleep(data->cas_vykladania);
            pthread_mutex_lock(&data->pult->mutex);
            data->pult->aktualny_pocet++;
            printf("Aktivistka vylozila noviny na pult. Aktualny pocet na pulte je %i\n", data->pult->aktualny_pocet);
            pthread_mutex_unlock(&data->pult->mutex);
        }
    }

    printf("Aktivistka predala vsetky noviny co mala a konci.\n");
    return NULL;
}

int main(int argc, char * argv[]) {
    srand(time(NULL));
    if (argc != 3) {
        n = 5;
        k = 30;
    } else {
        n = atoi(argv[1]);
        k = atoi(argv[2]);
    }

    printf("Stanok sa otvara.\n");
    PULT pult;
    init_pult(&pult);

    AKTIVISTKA aktivistka;
    aktivistka.cas_vykladania = 1;
    aktivistka.pult = &pult;
    pthread_t aktivistkat;
    pthread_create(&aktivistkat, NULL, akt_fun, &aktivistka);

    ZAKAZNIK zak_data[k];
    pthread_t zakaznici[k];
    for (int i = 0; i < k; ++i) {
        zak_data[i].cas_prichodu = 1 + rand() % 3;
        zak_data[i].pult = &pult;
        zak_data[i].id = i + 1;
        pthread_create(&zakaznici[i], NULL, zak_fun, &zak_data[i]);
        pthread_join(zakaznici[i], NULL);
    }

    pthread_join(aktivistkat, NULL);

    printf("Stanok sa zatvara.\n");

    printf("Percento elektronicky verzii novin: %d", (pult.pocet_el / pult.pocet_predanych) * 100);
    destroy_pult(&pult);
    return 0;
}