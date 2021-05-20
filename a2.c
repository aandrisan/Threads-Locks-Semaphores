#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

sem_t* sem1=NULL;
sem_t* sem2=NULL;

typedef struct{
    int id;
    int* pr2V;
    int* pr3V;
    pthread_mutex_t* lock1;
    pthread_cond_t* cond1;
    pthread_cond_t* cond2;
}proces2;

typedef struct{
    int id;
    int* nrThr;
    int* nrThr2;
    int* pr5V;
    int* pr5V1;
    pthread_mutex_t* lock1;
    pthread_cond_t* cond1;
    sem_t* sem;
    pthread_cond_t* cond2;
}proces5;

void* threadFunc2(void* arg){
    proces2* thr=(proces2*)arg;
    if(thr->id==4){
        sem_wait(sem2);
    }

    pthread_mutex_lock(thr->lock1);
        while(*thr->pr2V==1 && thr->id==3){
            pthread_cond_wait(thr->cond1,thr->lock1);
        }
    pthread_mutex_unlock(thr->lock1);

    info(BEGIN,2,thr->id);
    if(thr->id==1){
        pthread_mutex_lock(thr->lock1);
            *thr->pr2V=2;
        pthread_mutex_unlock(thr->lock1);
        pthread_cond_signal(thr->cond1);
    }

    pthread_mutex_lock(thr->lock1);
        while(thr->id==1 && *thr->pr3V==1){
            pthread_cond_wait(thr->cond2,thr->lock1);
        }
    pthread_mutex_unlock(thr->lock1);

    info(END,2,thr->id);
    if(thr->id==4){
        sem_post(sem1);
    }
    if(thr->id==3){
        pthread_mutex_lock(thr->lock1);
            *thr->pr3V=2;
        pthread_mutex_unlock(thr->lock1);
        pthread_cond_signal(thr->cond2);
    }
    return NULL;
}

void* threadFunc5(void* arg){
    proces5* thr=(proces5*)arg;
    sem_wait(thr->sem);
    info(BEGIN,5,thr->id);
    pthread_mutex_lock(thr->lock1);
        (*thr->nrThr2)++;
    pthread_mutex_unlock(thr->lock1);
        if(*thr->nrThr2<=44 || *thr->pr5V1==1){
            if(thr->id==15){
                pthread_mutex_lock(thr->lock1);
                *thr->pr5V=1; *thr->pr5V1=1;
                pthread_cond_wait(thr->cond1,thr->lock1);
                info(END,5,thr->id);
                *thr->pr5V=0;
                pthread_mutex_unlock(thr->lock1);
                pthread_cond_broadcast(thr->cond2);
            }
            if(*thr->pr5V==1){
                pthread_mutex_lock(thr->lock1);
                (*thr->nrThr)++;
                if(*thr->nrThr<3){
                    pthread_cond_wait(thr->cond2,thr->lock1);
                }else{
                    pthread_cond_signal(thr->cond1);
                    pthread_cond_wait(thr->cond2,thr->lock1);
                }
                pthread_mutex_unlock(thr->lock1);
            }
            if(thr->id!=15){
                info(END,5,thr->id);
            }
        }else{
            if(*thr->nrThr2!=48){
                pthread_mutex_lock(thr->lock1);
                if(thr->id==15){
                    pthread_cond_wait(thr->cond1,thr->lock1);
                    info(END,5,thr->id);
                }else{
                    pthread_cond_wait(thr->cond2,thr->lock1);
                    info(END,5,thr->id);
                }
                pthread_mutex_unlock(thr->lock1);
            }else{
                pthread_cond_signal(thr->cond1);
                info(END,5,thr->id);
                pthread_cond_broadcast(thr->cond2);
            }
        }
    sem_post(thr->sem);
    return NULL;
}

void* threadFunc6 (void* arg){
    int* thr=(int*) arg;
    if(*thr==5){
        sem_wait(sem1);
    }

    info(BEGIN,6,*thr);
    info(END,6,*thr);

    if(*thr==1){
        sem_post(sem2);
    }
    return NULL;
}

void threadP2(){
    pthread_t threads[4];
    proces2 thr[4];
    pthread_mutex_t lock1=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond1=PTHREAD_COND_INITIALIZER;
    pthread_cond_t cond2=PTHREAD_COND_INITIALIZER;
    int pr2V=1;
    int pr3V=1;
    for(int i=0; i<4; i++){
        thr[i].id=(i+1); thr[i].pr2V=&pr2V; thr[i].pr3V=&pr3V;
        thr[i].lock1=&lock1;
        thr[i].cond1=&cond1; thr[i].cond2=&cond2;
        pthread_create(&threads[i],NULL,threadFunc2,&thr[i]);
    }

    for(int i=0; i<4;i++){
        pthread_join(threads[i],NULL);
    }
    pthread_mutex_destroy(&lock1);
    pthread_cond_destroy(&cond1); pthread_cond_destroy(&cond2);

}

