#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "patterns.h"
#include <stdio.h>

#include "patterns.h"
#include "debug.h"
#include "unit.h"


#define TYPE double
#define FMT "%lf"


//=======================================================
// Workers
//=======================================================

/*
static void workerMax(void* a, const void* b, const void* c) {
    // a = max (b, c)
    *(TYPE *)a = *(TYPE *)b;
    if (*(TYPE *)c > *(TYPE *)a)
        *(TYPE *)a = *(TYPE *)c;
}
*/

/*
static void workerMin(void* a, const void* b, const void* c) {
    // a = min (b, c)
    *(TYPE *)a = *(TYPE *)b;
    if (*(TYPE *)c < *(TYPE *)a)
        *(TYPE *)a = *(TYPE *)c;
}
*/

static void workerAdd(void* a, const void* b, const void* c) {
    // a = b + c
    if (c == NULL){
        if (b == NULL){
            *(TYPE *)a = (TYPE) 0;
        } else {
            *(TYPE *)a = *(TYPE *)b;
        }
    }else{
        if(b == NULL){
            *(TYPE *)a = *(TYPE *)c;
        }else {
            *(TYPE *)a = *(TYPE *)b + *(TYPE *)c;
        }
    }
}

/*
static void workerSubtract(void* a, const void* b, const void* c) {
    // a = n - c
    *(TYPE *)a = *(TYPE *)b - *(TYPE *)c;
}
*/

/*
static void workerMultiply(void* a, const void* b, const void* c) {
    // a = b * c
    *(TYPE *)a = *(TYPE *)b + *(TYPE *)c;
}
*/

static void workerAddOne(void* a, const void* b) {
    // a = b + 1
    *(TYPE *)a = *(TYPE *)b + 1;
}

static void workerMultTwo(void* a, const void* b) {
    // a = b * 2
    *(TYPE *)a = *(TYPE *)b * 2;
}

static void workerDivTwo(void* a, const void* b) {
    // a = b / 2
    *(TYPE *)a = *(TYPE *)b / 2;
}


//=======================================================
// Unit testing funtions
//=======================================================

void testMap (void *src, size_t n, size_t size) {
    TYPE *dest = malloc (n * size);
    map (dest, src, n, size, workerAddOne);
    printDouble (dest, n, __FUNCTION__);
    free (dest);
}

void testReduce (void *src, size_t n, size_t size) {
    TYPE *dest = malloc (size);
    reduce (dest, src, n, size, workerAdd);
    printDouble (dest, 1, __FUNCTION__);
    free (dest);
}

void testScan (void *src, size_t n, size_t size) {
    TYPE *dest = malloc (n * size);
    scan (dest, src, n, size, workerAdd);
    printDouble (dest, n, __FUNCTION__);
    free (dest);
}

void testPack (void *src, size_t n, size_t size) {
    int nFilter = 3;
    TYPE *dest = malloc (nFilter * size);
    int *filter = calloc(n,sizeof(*filter));
    for (int i = 0;  i < n;  i++)
        filter[i] = (i == 0 || i == n/2 || i == n-1);
    int newN = pack (dest, src, n, size, filter);    
    printInt (filter, n, "filter");    
    printDouble (dest, newN, __FUNCTION__);
    free(filter);
    free (dest);
}

void testGather (void *src, size_t n, size_t size) {
    int nFilter = 3;
    //int nFilter = (n/10000000);
    TYPE *dest = malloc (nFilter * size);
    int filter[nFilter];
    for (int i = 0;  i < nFilter;  i++)
        filter[i] = rand() % n;
    printInt (filter, nFilter, "filter");    
    gather (dest, src, n, size, filter, nFilter);    
    printDouble (dest, nFilter, __FUNCTION__);
    free (dest);
}

void testScatter (void *src, size_t n, size_t size) {
    int nDest = n;
    TYPE *dest = malloc (nDest * size);
    memset (dest, 0, nDest * size);
    int *filter = calloc(n,sizeof(*filter));
    for (int i = 0;  i < n;  i++)
        filter[i] = rand() % nDest;
    printInt (filter, n, "filter");    
    scatter (dest, src, n, size, filter);    
    printDouble (dest, nDest, __FUNCTION__);
    free(filter);
    free (dest);
}

void testPipeline (void *src, size_t n, size_t size) {
    void (*pipelineFunction[])(void*, const void*) = {
        workerMultTwo,
        workerAddOne,
        workerDivTwo
    };
    int nPipelineFunction = sizeof (pipelineFunction)/sizeof(pipelineFunction[0]);
    TYPE *dest = malloc (n * size);
    pipeline (dest, src, n, size, pipelineFunction, nPipelineFunction);
    printDouble (dest, n, __FUNCTION__);    
    free (dest);
}

