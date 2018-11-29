#include <stdio.h> // just for debug
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>
#include <unistd.h>

#include "patterns.h"

#define TYPE double

void map (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2)) {
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);
    
    int i = 0;
    cilk_for (i=0; i < nJob; i++) {
        worker(dest + i * sizeJob, src + i * sizeJob);
    }
}


void reduce (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);
    
    if (nJob > 1){  
        // If array size less than 100 000, perform reduce in serial mode
        if(nJob >= 100000){
            int size = nJob/100;
            void* auxiliarOutput = malloc(size * sizeJob);

            cilk_for(int i = 0; i < (nJob/size); i++) {
                int first = i * size;
                void* writeToPosition = auxiliarOutput + i * sizeJob;

                for(int j = 0; j < size ; j++)
                    worker(writeToPosition, writeToPosition, src + (first + j) * sizeJob);
            }

            for(int i=0; i < size; i++)
                worker(dest , dest , auxiliarOutput + i * sizeJob);

            //Rest 
            for (int j=(100*size); j<nJob; j++ )
                worker(dest, dest, src + j * sizeJob);

        }else{
            for(int i=0; i < nJob; i++)
                worker(dest , dest , src + i * sizeJob);
        }     
    } else {
        memcpy (dest, src, sizeof(TYPE));
     }
}

void scanSerial (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    /* To be implemented */
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);
    if (nJob > 1) {
        memcpy (dest, src, sizeJob);
        for (int i = 1;  i < nJob;  i++)
            worker(dest + i * sizeJob, src + i * sizeJob, dest + (i-1) * sizeJob);
    }
}



void scan (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    /* To be implemented */
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);
    if (nJob > 0) {
        memcpy (dest, src, sizeJob);

        

        for (int i = 1;  i < nJob;  i++)
            worker(dest + i * sizeJob, src + i * sizeJob, dest + (i-1) * sizeJob);
    }
}

int pack (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter) {
    /* To be implemented */
    int pos = 0;
    for (int i=0; i < nJob; i++) {
        if (filter[i]) {
            memcpy (dest + pos * sizeJob, src + i * sizeJob, sizeJob);
            pos++;
        }
    }
    return pos;
}

void gather (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter, int nFilter) {

    // Tudo Independente
    cilk_for (int i=0; i < nFilter; i++) {
    
        memcpy (dest + i * sizeJob, src + filter[i] * sizeJob, sizeJob);
    }
}

void scatter (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter) {
    /* To be implemented */
    for (int i=0; i < nJob; i++) {
        memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);
    }
}


 void firstPipThread(void* dest, void* src, size_t nJob, size_t sizeJob, int tasks[], void (*worker)(void *v1, const void *v2), size_t nWorkers){

    cilk_for (int i=0; i < nJob; i++) {
        memcpy (dest + i * sizeJob, src + i * sizeJob, sizeJob);
        for (int j = 0;  j < nWorkers; j++){
            worker(dest + i * sizeJob, src + i * sizeJob);

            
        } tasks[i] = 1;   
     
    }
}

 void contPipThread(int id, void *dest, size_t nJob, size_t sizeJob, int tasks[], void (*worker)(void *v1, const void *v2)){

    int i = 0;   
    while (i < nJob) {

        while(tasks[i] != id);

        worker(dest + i * sizeJob, dest + i * sizeJob);  
        tasks[i] +=  1; 
        i++;         
             

    } 
}

void pipeline (void *dest, void *src, size_t nJob, size_t sizeJob, void (*workerList[])(void *v1, const void *v2), size_t nWorkers) {

    int *tasks = malloc(sizeof(int) * nJob);
    

    cilk_for (int i=0; i < nJob; i++) {
        tasks[i]= 0;
        memcpy (dest + i * sizeJob, src + i * sizeJob, sizeJob);
    }


    for(int id = 0; id < nWorkers ; id++){  
        cilk_spawn contPipThread(id, dest, nJob, sizeJob, tasks, workerList[id]);
        
    } //cilk_sync  ;


}






/*void pipeline (void *dest, void *src, size_t nJob, size_t sizeJob, void (*workerList[])(void *v1, const void *v2), size_t nWorkers) {
    for (int i=0; i < nJob; i++) {
        memcpy (dest + i * sizeJob, src + i * sizeJob, sizeJob);
        for (int j = 0;  j < nWorkers; j++)
            workerList[j](dest + i * sizeJob, dest + i * sizeJob);
    }
}*/


void farm (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2), size_t nWorkers) {
    
    int count = 0;
    for(int i=0; i<nJob; i+=nWorkers){
        cilk_for(int j=i; j<count+nWorkers; j++){
            if(j<nJob)
                worker(dest + i * sizeJob, src + i * sizeJob);
            else
                usleep(100);
        }
        count++;
    }
    
    
    
}


