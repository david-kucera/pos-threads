#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define POCET_HUB 15
#define KAPACITA_PULTU 5

int n, skonceni_hubari;

typedef struct pult {
    int aktualny_pocet;
    int huby[KAPACITA_PULTU];
    pthread_mutex_t mutex;
    pthread_cond_t je_plno;
    pthread_cond_t je_prazdno;
} PULT;

typedef struct hubar {
    int id;
    int cas_cesty;
    int zarobok;
    int huby[POCET_HUB];    // 1 = bedla 2 = dubak 3 = kozak 4 = muchotravka
    PULT * pult;
} HUBAR;

typedef struct susic {
    PULT * pult;
    int cas_dobra_huba;
    int cas_zla_huba;
    int pocty_nasusenych_druhy[3]; // 0 = bedla 1 = dubak 2 = kozak
} SUSIC;

void init_pult(PULT * pult) {
    pult->aktualny_pocet = 0;
    pthread_mutex_init(&pult->mutex, NULL);
    pthread_cond_init(&pult->je_plno, NULL);
    pthread_cond_init(&pult->je_prazdno, NULL);
}

void destroy_pult(PULT * pult) {
    pult->aktualny_pocet = 0;
    pthread_mutex_destroy(&pult->mutex);
    pthread_cond_destroy(&pult->je_plno);
    pthread_cond_destroy(&pult->je_prazdno);
}

void * hubar_fun(void * arg) {
    HUBAR * data = (HUBAR *)arg;
    printf("Hubar %i zacina svoju cinnost!\n", data->id);
    for (int i = 0; i < POCET_HUB; ++i) {
        printf("Hubar %i prichadza s %i. hubou ku susicovi.\n", data->id, i+1);
        sleep(data->cas_cesty);
        pthread_mutex_lock(&data->pult->mutex);
        while (data->pult->aktualny_pocet >= KAPACITA_PULTU) {
            printf("Hubar %i caka na uvolnenie pultu.\n", data->id);
            pthread_cond_wait(&data->pult->je_plno, &data->pult->mutex);
        }
        pthread_cond_signal(&data->pult->je_prazdno);
        data->pult->huby[data->pult->aktualny_pocet] = data->huby[i];
        data->pult->aktualny_pocet++;
        pthread_mutex_unlock(&data->pult->mutex);
        printf("Hubar %i prisiel k susicovi a dal hubu na pult. Aktualny pocet hub na pulte je %i.\n", data->id, data->pult->aktualny_pocet);
        int odmena;
        int typ_huby = data->huby[i];
        switch (typ_huby) {
            case 1:
                odmena = 10;
                break;
            case 2:
                odmena = 20;
                break;
            case 3:
                odmena = 5;
                break;
            default:
                odmena = 0;
                break;
        }
        data->zarobok += odmena;
    }
    printf("Hubar %i vylozil vsetky svoje huby a konci. Celkovo zarobil %i centov.\n", data->id, data->zarobok);
    skonceni_hubari++;
    return NULL;
}

void * susic_fun(void * arg) {
    SUSIC * data = (SUSIC *)arg;
    printf("Susic zacina svoju pracu!\n");
    while (skonceni_hubari != n) {
        pthread_mutex_lock(&data->pult->mutex);
        while (data->pult->aktualny_pocet == 0) {
            printf("Susic caka na doplnenie hub!\n");
            pthread_cond_wait(&data->pult->je_prazdno, &data->pult->mutex);
        }
        printf("Susic berie %i hub z pultu a ide susit!\n\n", data->pult->aktualny_pocet);
        int spracovavane[data->pult->aktualny_pocet];
        int pocet_sprac = data->pult->aktualny_pocet;
        for (int j = 0; j < data->pult->aktualny_pocet; ++j) {
            spracovavane[j] = data->pult->huby[j];
            data->pult->huby[j] = 0;
        }
        data->pult->aktualny_pocet = 0;
        pthread_cond_signal(&data->pult->je_plno);
        pthread_mutex_unlock(&data->pult->mutex);
        // Spracovanie hub
        for (int j = 0; j < pocet_sprac; ++j) {
            int typ_huby = spracovavane[j];
            switch (typ_huby) {
                case 1: // bedla
                    data->pocty_nasusenych_druhy[0]++;
                    sleep(data->cas_dobra_huba);
                    break;
                case 2: // dubak
                    data->pocty_nasusenych_druhy[1]++;
                    sleep(data->cas_dobra_huba);
                    break;
                case 3: // kozak
                    data->pocty_nasusenych_druhy[2]++;
                    sleep(data->cas_dobra_huba);
                    break;
                default: // muchotravka
                    sleep(data->cas_zla_huba);
                    break;
            }
        }
    }
    return NULL;
}

int main(int argc, char * argv[]) {
    if (argc != 2) {
        n = 5;
    } else {
        n = atoi(argv[1]);
    }

    printf("Cas susenia nastal, hor sa na to!\n");
    skonceni_hubari = 0;
    PULT pult;
    init_pult(&pult);

    HUBAR hubar_data[n];
    pthread_t hubar[n];
    for (int i = 0; i < n; ++i) {
        hubar_data[i].id = i + 1;
        hubar_data[i].pult = &pult;
        hubar_data[i].zarobok = 0;
        hubar_data[i].cas_cesty = 1 + rand() % 2;
        for (int j = 0; j < POCET_HUB; ++j) {
            int huba;
            int sanca = rand() % 100;
            if (sanca < 25) {
                huba = 1; // Bedla
            } else if (sanca > 25 && sanca < 35) {
                huba = 2; // Dubak
            } else if (sanca > 35 && sanca < 60) {
                huba = 3; // Kozak
            } else {
                huba = 4; // Muchotravka
            }
            hubar_data[i].huby[j] = huba;
        }
        pthread_create(&hubar[i], NULL, hubar_fun, &hubar_data[i]);
    }

    SUSIC susic;
    susic.pult = &pult;
    susic.cas_dobra_huba = 2;
    susic.cas_zla_huba = 0;

    pthread_t susict;
    pthread_create(&susict, NULL, susic_fun, &susic);

    pthread_join(susict, NULL);
    for (int i = 0; i < n; ++i) {
        pthread_join(hubar[i], NULL);
    }

    printf("Je nasusene! Parada!\n");
    for (int i = 0; i < n; ++i) {
        printf("Hubar %i si zarobil celkovo %i centov.\n", hubar_data[i].id, hubar_data[i].zarobok);
    }
    destroy_pult(&pult);
    return 0;
}