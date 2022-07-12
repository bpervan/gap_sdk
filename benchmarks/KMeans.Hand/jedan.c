//Accelerator functions

#include <stdint.h>
struct cluster_params {
  int16_t* output;
  int16_t* e18;
  int16_t* e19;
};

void cluster_core_task(void* args){
  struct cluster_params* cl_params = (struct cluster_params*)args;
  int16_t* output = (*cl_params).output;
  int16_t* e18 = (*cl_params).e18;
  int16_t* e19 = (*cl_params).e19;
  {
    /* mapSeq */
    for (int i_194 = 0; i_194 < 4; i_194 = 1 + i_194) {
      /* mapSeq */
      for (int i_195 = 0; i_195 < 4; i_195 = 1 + i_195) {
        /* reduceSeq */
        {
          int16_t x158;
          x158 = (int16_t)0;
          for (int i_196 = 0; i_196 < 9; i_196 = 1 + i_196) {
            x158 = x158 + (e18[((i_195 + (i_196 % 3)) + (6 * i_194)) + (6 * (i_196 / 3))] * e19[i_196]);
          }
          
          output[i_195 + (4 * i_194)] = x158;
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

void foo_run(Context ctx, foo_t* self, Buffer moutput, Buffer me18, Buffer me19){
  {
    DeviceBuffer b0 = deviceBufferSync(ctx, moutput, 4 * (4 * sizeof(int16_t)), 0);
    DeviceBuffer b1 = deviceBufferSync(ctx, me18, 6 * (6 * sizeof(int16_t)), 0);
    DeviceBuffer b2 = deviceBufferSync(ctx, me19, 3 * (3 * sizeof(int16_t)), 0);
    struct cluster_params* cl_params = (struct cluster_params*)pmsis_l2_malloc(sizeof(struct cluster_params));
    (*cl_params).output = b0;
    (*cl_params).e18 = b1;
    (*cl_params).e19 = b2;
    launchKernel(ctx, (*self).cluster_core_task, 8, cl_params);
  }
  
}

void foo_init_run(Context ctx, Buffer moutput, Buffer me18, Buffer me19){
  foo_t foo;
  foo_init(&foo);
  foo_run(ctx, &foo, moutput, me18, me19);
  foo_destroy(ctx, &foo);
}

int main(int argc, char** argv){
  return pmsis_kickoff((void*)__main);
  
}