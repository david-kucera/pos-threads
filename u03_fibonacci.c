#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFF_SIZE 5
int n;

int getRandInt(int from, int to) {
    return from + rand() % (from + to - 1);
}

int getFib(int n) {
    if (n <= 0)
        return 0;
    if (n <= 2)
        return 1;
    return getFib(n - 1) + getFib(n - 2);

}

typedef struct CommonData {
    int *nums;
    int *produced; //number of produced but not consumed

    pthread_mutex_t *mutex;
    pthread_cond_t *pCond;
    pthread_cond_t *cCond;
} COMMON_DATA;

typedef struct ProducerData {
    COMMON_DATA *commonData;
    int currentIndex;
} P_DATA;

typedef struct ConsumerData {
    COMMON_DATA *commonData;
    int currentIndex;
} C_DATA;

void *fibProducer(void *args) {
    P_DATA *data = (P_DATA *) args;

    for (int i = 0; i < n; i++) {
        pthread_mutex_lock(data->commonData->mutex);

        while (*data->commonData->produced >= BUFF_SIZE)
            pthread_cond_wait(data->commonData->pCond, data->commonData->mutex);

        data->commonData->nums[data->currentIndex] = getFib(i);
        printf("Vygenerovane %d\n", data->commonData->nums[data->currentIndex]);
        data->currentIndex++;
        data->currentIndex %= BUFF_SIZE;
        (*data->commonData->produced)++;

        pthread_cond_signal(data->commonData->cCond);
        pthread_mutex_unlock(data->commonData->mutex);
        sleep(1);

    }

    return NULL;
}

void *fibConsumer(void *args) {
    C_DATA *data = (C_DATA *) args;

    for (int i = 0; i < n; i++) {
        pthread_mutex_lock(data->commonData->mutex);

        while (*data->commonData->produced <= 0)
            pthread_cond_wait(data->commonData->cCond, data->commonData->mutex);

//        printf("\tVypisane %d\n", data->commonData->nums[data->currentIndex]);

        data->currentIndex++;
        data->currentIndex %= BUFF_SIZE;
        (*data->commonData->produced)--;

        pthread_cond_signal(data->commonData->pCond);
        pthread_mutex_unlock(data->commonData->mutex);
        sleep(2);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    pthread_t producerThread, consumerThread;
    pthread_mutex_t mutex;
    pthread_cond_t pCond, cCond;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&pCond, NULL);
    pthread_cond_init(&cCond, NULL);


    if (argc < 2)
        n = getRandInt(7, 30);
    else
        n = atoi(argv[1]);

    printf("Generujem prvych %d fibonaciho cisel\n\n", n);

    int nums[BUFF_SIZE];
    int produced = 0;

    COMMON_DATA commonData = {nums, &produced, &mutex, &pCond, &cCond};

    P_DATA pData = {&commonData, 0};
    C_DATA cData = {&commonData, 0};

    pthread_create(&producerThread, NULL, fibProducer, &pData);
    pthread_create(&consumerThread, NULL, fibConsumer, &cData);

    pthread_join(producerThread, NULL);
    pthread_join(consumerThread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&pCond);
    pthread_cond_destroy(&cCond);

    return 0;
}
