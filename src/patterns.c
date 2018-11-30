#include <stdio.h> // just for debug
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <cilk/cilk.h>
#include <unistd.h>
#include <pthread.h>
#include "queue.c"
#include "patterns.h"

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

void upsweepTree (void* src, size_t lo, size_t hi, struct ScanNode* node, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    if (lo + 1 == hi){
        node->sum = *(TYPE *)(src + lo * sizeJob);
        node->index = lo;
    } else {
        size_t mid = (lo + hi) / 2;

        node->left = malloc(sizeof(struct ScanNode));
        node->right = malloc(sizeof(struct ScanNode));
        node->index = -1;
        cilk_spawn upsweepTree (src, lo, mid, node->left, sizeJob, worker);     
        upsweepTree (src, mid, hi, node->right, sizeJob, worker);
        
        cilk_sync;
        worker(&(node->sum), &(node->left->sum), &(node->right->sum));
    }
}

void downsweepTree (void* src, void* dest, size_t sizeJob, struct ScanNode* node, TYPE fromLeft, void (*worker)(void *v1, const void *v2, const void *v3)) {
    
    if(node->index != -1){
            worker(dest + node->index * sizeJob, src + node->index * sizeJob, &fromLeft);
    }else{
    
        cilk_spawn downsweepTree(src, dest, sizeJob, node->left, fromLeft, worker);
        downsweepTree(src, dest, sizeJob, node->right, fromLeft + node->left->sum, worker);
        
        cilk_sync;
    }
}

void scanTree (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    /* To be implemented */
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);

    struct ScanNode* parent = malloc(sizeof(struct ScanNode));
    upsweepTree (src, 0, nJob, parent, sizeJob, worker);
    downsweepTree (src, dest, sizeJob, parent, 0, worker);
}

 
void upsweep( void* src, void* aux, size_t lo, size_t hi,  size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    if( lo + 1 == hi ) {
        *(TYPE *) (aux + lo * sizeJob) = *(TYPE *) (src + lo*sizeJob);
        //printf("lo: %zd, hi: %zd, writing in Aux: %p\n",lo, hi, aux);      
    } else {
        size_t mid = (hi + lo) / 2;

		cilk_spawn upsweep(src, aux, lo, mid, sizeJob, worker);
        upsweep(src, aux, mid, hi, sizeJob, worker);
        
		cilk_sync;
        
		worker(aux + (hi*sizeJob - sizeJob), aux + (mid*sizeJob - sizeJob), aux + (hi*sizeJob - sizeJob));
    }
}

void downsweep (void* src, void* dest, void* aux, size_t sizeJob, size_t lo, size_t hi, TYPE sum, TYPE fromLeft, void (*worker)(void *v1, const void *v2, const void *v3)) {
	
	if(lo + 1 == hi){
		worker(dest + lo * sizeJob, src + lo * sizeJob, &fromLeft);
	}else{
		size_t mid = (hi + lo) / 2;
		cilk_spawn downsweep(src, dest,aux, sizeJob, lo, mid, *(TYPE *)(aux + mid*sizeJob - sizeJob), fromLeft, worker);	
		downsweep(src, dest, aux, sizeJob, mid, hi, (*(TYPE *)(aux + (hi*sizeJob -sizeJob)) - *(TYPE *)(aux + (mid*sizeJob -sizeJob))), *(TYPE *)(aux + mid*sizeJob -sizeJob) + fromLeft, worker);
		
		//implicit cilk_sync;		
	}
}

void scan (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    /* To be implemented */
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);

    //struct ScanNode* parent = malloc(sizeof(struct ScanNode));
	  void* aux = malloc(nJob*sizeJob);
	
    upsweep(src, aux, 0, nJob, sizeJob, worker);
	downsweep (src, dest, aux, sizeJob, 0, nJob, *(TYPE *)(aux + nJob*sizeJob - sizeJob), 0, worker);
}

