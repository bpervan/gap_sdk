//Accelerator functions

#include <stdint.h>
//Moved here
#include "gap8/gap8.h"

//Added
Buffer in;
Buffer out;

#define ROWS 250
#define COLS 250

struct cluster_params {
  uint32_t* output;
  int n3;
  int n2;
  int n1;
  uint32_t* e4;
  uint32_t* e5;
};

void cluster_core_task(void* args){
  pi_perf_conf(1 << PI_PERF_ACTIVE_CYCLES);
  pi_perf_reset();
  pi_perf_start();
  int time_cycles1 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);

  struct cluster_params* cl_params = (struct cluster_params*)args;
  uint32_t* output = (*cl_params).output;
  int n3 = (*cl_params).n3;
  int n2 = (*cl_params).n2;
  int n1 = (*cl_params).n1;
  uint32_t* e4 = (*cl_params).e4;
  uint32_t* e5 = (*cl_params).e5;
  {
    //added collapse 2
    #pragma omp parallel for collapse(2)
    for (int i_119 = 0; i_119 < n1; i_119 = 1 + i_119) {
      //commented this out
      //#pragma omp parallel for
      for (int i_120 = 0; i_120 < n2; i_120 = 1 + i_120) {
        /* reduceSeq */
        {
          uint32_t x95;
          x95 = (uint32_t)0;
          for (int i_121 = 0; i_121 < n3; i_121 = 1 + i_121) {
            x95 = x95 + (e4[i_121 + (i_119 * n3)] * e5[i_120 + (i_121 * n2)]);
          }
          
          output[i_120 + (i_119 * n2)] = x95;
        }
        
      }
      
    }
    
  }

  pi_perf_stop();
  int time_cycles2 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);
  printf("Total cycles: %d\n", time_cycles2 - time_cycles1);
  
}


//Host code

//#include "gap8/gap8.h"
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

void foo_run(Context ctx, foo_t* self, Buffer moutput, int n1, int n2, int n3, Buffer me4, Buffer me5){
  {
    DeviceBuffer b0 = deviceBufferSync(ctx, moutput, n1 * (n2 * sizeof(uint32_t)), 0);
    int b1 = n3;
    int b2 = n2;
    int b3 = n1;
    DeviceBuffer b4 = deviceBufferSync(ctx, me4, n1 * (n3 * sizeof(uint32_t)), 0);
    DeviceBuffer b5 = deviceBufferSync(ctx, me5, n3 * (n2 * sizeof(uint32_t)), 0);
    struct cluster_params* cl_params = (struct cluster_params*)pmsis_l2_malloc(sizeof(struct cluster_params));
    (*cl_params).output = b0;
    (*cl_params).n3 = b1;
    (*cl_params).n2 = b2;
    (*cl_params).n1 = b3;
    (*cl_params).e4 = b4;
    (*cl_params).e5 = b5;
    launchKernel(ctx, (*self).cluster_core_task, 8, cl_params);
  }
  
}

void foo_init_run(Context ctx, Buffer moutput, int n1, int n2, int n3, Buffer me4, Buffer me5){
  foo_t foo;
  foo_init(&foo);
  foo_run(ctx, &foo, moutput, n1, n2, n3, me4, me5);
  foo_destroy(ctx, &foo);
}

struct pi_device gpio_a1;
struct pi_gpio_conf gpio_conf;

#define FREQ_FC (250*1000000)
#define FREQ_CL (175*1000000)

void __main(){
  printf("Main FC entry point (Manually written)\n");

  pi_pad_set_function(PI_PAD_12_A3_RF_PACTRL0, PI_PAD_12_A3_GPIO_A0_FUNC1);
  pi_gpio_e gpio_out_a1 = PI_GPIO_A0_PAD_12_A3;
  pi_gpio_flags_e cfg_flags = PI_GPIO_OUTPUT;
  pi_gpio_pin_configure(&gpio_a1, gpio_out_a1, cfg_flags);
  pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);

  Context ctx = createDefaultContext();
  in = createBuffer(ctx, ROWS * COLS * sizeof(uint32_t), 0);
  out = createBuffer(ctx, ROWS * COLS * sizeof(uint32_t), 0);

  printf("FC FREQ: %d\n", rt_freq_get(RT_FREQ_DOMAIN_FC));
  printf("CL FREQ: %d\n", rt_freq_get(RT_FREQ_DOMAIN_CL));

  uint32_t* intmp = (uint32_t*)in->inner;
  for(int i = 0; i < ROWS; ++i){
    for(int j = 0; j < COLS; ++j){
      intmp[i * COLS + j] = i + j;
    }
  }

  printf("Start \n");
  /* Benchmarking. Count active cycles */
  pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 1);
  long time_usec1 = rt_time_get_us();

  foo_init_run(ctx, out, ROWS, COLS, ROWS, in, in);

  /* Stop the counter and print # active cycles */

  pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);
  long time_usec2 = rt_time_get_us();
  printf("Wall clock time: %ld usec\n", time_usec2 - time_usec1);
  printf("End \n");

  destroyBuffer(ctx, in);
  destroyBuffer(ctx, out);
  destroyContext(ctx);

  printf("Over and out\n");
  pmsis_exit(0);
}

int main(int argc, char** argv){
  printf("\n\n\t *** MatMul - RISE (OMP) ***\n\n");
  return pmsis_kickoff((void*)__main);
  
}