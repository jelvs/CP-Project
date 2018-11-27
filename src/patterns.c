#include <stdio.h> // just for debug
#include <string.h>
#include <assert.h>

#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>

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

void upsweep (void* src, size_t lo, size_t hi, struct ScanNode* node, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    if (lo + 1 == hi){
        node->sum = *(TYPE *)(src + lo * sizeJob);
		node->index = lo;
    } else {
        size_t mid = (lo + hi) / 2;

        node->left = malloc(sizeof(struct ScanNode));
        node->right = malloc(sizeof(struct ScanNode));
		node->index = -1;
        cilk_spawn upsweep (src, lo, mid, node->left, sizeJob, worker);     
		upsweep (src, mid, hi, node->right, sizeJob, worker);
        
        cilk_sync;
		worker(&(node->sum), &(node->left->sum), &(node->right->sum));
    }
}

void downsweep (void* src, void* dest, size_t sizeJob, struct ScanNode* node, TYPE fromLeft, void (*worker)(void *v1, const void *v2, const void *v3)) {
	
	if(node->index != -1){
			worker(dest + node->index * sizeJob, src + node->index * sizeJob, &fromLeft);
	}else{
	
		cilk_spawn downsweep(src, dest, sizeJob, node->left, fromLeft, worker);
		downsweep(src, dest, sizeJob, node->right, fromLeft + node->left->sum, worker);
		
		cilk_sync;
	}
}

void scan (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    /* To be implemented */
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);

    struct ScanNode* parent = malloc(sizeof(struct ScanNode));
    upsweep (src, 0, nJob, parent, sizeJob, worker);
	downsweep (src, dest, sizeJob, parent, 0, worker);
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
    cilk_for (int i=0; i < nJob; i++) {
        memcpy (dest + i * sizeJob, src + i * sizeJob, sizeJob);
        for (int j = 0;  j < nWorkers;  j++)
            workerList[j](dest + i * sizeJob, dest + i * sizeJob);
    }
}

void farm (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2), size_t nWorkers) {
    /* To be implemented */
    map (dest, src, nJob, sizeJob, worker);
}
