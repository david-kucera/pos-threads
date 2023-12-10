#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int n, k, m;

typedef struct stol {
    int aktualny_pocet;

    pthread_mutex_t mutex;
    pthread_cond_t je_plny;
    pthread_cond_t je_prazdny;
} STOL;

typedef struct zamestnanec {
    int id;
    int cas_napadu;
    int cas_prestavka;
    STOL * stol;
} ZAMESTNANEC;

typedef struct manazerka {
    STOL * stol;
    int pocet_vydanych;
    int spracovanie_hromady;
    int interval;
} MANAZERKA;

void init_stol(STOL * stol) {
    stol->aktualny_pocet = 0;
    pthread_mutex_init(&stol->mutex, NULL);
    pthread_cond_init(&stol->je_prazdny, NULL);
    pthread_cond_init(&stol->je_plny, NULL);
}

void destroy_stol(STOL * stol) {
    stol->aktualny_pocet = 0;
    pthread_mutex_destroy(&stol->mutex);
    pthread_cond_destroy(&stol->je_plny);
    pthread_mutex_destroy(&stol->je_prazdny);
}

void * zam_fun(void * arg) {
    ZAMESTNANEC * data = (ZAMESTNANEC *)arg;

    for (int i = 0; i < m; ++i) {
        sleep(data->cas_napadu);
        pthread_mutex_lock(&data->stol->mutex);
        while (data->stol->aktualny_pocet == k) {
            printf("Zamestnanec %i caka, lebo stol je plny!\n", data->id);
            pthread_cond_wait(&data->stol->je_plny, &data->stol->mutex);
        }
        pthread_cond_signal(&data->stol->je_prazdny);
        data->stol->aktualny_pocet++;
        printf("Zamestnanec %i pridal napad na stol. Aktualne je na stole %i napadov.\n", data->id, data->stol->aktualny_pocet);
        pthread_mutex_unlock(&data->stol->mutex);

        sleep(data->cas_prestavka);
    }

    printf("Zamestnanec %i skoncil vo firme!\n", data->id);
    return NULL;
}

void * man_fun(void * arg) {
    MANAZERKA * data = (MANAZERKA *)arg;

    while (1) {
        sleep(data->interval);
        pthread_mutex_lock(&data->stol->mutex);
        while (data->stol->aktualny_pocet < 5) {
            printf("Manazerka caka, lebo na stole nie je dostatok napadov!\n");
            pthread_cond_wait(&data->stol->je_prazdny, &data->stol->mutex);
        }
        pthread_cond_signal(&data->stol->je_plny);
        printf("Manazerka si vzala zo stola 5 napadov a ide ich ohodnotit.\n");
        data->stol->aktualny_pocet -= 5;
        sleep(data->spracovanie_hromady);
        int hod1 = rand() % 6 + 1;
        int hod2 = rand() % 6 + 1;
        int rozdiel = abs(hod1 - hod2);
        if (rozdiel > 3) {
            data->pocet_vydanych++;
        }
        pthread_mutex_unlock(&data->stol->mutex);
        printf("Manazerka ohodnotila napady. Zatial vyslo %i hier.\n", data->pocet_vydanych);
    }
    return NULL;
}

int main(int argc, char * argv[]) {
    srand(time(NULL));
    if (argc != 2) {
        n = 10; // pocet zamestnancov
        k = 10; // kapacita stola
        m = 5; // pocet napadov na pracovnika
    } else {
        n = atoi(argv[1]);
        k = atoi(argv[2]);
        m = atoi(argv[3]);
    }

    STOL stol;
    init_stol(&stol);

    ZAMESTNANEC zamestnanci[n];
    pthread_t zamestnancit[n];
    for (int i = 0; i < n; ++i) {
        zamestnanci[i].id = i + 1;
        zamestnanci[i].cas_napadu = 3;
        zamestnanci[i].cas_prestavka = 2;
        zamestnanci[i].stol = &stol;
        pthread_create(&zamestnancit[i], NULL, zam_fun, &zamestnanci[i]);
    }

    MANAZERKA manazerka;
    manazerka.spracovanie_hromady = 2;
    manazerka.interval = 5;
    manazerka.pocet_vydanych = 0;
    manazerka.stol = &stol;
    pthread_t manazerkat;
    pthread_create(&manazerkat, NULL, man_fun, &manazerka);

    pthread_join(manazerkat, NULL);

    for (int i = 0; i < n; ++i) {
        pthread_join(zamestnancit[i], NULL);
    }

    printf("Koniec programu.\n");
    destroy_stol(&stol);
    return 0;
}