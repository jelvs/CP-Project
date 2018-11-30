#include <stdio.h> // just for debug
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <cilk/cilk.h>
#include <unistd.h>
#include <pthread.h>
#include "queue.c"
#include "patterns.h"

void mapSerial (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2)) {
     /* To be implemented */
     assert (dest != NULL);
     assert (src != NULL);
     assert (worker != NULL);
     for (int i=0; i < nJob; i++) {
         worker(dest + i * sizeJob, src + i * sizeJob);
     }
 }

void map (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2)) {
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);
    
    int i = 0;
    cilk_for (i=0; i < nJob; i++) {
        worker(dest + i * sizeJob, src + i * sizeJob);
    }
}

void reduceSerial (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
     assert (dest != NULL);
     assert (src != NULL);
     assert (worker != NULL);
     if (nJob > 1) {
         memcpy (dest, src, sizeJob);
         for (int i = 1;  i < nJob;  i++)
             worker(dest, dest, src + i * sizeJob);
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
        memcpy (dest, src, sizeJob);
     }
}

void upsweepTree (void* src, size_t lo, size_t hi, struct ScanNode* node, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    if (lo + 1 == hi){
        node->sum = malloc (sizeJob);
        node->fromLeft = malloc (sizeJob);
        
        memcpy (node->sum, src + lo * sizeJob, sizeJob);
        node->index = lo;
    } else {
        size_t mid = (lo + hi) / 2;

        node->left = malloc(sizeof(struct ScanNode));
        node->right = malloc(sizeof(struct ScanNode));
        node->sum = malloc (sizeJob);
        node->fromLeft = malloc (sizeJob);
        node->index = -1;

        cilk_spawn upsweepTree (src, lo, mid, node->left, sizeJob, worker);
        upsweepTree (src, mid, hi, node->right, sizeJob, worker);
        
        cilk_sync;
        worker(node->sum, node->left->sum, node->right->sum);
    }
}

void downsweepTree (void* src, void* dest, size_t sizeJob, struct ScanNode* node, void* fromLeft, void (*worker)(void *v1, const void *v2, const void *v3)) {
    
    if(node->index != -1){
        worker(dest + node->index * sizeJob, src + node->index * sizeJob, fromLeft);
    }else{
    
        cilk_spawn downsweepTree(src, dest, sizeJob, node->left, fromLeft, worker);
    
        void* fromLeftSum = malloc (sizeJob);
        worker(fromLeftSum, node->left->sum, fromLeft);

        downsweepTree(src, dest, sizeJob, node->right, fromLeftSum, worker);
        
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
    downsweepTree (src, dest, sizeJob, parent, NULL, worker);
}

 
void upsweep( void* src, void* aux, size_t lo, size_t hi,  size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    if( lo + 1 == hi ) {
        memcpy (aux + lo * sizeJob, src + lo * sizeJob, sizeJob);
    } else {
        size_t mid = (hi + lo) / 2;

		cilk_spawn upsweep(src, aux, lo, mid, sizeJob, worker);
        upsweep(src, aux, mid, hi, sizeJob, worker);
        
		cilk_sync;
        
		worker(aux + (hi*sizeJob - sizeJob), aux + (mid*sizeJob - sizeJob), aux + (hi*sizeJob - sizeJob));
    }
}

void downsweep (void* src, void* dest, void* aux, size_t sizeJob, size_t lo, size_t hi, void* sum, void* fromLeft, void (*worker)(void *v1, const void *v2, const void *v3)) {
	
	if(lo + 1 == hi){
		worker(dest + lo * sizeJob, src + lo * sizeJob, fromLeft);
	}else{
		size_t mid = (hi + lo) / 2;
    
		cilk_spawn downsweep(src, dest,aux, sizeJob, lo, mid, aux + mid * sizeJob - sizeJob, fromLeft, worker);

        void* sum = malloc (sizeJob);
        worker (sum, aux + (hi*sizeJob -sizeJob), aux + (mid*sizeJob -sizeJob));

        void* fromLeftSum = malloc (sizeJob);
        worker(fromLeftSum, aux + mid*sizeJob -sizeJob, fromLeft);

		downsweep(src, dest, aux, sizeJob, mid, hi, sum, fromLeftSum, worker);
		
		//implicit cilk_sync;		
	}
}

void scan (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    /* To be implemented */
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);

	void* aux = malloc(nJob*sizeJob);
	
    upsweep(src, aux, 0, nJob, sizeJob, worker);
	downsweep (src, dest, aux, sizeJob, 0, nJob, aux + nJob*sizeJob - sizeJob, NULL, worker);
}

