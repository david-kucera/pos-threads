#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct point {
    double x;
    double y;
} POINT;

typedef struct buffer {
    int capacity;
    int size;
    POINT *data;
} BUFFER;

typedef struct thread_data {
    long long replication_count;
    BUFFER buf;
    pthread_mutex_t mutex;
    pthread_cond_t is_empty;
    pthread_cond_t is_full;
} THREAD_DATA;

void init_buffer(BUFFER *buf, int capacity) {
    buf->capacity = capacity;
    buf->data = (POINT *)calloc(capacity, sizeof(POINT));
    buf->size = 0;
}

void destroy_buffer(BUFFER *buf) {
    buf->capacity = 0;
    buf->size = 0;
    free(buf->data);
    buf->data = NULL;
}

_Bool buffer_try_push(BUFFER *buf, POINT data) {
    if(buf->capacity > buf->size) {
        buf->data[buf->size] = data;
        ++buf->size;
        return true;
    } else {
        return false;
    }
}

_Bool buffer_try_pop(BUFFER *buf, POINT *data) {
    if(buf->size > 0) {
        --buf->size;
        *data = buf->data[buf->size];
        return true;
    } else {
        return false;
    }
}

void init_thread_data(THREAD_DATA *thread_data, long long replication_count, int buffer_size) {
    thread_data->replication_count = replication_count;
    init_buffer(&thread_data->buf, buffer_size);
    pthread_mutex_init(&thread_data->mutex, NULL);
    pthread_cond_init(&thread_data->is_empty, NULL);
    pthread_cond_init(&thread_data->is_full, NULL);
}

void destroy_thread_data(THREAD_DATA *thread_data) {
    thread_data->replication_count = 0;
    destroy_buffer(&thread_data->buf);
    pthread_mutex_destroy(&thread_data->mutex);
    pthread_cond_destroy(&thread_data->is_full);
    pthread_cond_destroy(&thread_data->is_empty);
}

// Producent - generuje body a da ich do spolocneho buffera
void *produce(void *thr_data) {
    THREAD_DATA *data = (THREAD_DATA *)thr_data;
    for (long long i = 0; i < data->replication_count; ++i) {
        // Vygenerujeme bod
        POINT point = {
                2*(rand()/(double)RAND_MAX)-1,
                2*(rand()/(double)RAND_MAX)-1
        };
        // Vlozime do buffera
        // Ak je buffer plny, pockam, kym sa neuvolni miesto
        pthread_mutex_lock(&data->mutex);
        while (!buffer_try_push(&data->buf, point)) {
            pthread_cond_wait(&data->is_empty, &data->mutex);
        }
        pthread_cond_signal(&data->is_full);
        pthread_mutex_unlock(&data->mutex);
    }
}

void *consume(void *thr_data) {
    THREAD_DATA *data = (THREAD_DATA *)thr_data;
    long long total_count = 0;
    long long inside_count = 0;
    for (long long i = 0; i < data->replication_count; ++i) {
        // Vyberieme bod z buffera
        POINT point;
        pthread_mutex_lock(&data->mutex);
        while (!buffer_try_pop(&data->buf, &point)) {
            pthread_cond_wait(&data->is_full, &data->mutex);
        }
        pthread_cond_signal(&data->is_empty);
        pthread_mutex_unlock(&data->mutex);

        // Ak lezi v kruhu, aktualizujeme pocet
        ++total_count;
        if(point.x*point.x + point.y*point.y <= 1) {
            ++inside_count;
        }
        // Vypocitam hodnotu PI
        printf("Replikacia %lld:\t%lf\n", i, 4*(double)((double)inside_count/(double) total_count));
    }
}

int main() {
    srand(time(NULL));
    THREAD_DATA td;
    init_thread_data(&td, 1000000, 10);
    pthread_t produce_thread, consume_thread;
    pthread_create(&produce_thread, NULL, produce, &td);
    pthread_create(&consume_thread, NULL, consume, &td);
    pthread_join(produce_thread, NULL);
    pthread_join(consume_thread, NULL);

    destroy_thread_data(&td);
    return 0;
}