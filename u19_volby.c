#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int n;

typedef struct stol {
    int aktualna_kapacita;
    int pocet_putty;
    int pocet_bash;

    pthread_mutex_t mutex;
    pthread_cond_t je_prazdny;
    pthread_cond_t je_ok;
} STOL;

void init_stol(STOL * stol) {
    stol->aktualna_kapacita = 0;
    stol->pocet_bash = 0;
    stol->pocet_putty = 0;
    pthread_mutex_init(&stol->mutex, NULL);
    pthread_cond_init(&stol->je_prazdny, NULL);
    pthread_cond_init(&stol->je_ok, NULL);
}

void destroy_stol(STOL * stol) {
    stol->aktualna_kapacita = 0;
    stol->pocet_bash = 0;
    stol->pocet_putty = 0;
    pthread_mutex_destroy(&stol->mutex);
    pthread_cond_destroy(&stol->je_prazdny);
    pthread_cond_destroy(&stol->je_ok);
}

typedef struct volic {
    int id;
    int preferencia;
    int cas_prichodu;
    STOL * stol;
} VOLIC;

typedef struct komisia {
    STOL * stol;
} KOMISIA;

void * volic_fun(void * arg) {
    VOLIC * data = (VOLIC *)arg;
    sleep(data->cas_prichodu);
    printf("Volic %i prisiel do volebnej miestnosti!\n", data->id);

    pthread_mutex_lock(&data->stol->mutex);
    while (data->stol->aktualna_kapacita == 0) {
        printf("Na stole nie su ziadne volebne harky! Volic %i caka!\n", data->id);
        pthread_cond_wait(&data->stol->je_prazdny, &data->stol->mutex);
    }
    pthread_cond_signal(&data->stol->je_ok);
    data->stol->aktualna_kapacita--;
    pthread_mutex_unlock(&data->stol->mutex);
    printf("Volic %i si zobral harok a ide odvolit!\n", data->id);

    int cas_volenia_s = rand() % 150 + 100;
    int cas_volenia_ms = cas_volenia_s / 100;
    sleep(cas_volenia_ms);

    pthread_mutex_lock(&data->stol->mutex);
    printf("Volic %i odvolil a vhadzuje obalku do urny.\n", data->id);
    if (data->preferencia < 35) {
        data->stol->pocet_bash++;
    } else {
        data->stol->pocet_putty++;
    }
    pthread_mutex_unlock(&data->stol->mutex);

    return NULL;
}

void * komisia_fun(void * arg) {
    KOMISIA * data = (KOMISIA *)arg;
    printf("Komisia zacina svoju pracu!\n");

    for (int i = 0; i < n; ++i) {
        pthread_mutex_lock(&data->stol->mutex);

        while (data->stol->aktualna_kapacita > 2 && i > 4) {
            printf("Komisia caka, kym sa kapacita znizi pod 2.\n");
            pthread_cond_wait(&data->stol->je_ok, &data->stol->mutex);
        }
        pthread_cond_signal(&data->stol->je_prazdny);

        printf("Komisia vyklada %i. harok na stol.\n", i + 1);
        data->stol->aktualna_kapacita++;
        pthread_mutex_unlock(&data->stol->mutex);
        printf("Aktualne je na stole %i harkov.\n", data->stol->aktualna_kapacita);
    }

    printf("Komisia skoncila. Vsetky harky boli vylozene.\n");
    return NULL;
}

int main(int argc, char * argv[]) {
    srand(time(NULL));
    if(argc != 2) {
        n = 15;
    } else {
        n = atoi(argv[1]);
    }

    STOL stol;
    init_stol(&stol);

    KOMISIA komisia;
    komisia.stol = &stol;

    pthread_t komisiat;
    pthread_create(&komisiat, NULL, komisia_fun, &komisia);

    printf("VOLBY OTVORENE!\n");

    VOLIC volic_data[n];
    pthread_t volici[n];
    for (int i = 0; i < n; ++i) {
        volic_data[i].id = i + 1;
        volic_data[i].cas_prichodu = rand() % 10 + 1;
        volic_data[i].preferencia = rand() % 100;
        volic_data[i].stol = &stol;
        pthread_create(&volici[i], NULL, volic_fun, &volic_data[i]);
    }

    pthread_join(komisiat, NULL);
    for (int i = 0; i < n; ++i) {
        pthread_join(volici[i], NULL);
    }

    printf("VOLBY UKONCENE!\n");
    printf("Celkovy pocet hlasov pre Putty: %i\n", stol.pocet_putty);
    printf("Celkovy pocet hlasov pre Bash: %i\n", stol.pocet_bash);
    destroy_stol(&stol);
    return 0;
}