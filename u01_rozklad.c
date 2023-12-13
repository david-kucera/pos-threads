#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define BUFFER_SIZE 10

int aG, bG, nG;

typedef struct buffer {
    int data[BUFFER_SIZE];
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t je_prazdny;
} BUFFER;

void init_buffer(BUFFER * buf) {
    buf->count = 0;
    pthread_mutex_init(&buf->mutex, NULL);
    pthread_cond_init(&buf->je_prazdny, NULL);
}

void push_buffer(BUFFER *buf, int value) {
    pthread_mutex_lock(&buf->mutex);
    while (buf->count >= BUFFER_SIZE) {
        pthread_cond_wait(&buf->je_prazdny, &buf->mutex);
    }
    buf->data[buf->count++] = value;
    pthread_cond_signal(&buf->je_prazdny);
    pthread_mutex_unlock(&buf->mutex);
}

int pop_buffer(BUFFER * buf) {
    pthread_mutex_lock(&buf->mutex);
    while (buf->count <= 0) {
        pthread_cond_wait(&buf->je_prazdny, &buf->mutex);
    }
    int value = buf->data[--buf->count];
    pthread_cond_signal(&buf->je_prazdny);
    pthread_mutex_unlock(&buf->mutex);
    return value;
}

void *producer(void *arg) {
    BUFFER *buf = (BUFFER *)arg;

    for (int i = 0; i < nG; ++i) {
        int value = aG + rand() % (bG - aG + 1);
        push_buffer(buf, value);
        printf("Produced: %d\n", value);
    }

    pthread_exit(NULL);
}

void *consumer(void *arg) {
    BUFFER *buffer = (BUFFER *)arg;

    while (1) {
        int value = pop_buffer(buffer);
        printf("Consumed: %d, Prime Factors: ", value);

        for (int j = 2; j <= value; ++j) {
            while (value % j == 0) {
                printf("%d ", j);
                value /= j;
            }
        }

        printf("\n");

        pthread_mutex_lock(&buffer->mutex);
        if (buffer->count == 0) {
            pthread_mutex_unlock(&buffer->mutex);
            break;
        }
        pthread_mutex_unlock(&buffer->mutex);
    }

    pthread_exit(NULL);
}

int main(int argc, char * argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Chybný počet argumentov. Použitie: %s <a> <b> <n>\n", argv[0]);
        return EXIT_FAILURE;
    }

    aG = atoi(argv[1]);
    bG = atoi(argv[2]);
    nG = atoi(argv[3]);

    BUFFER buffer;
    init_buffer(&buffer);

    pthread_t producerThread, consumerThread;

    pthread_create(&producerThread, NULL, producer, (void *)&buffer);
    pthread_create(&consumerThread, NULL, consumer, (void *)&buffer);

    pthread_join(producerThread, NULL);
    pthread_join(consumerThread, NULL);

    return EXIT_SUCCESS;
}