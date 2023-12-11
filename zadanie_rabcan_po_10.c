#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int a, b, c;

typedef struct buffer {
    int capacity;
    int size;
    int *data;

    pthread_mutex_t mutex;
    pthread_cond_t je_plny;
    pthread_cond_t je_prazdny;
} BUFFER;

typedef struct thread_data {
    BUFFER * buffer;
} THREAD_DATA;

void init_buffer(BUFFER * buffer, int capacity) {
    buffer->capacity = capacity;
    buffer->size = 0;
    buffer->data = calloc(capacity, sizeof(int));
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->je_prazdny, NULL);
    pthread_cond_init(&buffer->je_plny, NULL);
}

void destroy_buffer(BUFFER * buffer) {
    buffer->size = 0;
    free(buffer->data);
    buffer->data = NULL;
    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->je_plny);
    pthread_cond_destroy(&buffer->je_prazdny);
}

_Bool push_buffer(BUFFER * buffer, int value) {
    if (buffer->capacity > buffer->size) {
        buffer->data[buffer->size] = value;
        buffer->size++;
        return true;
    } else {
        return false;
    }
}

_Bool pop_buffer(BUFFER * buffer, int *data) {
    if (buffer->size > 0) {
        buffer->size--;
        *data = buffer->data[buffer->size];
        return true;
    } else {
        return false;
    }
}

void * gen_fun(void * arg) {
    THREAD_DATA *data = (THREAD_DATA *)arg;

    while (1) {
        // vygenerujem cislo
        int vygenerovane_cislo = rand() % b + a;

        // ideme pridat cislo do buffra
        pthread_mutex_lock(&data->buffer->mutex);
        while (!push_buffer(data->buffer, vygenerovane_cislo)) {
            // ak sa neda vlozit, cakame na uvolnenie buffra
            printf("Generujuce vlakno caka na uvolnenie buffra...\n");
            pthread_cond_wait(&data->buffer->je_plny, &data->buffer->mutex);
        }
        printf("Generujuce vlakno vygenerovalo cislo: %i\n", vygenerovane_cislo);
        // ak je buffer volny, pridame cislo do neho
        pthread_cond_signal(&data->buffer->je_prazdny);
        pthread_mutex_unlock(&data->buffer->mutex);
    }
    return NULL;
}

void * sprac_fun(void * arg) {
    THREAD_DATA * data = (THREAD_DATA *)arg;

    while (1) {
        pthread_mutex_lock(&data->buffer->mutex);
        // vyberieme cislo z buffra
        int cislo;
        while (!pop_buffer(data->buffer, &cislo)) {
            printf("Matematicke vlakno caka, lebo buffer je prazdny!\n");
            pthread_cond_wait(&data->buffer->je_prazdny, &data->buffer->mutex);
        }
        pthread_cond_signal(&data->buffer->je_plny);
        // vyhodnotenie
        if (cislo % 2 == 0) {
            printf("Cislo %i je parne!\n", cislo);
        } else {
            printf("Cislo %i je neparne!\n", cislo);
        }
        pthread_mutex_unlock(&data->buffer->mutex);
    }
    return NULL;
}

int main(int argc, char * argv[]) {
    srand(time(NULL));
    if (argc != 4) {
        a = 0;
        b = 10;
        c = 5;
    } else {
        a = atoi(argv[1]);
        b = atoi(argv[2]);
        c = atoi(argv[3]);
    }

    BUFFER buffer;
    init_buffer(&buffer, c);

    THREAD_DATA data;
    data.buffer = &buffer;
    pthread_t gent, vypt;
    pthread_create(&gent, NULL, gen_fun, &data);
    pthread_create(&vypt, NULL, sprac_fun, &data);

    pthread_join(gent, NULL);
    pthread_join(vypt, NULL);

    destroy_buffer(&buffer);
    return 0;
}