#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_CLIENTS 5


int client_count;
int barber_works;

sem_t client_count_mutex;
sem_t barber_works_mutex; 
sem_t barber_wakeup; 


void* barber(void*);
void* client(void*);


int main(int argc, char *argv[]) 
{
  sem_init(&client_count_mutex, 0, 1);
  sem_init(&barber_works_mutex, 0, 1);
  sem_init(&barber_wakeup, 0, 0);

  pthread_t barber_thread;
  pthread_t client_thread;

  int creation_status; 

  creation_status = pthread_create(&barber_thread, NULL, barber, NULL); 
  if(creation_status)
  {
    printf("creating barber thread failed\n");
    exit(1);
  }

  creation_status = pthread_create(&client_thread, NULL, client, NULL); 
  if(creation_status)
  {
    printf("creating client thread failed\n");
    exit(1);
  }

  pthread_join(barber_thread, NULL);
  pthread_join(client_thread, NULL);
}


void* barber(void* params)
{
  while(1) 
  {
    sem_wait(&client_count_mutex); 
    if(client_count == 0)
    {
      sem_post(&client_count_mutex); 
      sem_wait(&barber_wakeup);
      
      sem_wait(&barber_works_mutex);
      barber_works = 1;
      sem_post(&barber_works_mutex);
      printf("barber works: waked up by client\n");
      sleep(3);
    
      sem_wait(&barber_works_mutex);
      barber_works = 0;
      sem_post(&barber_works_mutex);
    }
    else
    {
      sem_wait(&barber_works_mutex);
      barber_works = 1;
      sem_post(&barber_works_mutex);

      client_count--;
      printf("barber works: left %d client(s) in queue\n", client_count);
      sem_post(&client_count_mutex);
      sleep(3);
                   
      sem_wait(&barber_works_mutex);
      barber_works = 0;
      sem_post(&barber_works_mutex);
    }
  }
}


void* client(void* params)
{
  while(1)
  {
    sem_wait(&client_count_mutex);
    if(client_count == 0)
    {
      sem_wait(&barber_works_mutex);
      if(barber_works) 
      {
        client_count++;
        printf("client arrives: now %d client(s) in queue\n", client_count);
      }
      else
      {
        printf("client arrives: wakes the barber\n");
        sem_post(&barber_wakeup);
      }
      sem_post(&barber_works_mutex);
    }
    else if(client_count < MAX_CLIENTS)
    {
      client_count++;
      printf("client arrives: now %d client(s) in queue\n", client_count);
    }
    else
    {
      printf("client arrives: client queue is full\n");
    }
    sem_post(&client_count_mutex);
    sleep(2);
  }
}
