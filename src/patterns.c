#include <stdio.h> // just for debug
#include <string.h>
#include <assert.h>

#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>

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


void reduceSerie (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);
    
    if (nJob > 1){
        
            for(int i=0; i < nJob; i++)
                worker(dest , dest , src + i * sizeJob);
            
        
    } else {
        memcpy (dest, src, sizeof(TYPE));
    
     }
}


void reduce (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);
    
    if (nJob > 1){
        
        //Por causa do malloc
        if(nJob >= 10000){

       
        //int size = 1000;
        //iniciar a 0

            int size = nJob/1000;

            void* array = malloc(size * sizeJob);

            //void* tmp = malloc(sizeJob);
            //reduceSerie(tmp, src, nJob, sizeJob, worker);
            //printf("%lf\n", *(double*)tmp);


            cilk_for(int i = 0; i < 1000 ; i++) {

                int first = i * size;

                void* aux = array + i * sizeJob;

                worker(aux, src + first * sizeJob, src + (first + 1) * sizeJob);

                for(int j = 2; j < size ; j++)
                    worker(aux, aux, src + (first + j) * sizeJob);


            }
            for(int i=0; i < size; i++)
                worker(dest , dest , array + i * sizeJob);

            //Restantes 
            for (int j=(1000*size); j<nJob; j++ )
                worker(dest, dest, src + j * sizeJob);



            //printf("%lf\n", *(double*)dest);


        }else{
            for(int i=0; i < nJob; i++)
                worker(dest , dest , src + i * sizeJob);
        }
         
        
    } else {
        memcpy (dest, src, sizeof(TYPE));
    
     }
}




void scan (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
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

void pipeline (void *dest, void *src, size_t nJob, size_t sizeJob, void (*workerList[])(void *v1, const void *v2), size_t nWorkers) {
    /* To be implemented */
    for (int i=0; i < nJob; i++) {
        memcpy (dest + i * sizeJob, src + i * sizeJob, sizeJob);
        for (int j = 0;  j < nWorkers;  j++)
            workerList[j](dest + i * sizeJob, dest + i * sizeJob);
    }
}

void farm (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2), size_t nWorkers) {
    /* To be implemented */
    map (dest, src, nJob, sizeJob, worker);
}