void threadP5(){
    pthread_t thr[48];
    proces5 threa[48];
    pthread_mutex_t lock1=PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond1=PTHREAD_COND_INITIALIZER;
    pthread_cond_t cond2=PTHREAD_COND_INITIALIZER;
    sem_t semafor;
    int nrThr=0, nrThr2=0, pr5V=0, pr5V1=0;
    if(sem_init(&semafor,0,4)!=0){
        perror("Eroare la initiere semafor");
        return;
    }

    for(int i=0; i<48; i++){
        threa[i].nrThr2=&nrThr2; threa[i].nrThr=&nrThr; threa[i].pr5V1=&pr5V1; threa[i].pr5V=&pr5V;
        threa[i].id=(i+1); threa[i].lock1=&lock1; threa[i].cond2=&cond2;
        threa[i].cond1=&cond1; threa[i].sem=&semafor;
        pthread_create(&thr[i],NULL,threadFunc5,&threa[i]);
    }

    for(int i=0; i<48; i++){
        pthread_join(thr[i],NULL);
    }
    pthread_mutex_destroy(&lock1); pthread_cond_destroy(&cond1); sem_destroy(&semafor);
     pthread_cond_destroy(&cond2);
}

void threadP6(){
    pthread_t threads[6];
    int thr[6];
    for(int i=0; i<6; i++){
        thr[i]=(i+1);
        pthread_create(&threads[i],NULL,threadFunc6,&thr[i]);
    }
    for(int i=0; i<6;i++){
        pthread_join(threads[i],NULL);
    }
}

int main(){
    init();

    info(BEGIN, 1, 0);
    sem1= sem_open("sem1Punct4", O_CREAT, 0644, 0);
    sem2= sem_open("sem2Punct4", O_CREAT, 0644, 0);
     if(sem1 == NULL) {
        perror("Could not aquire the semaphore");
        return -1;
    }
     if(sem2 == NULL) {
        perror("Could not aquire the semaphore");
        return -1;
    }

    pid_t pid2=fork();
    if(pid2==-1){
        perror("Procesul nu s-a putu crea");
        return -1;
    }else if(pid2==0){
        info(BEGIN,2,0);
        threadP2();
        info(END,2,0);
        exit(0);
    }

    pid_t pid3=fork();
    if(pid3==-1){
        perror("Procesul nu s-a putu crea");
        return -1;
    }else if(pid3==0){
        info(BEGIN,3,0);
        info(END,3,0);
        exit(0);
    }

    pid_t pid4=fork();
    if(pid4==-1){
        perror("Procesul nu s-a putu crea");
        return -1;
    }else if(pid4==0){
        info(BEGIN,4,0);
        pid_t pid6=fork();

        if(pid6==-1){
            perror("Procesul nu s-a putu crea");
            return -1;
        }else if(pid6==0){
            info(BEGIN,6,0);

            pid_t pid7=fork();
            if(pid7==-1){
                perror("Procesul nu s-a putu crea");
                return -1;
            }else if(pid7==0){
                info(BEGIN,7,0);
                info(END,7,0);
                exit(0);
            }
            waitpid(pid7,NULL,0);
            threadP6();
            info(END,6,0);
            exit(0);
        }
        waitpid(pid6,NULL,0);
        info(END,4,0);
        exit(0);
    }

    pid_t pid5=fork();
    if(pid5==-1){
        perror("Procesul nu s-a putu crea");
        return -1;
    }else if(pid5==0){
        info(BEGIN,5,0);

        pid_t pid8=fork();
        if(pid8==-1){
            perror("Procesul nu s-a putu crea");
            return -1;
        }else if(pid8==0){
            info(BEGIN,8,0);
            info(END,8,0);
            exit(0);
        }
        waitpid(pid8,NULL,0);
        threadP5();
        info(END,5,0);
        exit(0);
    }

    sem_unlink("sem1Punct4"); sem_unlink("sem2Punct4");
    waitpid(pid2,NULL,0);
    waitpid(pid3,NULL,0);
    waitpid(pid4,NULL,0);
    waitpid(pid5,NULL,0);

    info(END,1,0);
    return 0;
}
