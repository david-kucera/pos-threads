#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

int c;

typedef struct buffer {
    int size;
    int kapacita;
    float *data;

    pthread_mutex_t mutex;
    pthread_cond_t je_plny;
    pthread_cond_t je_prazdny;
} BUFFER;

typedef struct thread_data {
    float aproximacia;
    int pocet_rep;
    BUFFER * buffer;
} THREAD_DATA;

void init_buffer(BUFFER * buffer, int kapacita) {
    buffer->size = 0;
    buffer->kapacita = kapacita;
    buffer->data = calloc(kapacita, sizeof(float));
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->je_plny, NULL);
    pthread_cond_init(&buffer->je_prazdny, NULL);
}

void destroy_buffer(BUFFER * buffer) {
    buffer->size = 0;
    buffer->kapacita = 0;
    free(buffer->data);
    buffer->data = NULL;
    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->je_prazdny);
    pthread_cond_destroy(&buffer->je_plny);
}

_Bool try_push_buffer(BUFFER * buffer, float value) {
    if (buffer->kapacita > buffer->size) {
        buffer->data[buffer->size] = value;
        buffer->size++;
        return true;
    } else {
        return false;
    }
}

_Bool try_pop_buffer(BUFFER * buffer, float *data) {
    if (buffer->size > 0) {
        buffer->size--;
        *data = buffer->data[buffer->size];
        return true;
    } else {
        return false;
    }
}

float faktorial(int cislo) {
    int fak = 1;

    for (int i = cislo; i > 0; --i) {
        fak *= i;
    }

    return (float)fak;
}

void * rataj_fun(void * arg) {
    int replikacia = 0;
    THREAD_DATA * data = (THREAD_DATA *)arg;

    // Budeme aproximovat do nekonecna
    for (int i = 0; i < data->pocet_rep; ++i) {
        // vyratanie clena postupnosti
        float clen = faktorial(replikacia);
        replikacia++;
        // vlozenie clena do buffra
        pthread_mutex_lock(&data->buffer->mutex);
        while (!try_push_buffer(data->buffer, (1/clen))) {
            printf("Generujuce vlakno caka na vyprazdnenie buffra!\n");
            pthread_cond_wait(&data->buffer->je_plny, &data->buffer->mutex);
        }
        printf("Do buffra bol vlozeny clen %f\n", (float)1/clen);
        pthread_cond_signal(&data->buffer->je_prazdny);
        pthread_mutex_unlock(&data->buffer->mutex);
    }
    printf("Vlakno na ratanie clenov skoncilo!\n");
    return NULL;
}

void * aproximuj_fun(void * arg) {
    THREAD_DATA * data = (THREAD_DATA *)arg;

    // ziskame vsetky prvky z buffra a priratame ich do celkovej aproximacie
    for (int i = 0; i < (data->pocet_rep/data->buffer->kapacita); ++i) {

        for (int j = 0; j < data->buffer->kapacita; ++j) {
            float cislo;
            pthread_mutex_lock(&data->buffer->mutex);
            while (!try_pop_buffer(data->buffer, &cislo)) {
//                printf("Aproximovacie vlano caka, lebo buffer je prazdny!\n");
                pthread_cond_wait(&data->buffer->je_prazdny, &data->buffer->mutex);
            }
            pthread_cond_signal(&data->buffer->je_plny);

            // pridame cisla do aproximacie euleroveho cisla
            data->aproximacia += cislo;
//            printf("Aprox pridavame: %f\n", cislo);
            pthread_mutex_unlock(&data->buffer->mutex);
//            printf("aproximator zobral data...%f\n", cislo);
        }
        printf("aktualna aproximacia je %f\n", data->aproximacia);
    }
    printf("Vlakno na aproximovanie eulerovho cisla skoncilo!\n");
    return NULL;
}

int main(int argc, char * argv[]) {
    printf("Spusta sa program!\n");

    if (argc != 2) {
        c = 5;
    } else {
        c = atoi(argv[1]);
    }

    BUFFER buf;
    init_buffer(&buf, c);

    THREAD_DATA data;
    data.buffer = &buf;
    data.aproximacia = 0;
    data.pocet_rep = 30;

    pthread_t clen_t, aprox_t;
    pthread_create(&clen_t, NULL, rataj_fun, &data);
    pthread_create(&aprox_t, NULL, aproximuj_fun, &data);

    pthread_join(aprox_t, NULL);
    pthread_join(clen_t, NULL);

    destroy_buffer(&buf);
    printf("Program skoncil!\n");

    return 0;
}