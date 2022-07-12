//Accelerator functions

#include <stdint.h>
struct cluster_params {
  int16_t* output;
  int16_t* e25;
  int16_t* e24;
};

void cluster_core_task(void* args){
  struct cluster_params* cl_params = (struct cluster_params*)args;
  int16_t* output = (*cl_params).output;
  int16_t* e25 = (*cl_params).e25;
  int16_t* e24 = (*cl_params).e24;
  {
    /* mapSeq */
    for (int i_363 = 0; i_363 < 4; i_363 = 1 + i_363) {
      /* mapSeq */
      for (int i_364 = 0; i_364 < 4; i_364 = 1 + i_364) {
        /* reduceSeq */
        {
          int16_t x327;
          x327 = (int16_t)0;
          for (int i_365 = 0; i_365 < 9; i_365 = 1 + i_365) {
            x327 = x327 + (e24[((i_364 + (i_365 % 3)) + (6 * i_363)) + (6 * (i_365 / 3))] * e25[i_365]);
          }
          
          output[i_364 + (4 * i_363)] = x327;
        }
        
      }
      
    }
    
  }
  
}


//Host code

#include "gap8/gap8.h"
struct foo_t {
  Kernel cluster_core_task;
};

typedef struct foo_t foo_t;

void foo_init(foo_t* self){
  (*self).cluster_core_task = loadKernel(cluster_core_task, 2048);
}

void foo_destroy(Context ctx, foo_t* self){
  destroyKernel(ctx, (*self).cluster_core_task);
}

void foo_run(Context ctx, foo_t* self, Buffer moutput, Buffer me24, Buffer me25){
  {
    DeviceBuffer b0 = deviceBufferSync(ctx, moutput, 4 * (4 * sizeof(int16_t)), 0);
    DeviceBuffer b1 = deviceBufferSync(ctx, me25, 3 * (3 * sizeof(int16_t)), 0);
    DeviceBuffer b2 = deviceBufferSync(ctx, me24, 6 * (6 * sizeof(int16_t)), 0);
    struct cluster_params* cl_params = (struct cluster_params*)pmsis_l2_malloc(sizeof(struct cluster_params));
    (*cl_params).output = b0;
    (*cl_params).e25 = b1;
    (*cl_params).e24 = b2;
    launchKernel(ctx, (*self).cluster_core_task, 8, cl_params);
  }
  
}

void foo_init_run(Context ctx, Buffer moutput, Buffer me24, Buffer me25){
  foo_t foo;
  foo_init(&foo);
  foo_run(ctx, &foo, moutput, me24, me25);
  foo_destroy(ctx, &foo);
}

int main(int argc, char** argv){
  return pmsis_kickoff((void*)__main);
  
}