void testFarm (void *src, size_t n, size_t size) {
    TYPE *dest = malloc (n * size);
    farm (dest, src, n, size, workerAddOne, 10);
    printDouble (dest, n, __FUNCTION__);
    free (dest);
}

//=======================================================
// List of unit test functions in serial mode
//=======================================================

void testMapSerial (void *src, size_t n, size_t size) {
    TYPE *dest = malloc (n * size);
    mapSerial (dest, src, n, size, workerAddOne);
    printDouble (dest, n, __FUNCTION__);
    free (dest);
}

void testReduceSerial (void *src, size_t n, size_t size) {
    TYPE *dest = malloc (size);
    reduceSerial (dest, src, n, size, workerAdd);
    printDouble (dest, 1, __FUNCTION__);
    free (dest);
}

void testScanSerial (void *src, size_t n, size_t size) {
    TYPE *dest = malloc (n * size);
    scanSerial (dest, src, n, size, workerAdd);
    printDouble (dest, n, __FUNCTION__);
    free (dest);
}

void testPackSerial (void *src, size_t n, size_t size) {
    int nFilter = 3;
    TYPE *dest = malloc (nFilter * size);
    int *filter = calloc(n,sizeof(*filter));
    for (int i = 0;  i < n;  i++)
        filter[i] = (i == 0 || i == n/2 || i == n-1);
    int newN = packSerial (dest, src, n, size, filter);    
    printInt (filter, n, "filter");    
    printDouble (dest, newN, __FUNCTION__);
    free(filter);
    free (dest);
}

void testGatherSerial (void *src, size_t n, size_t size) {
    int nFilter = 3;
    //int nFilter = (n/10000000);
    TYPE *dest = malloc (nFilter * size);
    int filter[nFilter];
    for (int i = 0;  i < nFilter;  i++)
        filter[i] = rand() % n;
    printInt (filter, nFilter, "filter");    
    gatherSerial (dest, src, n, size, filter, nFilter);    
    printDouble (dest, nFilter, __FUNCTION__);
    free (dest);
}

void testScatterSerial (void *src, size_t n, size_t size) {
    int nDest = n;
    TYPE *dest = malloc (nDest * size);
    memset (dest, 0, nDest * size);
    int *filter = calloc(n,sizeof(*filter));
    for (int i = 0;  i < n;  i++)
        filter[i] = rand() % nDest;
    printInt (filter, n, "filter");    
    scatterSerial (dest, src, n, size, filter);    
    printDouble (dest, nDest, __FUNCTION__);
    free(filter);
    free (dest);
}

void testPipelineSerial (void *src, size_t n, size_t size) {
    void (*pipelineFunction[])(void*, const void*) = {
        workerMultTwo,
        workerAddOne,
        workerDivTwo
    };
    int nPipelineFunction = sizeof (pipelineFunction)/sizeof(pipelineFunction[0]);
    TYPE *dest = malloc (n * size);
    pipelineSerial (dest, src, n, size, pipelineFunction, nPipelineFunction);
    printDouble (dest, n, __FUNCTION__);    
    free (dest);
}

void testFarmSerial (void *src, size_t n, size_t size) {
    TYPE *dest = malloc (n * size);
    farmSerial (dest, src, n, size, workerAddOne, 10);
    printDouble (dest, n, __FUNCTION__);
    free (dest);
}


//=======================================================
// List of unit test functions
//=======================================================


typedef void (*TESTFUNCTION)(void *, size_t, size_t);

TESTFUNCTION testAllFunction[] = {
    testMap,
    testMapSerial,
    testReduce,
    testReduceSerial,
    testScan,
    testScanSerial,
    testPack,
    testPackSerial,
    testGather,
    testGatherSerial,
    testScatter,
    testScatterSerial,
    testPipeline,
    testPipelineSerial,
    testFarm,
    testFarmSerial
};

char *testAllNames[] = {
    "testMap",
    "testMapSerial",
    "testReduce",
    "testReduceSerial",
    "testScan",
    "testScanSerial",
    "testPack",
    "testPackSerial",
    "testGather",
    "testGatherSerial",
    "testScatter",
    "testScatterSerial",
    "testPipeline",
    "testPipelineSerial",
    "testFarm",
    "testFarmSerial"
};

TESTFUNCTION testParallelFunction[] = {
    testMap,
    testReduce,
    testScan,
    testPack,
    testGather,
    testScatter,
    testPipeline,
    testFarm
};

char *testParallelNames[] = {
    "testMap",
    "testReduce",
    "testScan",
    "testPack",
    "testGather",
    "testScatter",
    "testPipeline",
    "testFarm"
};


int nTestAllFunction = sizeof (testAllFunction)/sizeof(testAllFunction[0]);

int nTestParallelFunction = sizeof (testParallelFunction)/sizeof(testParallelFunction[0]);