void scanSerial (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
     assert (dest != NULL);
     assert (src != NULL);
     assert (worker != NULL);
     if (nJob > 1) {
         memcpy (dest, src, sizeJob);
         for (int i = 1;  i < nJob;  i++)
             worker(dest + i * sizeJob, src + i * sizeJob, dest + (i-1) * sizeJob);
     }
 }

// Worker used just for pack, adding the filter array
static void packWorker(void* a, const void* b, const void* c) {
    if (c == NULL) {
        *(int *)a = *(int *)b;
    } else {
        *(int *)a = *(int *)b + *(int *)c;
    }
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

int packSerial (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter) {
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

void gatherSerial (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter, int nFilter) {
     /* To be implemented */
     for (int i=0; i < nFilter; i++) {
         memcpy (dest + i * sizeJob, src + filter[i] * sizeJob, sizeJob);
     }
 }

 void scatterSerial (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter) {
     /* To be implemented */
     for (int i=0; i < nJob; i++) {
         memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);
     }
 }

void scatter (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter) {
    /* IMPLEMENTATION NON DETERMINISTIC */
    cilk_for(int i=0; i < nJob; i++) {
		memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);
	}
	
	
	/* IMPLEMENTATION DETERMINISTIC LEFT PRIORITY 
	
	cilk_for(int i=0; i < nJob; i++) {
        int comp = memcmp(dest + filter[i] * sizeJob, 0.0, sizeJob);
		if(comp == 0){
			memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);
		}
    }*/
	
	
	/*IMPLEMENTATION DETERMINISTIC GREATER VALUE PRIORITY 
	
	cilk_for(int i=0; i < nJob; i++) {
        double zero = 0.0;
        void* x = &zero;
        int comp = memcmp(dest + filter[i] * sizeJob, x, sizeJob);
		if(comp == 0){
			memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);
		}
		else{
			//printf("OLD VALUE %lf -- NEW VALUE %lf\n", *(double*)(dest + filter[i] * sizeJob), *(double*)(src + i * sizeJob));
			int comp2 = memcmp(dest + (filter[i] * sizeJob), src + (i * sizeJob), sizeJob);
			//printf("COMP2 %d\n", comp2);
			if(comp2 > 0){
				//printf("Write %lf on %lf with comp2 = %d\n", *(double*)(src + i * sizeJob), *(double*)(dest + filter[i] * sizeJob), comp2);
				memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);
			}
		}
           
    }*/
	
	/* IMPLEMENTATION DETERMINISTIC SMALLER VALUE PRIORITY 
	cilk_for(int i=0; i < nJob; i++) {
        int comp = memcmp(dest + filter[i] * sizeJob, 0.0, sizeJob);
        int comp2 = memcmp(dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);
		if(comp == 0){
			memcpy (dest + filter[i] * sizeJob, src + i * sizeJob, sizeJob);
		}else if(comp2 > 0){
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

void pipelineSerial (void *dest, void *src, size_t nJob, size_t sizeJob, void (*workerList[])(void *v1, const void *v2), size_t nWorkers) {
     /* To be implemented */
     for (int i=0; i < nJob; i++) {
         memcpy (dest + i * sizeJob, src + i * sizeJob, sizeJob);
         for (int j = 0;  j < nWorkers;  j++)
             workerList[j](dest + i * sizeJob, dest + i * sizeJob);
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

 void farmSerial (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2), size_t nWorkers) {
     /* To be implemented */
     mapSerial (dest, src, nJob, sizeJob, worker);
 }