// Worker used just for pack, adding the filter array
static void packWorker(void* a, const void* b, const void* c) {
    // a = b + c
    *(int *)a = *(int *)b + *(int *)c;
}

int pack (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter) {
    
    void* bitsum = malloc (nJob * sizeof(int));
    scan (bitsum, (void*) filter, nJob, sizeof(int), packWorker);

    size_t lastPos = 0;
    cilk_for (int i = 0; i < nJob; i++) {
        // If there is an increment
        if (*(int*) (bitsum + i * sizeof(int)) > lastPos){
            int outputPos = *(int*)(bitsum + i * sizeof(int)) - 1;
            memcpy (dest + outputPos * sizeJob, src + i * sizeJob, sizeJob);
            lastPos++;
        }
    }

    return lastPos;
}

void gather (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter, int nFilter) {
    // Tudo Independente
    cilk_for (int i=0; i < nFilter; i++) {
        memcpy (dest + i * sizeJob, src + filter[i] * sizeJob, sizeJob);
    }
}

void scatter (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter) {
    /* IMPLEMENTATION NON DETERMINISTIC */
    cilk_for(int i=0; i < nJob; i++) {
		memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);
	}
	
	
	/* IMPLEMENTATION DETERMINISTIC LEFT PRIORITY 
	
	cilk_for(int i=0; i < nJob; i++) {
		if(*(TYPE *)(dest + filter[i] * sizeJob) == 0.0){
			memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);
		}
    }
	*/
	
	/* IMPLEMENTATION DETERMINISTIC GREATER VALUE PRIORITY 
	
	cilk_for(int i=0; i < nJob; i++) {
		if(*(TYPE *)(dest + filter[i] * sizeJob) == 0.0){
			memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);
		}else if(*(TYPE *)(dest + filter[i] * sizeJob) < *(TYPE *)(src + i * sizeJob)){
			memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);	
		}
    }
	*/
	/* IMPLEMENTATION DETERMINISTIC SMALLER VALUE PRIORITY 
	cilk_for(int i=0; i < nJob; i++) {
		if(*(TYPE *)(dest + filter[i] * sizeJob) == 0.0){
			memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);
		}else if(*(TYPE *)(dest + filter[i] * sizeJob) > *(TYPE *)(src + i * sizeJob)){
			memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);	
		}
    }
	*/
	
}


void workerThread(int id, void *dest, size_t nJob, size_t sizeJob, int tasks[], void (*worker)(void *v1, const void *v2)){
    int count = 0;   
    while (count < nJob) {

        //Wait and do nothing until is equal to id
        while(tasks[count] != id);

        worker(dest + count * sizeJob, dest + count * sizeJob);  
        tasks[count] +=  1; 
        count++;         
    } 
}

void pipeline (void *dest, void *src, size_t nJob, size_t sizeJob, void (*workerList[])(void *v1, const void *v2), size_t nWorkers) {
    int *tasks = malloc(sizeof(int) * nJob);
    
    cilk_for (int i=0; i < nJob; i++) {
        tasks[i]= 0;
        memcpy (dest + i * sizeJob, src + i * sizeJob, sizeJob);
    }

    for(int id = 0; id < nWorkers ; id++){  
        
        cilk_spawn workerThread(id, dest, nJob, sizeJob, tasks, workerList[id]);
    }

}

void farmSlave(void* dest, void* src, Queue* stream, void* mutex, void (*worker)(void *v1, const void *v2), size_t sizeJob){
	size_t index = 0;
	while(index != -1){
		pthread_mutex_lock(mutex);
		index = Dequeue(stream);
		pthread_mutex_unlock(mutex);
		
		if(index != -1)
			worker(dest + index, src + index);
	}
}

void farm (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2), size_t nWorkers) {
    Queue* stream = createQueue(nJob, src, sizeJob);
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	for(int j = 0; j < nWorkers; j++){
		cilk_spawn farmSlave(dest, src, stream, &mutex, worker, sizeJob);
	} 
}