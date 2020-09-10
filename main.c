#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>


#define NUM_PRODUCERS 3
#define NUM_CONSUMER 4
#define BOX_SIZE 6

typedef struct {
    size_t buf[BOX_SIZE];
    int occupied;
    int next_in;
    int next_out;
    pthread_mutex_t mutex;
    pthread_cond_t more;
    pthread_cond_t less;
} buffer_t;


buffer_t buffer;


void* producer(void* product){
    int i;

    for(i=0; i<40; ++i){
        size_t new_product = rand() % 1000000;

        while(buffer.occupied >= BOX_SIZE){
            pthread_cond_wait(&buffer.less, &buffer.mutex);
        }

        assert(buffer.occupied < BOX_SIZE);

        buffer.buf[buffer.next_in++] = new_product;
        printf("Push product %zu\n", new_product);

        buffer.next_in %= BOX_SIZE;
        ++buffer.occupied;

        pthread_cond_signal(&buffer.more);
        pthread_mutex_unlock(&buffer.mutex);

    }
}



void* consumer(void* params){
    int i;

    for(i = 0; i < 30; ++i)	{
        size_t product;
        pthread_mutex_lock(&buffer.mutex);

        while(buffer.occupied <= 0)	{
            pthread_cond_wait(&buffer.more, &buffer.mutex);
        }

        assert(buffer.occupied > 0);

        product = buffer.buf[buffer.next_out++];
        printf("Pop product %zu \n",product);

        buffer.next_out %= BOX_SIZE;
        --buffer.occupied;

        pthread_cond_signal(&buffer.less);
        pthread_mutex_unlock(&buffer.mutex);
        params = (void*)&product;
    }
    return params;
}


int main (int argc, char *argv[]) {
    pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMER];

    int i;

    for (i = 0; i < NUM_PRODUCERS; ++i) {
        pthread_create(&producers[i], NULL, producer, NULL);
    }

    for (i = 0; i < NUM_CONSUMER; ++i) {
        pthread_create(&consumers[i], NULL, consumer, NULL);
    }

    for (i = 0; i < NUM_CONSUMER; ++i) {
        pthread_join(consumers[i], NULL);
    }

    pthread_mutex_destroy(&buffer.mutex);
    return 0;
}