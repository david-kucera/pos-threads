#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_CAP 20
int a, b, n;

typedef struct buffer {
    unsigned int data[BUFFER_CAP];
    unsigned int size;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
} BUFFER;

void init_buffer(BUFFER * buf) {
    pthread_mutex_init(&buf->mutex, NULL);
    pthread_cond_init(&buf->condition, NULL);
    buf->size = 0;
}

void push_buffer(BUFFER * buf, int value) {
    pthread_mutex_lock(&buf->mutex);
    while (buf->size >= BUFFER_CAP) {
        pthread_cond_wait(&buf->condition, &buf->mutex);
    }
    buf->data[buf->size++] = value;
//    buf->size++;
    pthread_cond_signal(&buf->condition);
    pthread_mutex_unlock(&buf->mutex);
}

unsigned int pop_buffer(BUFFER * buf) {
    pthread_mutex_lock(&buf->mutex);
    while (buf->size <= 0) {
        pthread_cond_wait(&buf->condition, &buf->mutex);
    }
//    buf->size--;
    unsigned int value = buf->data[--buf->size];
    pthread_cond_signal(&buf->condition);
    pthread_mutex_unlock(&buf->mutex);
    return value;
}

// Vlakno postupne vybera z buffra dvojice a pre kazdu dvojicu vypocita
// kombinacne cislo
void * consume(void * arg) {
    BUFFER * buf = (BUFFER *)arg;

    for (int i = 0; i < n; ++i) {
        unsigned int num1 = pop_buffer(buf);
        unsigned int num2 = pop_buffer(buf);
        unsigned int value1 = 1;
        unsigned int value2 = 1;

        for (unsigned int j = 1; j <= num1; ++j) {
            value1 *= j;
        }
        for (unsigned int j = 1; j <= num2; ++j) {
            value2 *= j;
        }

        unsigned int result;
        if (num1 > num2) {
            result = value1 / value2;
        } else {
            result = value2 / value1;
        }

        printf("Kombinacne cislo z cisiel %u %u je %u\n", num1, num2, result);
    }
//    printf("Consumer finished\n");
    pthread_exit(NULL);
}

// Vlakno generujuce n dvojic nahodnych cisel z intervalu <a,b>
// ulozi do buffra
void * produce(void * arg) {
    BUFFER * buf = (BUFFER *)arg;

    for (int i = 0; i < n; ++i) {
        int value1 = a + rand() % (b - a + 1);
        push_buffer(buf, value1);
        int value2 = a + rand() % (b - a + 1);
        push_buffer(buf, value2);
        printf("Produced: %i %i\n", value1, value2);
    }
//    printf("Producer finished\n");
    pthread_exit(NULL);
}

int main(int argc, char * argv[]) {
    srand(time(NULL));
    if (argc != 4) {
        fprintf(stderr, "Nespravny pocet argumentov programu!\n");
        return 1;
    }
    a = atoi(argv[1]);
    b = atoi(argv[2]);
    n = atoi(argv[3]);

    BUFFER buffer;
    init_buffer(&buffer);

    pthread_t producer, consumer;

    pthread_create(&producer, NULL, produce, (void *) &buffer);
    pthread_create(&consumer, NULL, consume, (void *)&buffer);

    pthread_join(producer, NULL);

    pthread_mutex_lock(&buffer.mutex);
    pthread_cond_broadcast(&buffer.condition);
    pthread_mutex_unlock(&buffer.mutex);

    pthread_join(consumer, NULL);

    return 0;
}