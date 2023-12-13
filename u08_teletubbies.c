#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define MAX_MAZIAR 8

typedef struct maziar {
    int pocet_parnych;
    int pocet_neparnych;
    int size;
    int data[MAX_MAZIAR];

    pthread_mutex_t mutex;
    pthread_cond_t je_prazdny;
} MAZIAR;

void init_maziar(MAZIAR *maziar) {
    maziar->size = 0;
    maziar->pocet_neparnych = 0;
    maziar->pocet_parnych = 0;
    pthread_mutex_init(&maziar->mutex, NULL);
    pthread_cond_init(&maziar->je_prazdny, NULL);
}

void push_maziar(MAZIAR * maziar, int value) {
    pthread_mutex_lock(&maziar->mutex);
    while (maziar->size >= MAX_MAZIAR) {
        pthread_cond_wait(&maziar->je_prazdny, &maziar->mutex);
    }
    maziar->data[maziar->size++] = value;
    pthread_cond_signal(&maziar->je_prazdny);
    pthread_mutex_unlock(&maziar->mutex);
}

int pop_maziar(MAZIAR * maziar) {
    pthread_mutex_lock(&maziar->mutex);

    while (maziar->size <= 0) {
        pthread_cond_wait(&maziar->je_prazdny, &maziar->mutex);
    }
    int value = maziar->data[--maziar->size];
    pthread_cond_signal(&maziar->je_prazdny);
    pthread_mutex_unlock(&maziar->mutex);
    return value;
}

// Tinky generuje 1000 hodnot do maziaru
void * tinky_fun(void * arg) {
    MAZIAR * maziar = (MAZIAR *)arg;

    for (int i = 0; i < 1000; ++i) {
        int value = rand();
        push_maziar(maziar, value);
        printf("Tinky vygeneroval: %i\n", value);
    }
    pthread_exit(NULL);
}

// Lala kontroluje hodnoty v maziari, ci su parne alebo nie
void * lala_fun(void * arg) {
    MAZIAR * maziar = (MAZIAR *)arg;

    for (int i = 0; i < 1000; ++i) {
        int value = pop_maziar(maziar);

        if(value % 2 == 0) {
            maziar->pocet_parnych++;
        } else {
            maziar->pocet_neparnych++;
        }

        printf("Lala skontrolovala %i\n", value);
    }
    pthread_exit(NULL);
}

int main(int argc, char * argv[]) {
    srand(time(NULL));
    MAZIAR maziar;
    init_maziar(&maziar);

    pthread_t tinky, lala;
    pthread_create(&tinky, NULL, tinky_fun, (void *)&maziar);
    pthread_create(&lala, NULL, lala_fun, (void *)&maziar);

    pthread_join(tinky, NULL);
    pthread_join(lala, NULL);

    float res = (float)maziar.pocet_neparnych / (float)(maziar.pocet_neparnych + maziar.pocet_parnych) * 100;
    printf("Vyhodnotenie: %f\n", res);

    if (res > 60) {
        printf("Vyhrava Tinky-Winky!\n");
    } else {
        printf("Vyhrava Laa-Laa!\n");
    }


    pthread_mutex_destroy(&maziar.mutex);
    pthread_cond_destroy(&maziar.je_prazdny);
    return 0;
}