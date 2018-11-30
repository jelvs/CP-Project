#ifndef __PATTERNS_H
#define __PATTERNS_H
#define TYPE double

struct ScanNode{
  void* sum;
	void* fromLeft;
	size_t index;
  struct ScanNode* left;
	struct ScanNode* right;
};

void map (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  void (*worker)(void *v1, const void *v2) // [ v1 = op (v2) ]
);

void mapSerial (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  void (*worker)(void *v1, const void *v2) // [ v1 = op (v2) ]
);

void reduce (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  void (*worker)(void *v1, const void *v2, const void *v3) // [ v1 = op (v2, v3) ]
);

void reduceSerial (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  void (*worker)(void *v1, const void *v2, const void *v3) // [ v1 = op (v2, v3) ]
);


void scan (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  void (*worker)(void *v1, const void *v2, const void *v3) // [ v1 = op (v2, v3) ]
);

void scanSerial (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  void (*worker)(void *v1, const void *v2, const void *v3) // [ v1 = op (v2, v3) ]
);

void scanTree (
  void *dest, 
  void *src, 
  size_t nJob, 
  size_t sizeJob, 
  void (*worker)(void *v1, const void *v2, const void *v3)
);

int pack (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  const int *filter     // Filer for pack
);

int packSerial (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  const int *filter     // Filer for pack
);

void gather (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  const int *filter,    // Filter for gather
  int nFilter           // # elements in the filter
);

void gatherSerial (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  const int *filter,    // Filter for gather
  int nFilter           // # elements in the filter
);

void scatter (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  const int *filter     // Filter for scatter
);

void scatterSerial (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  const int *filter     // Filter for scatter
);

void pipeline (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  void (*workerList[])(void *v1, const void *v2), // one function for each stage of the pipeline
  size_t nWorkers       // # stages in the pipeline
);

void pipelineSerial (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  void (*workerList[])(void *v1, const void *v2), // one function for each stage of the pipeline
  size_t nWorkers       // # stages in the pipeline
);

void farm (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  void (*worker)(void *v1, const void *v2),  // [ v1 = op (22) ]
  size_t nWorkers       // # workers in the farm
);

void farmSerial (
  void *dest,           // Target array
  void *src,            // Source array
  size_t nJob,          // # elements in the source array
  size_t sizeJob,       // Size of each element in the source array
  void (*worker)(void *v1, const void *v2),  // [ v1 = op (22) ]
  size_t nWorkers       // # workers in the farm
);

#endif