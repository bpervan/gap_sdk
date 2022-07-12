//Accelerator functions

#include <stdint.h>
//moved here
#include "gap8/gap8.h"
uint32_t gap_sqrt(uint32_t a_nInput)
 {
   uint32_t op  = a_nInput;
   uint32_t res = 0;

   uint32_t one = 1uL << 30;
   while (one > op){
     one >>= 2;
   }
   while (one != 0) {
     if (op >= res + one){
       op = op - (res + one);
       res = res +  2 * one;
     }
     res >>= 1;
     one >>= 2;
   }
   return res;
 }

struct cluster_params {
  uint8_t* output;
  int n2;
  int n1;
  int* e5;
  int* e4;
  uint8_t* e3;
};

void cluster_core_task(void* args){
  /* Benchmarking. Count active cycles */
  pi_perf_conf(1 << PI_PERF_ACTIVE_CYCLES);
  pi_perf_reset();
  pi_perf_start();
  int time1 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);

  struct cluster_params* cl_params = (struct cluster_params*)args;
  uint8_t* output = (*cl_params).output;
  int n2 = (*cl_params).n2;
  int n1 = (*cl_params).n1;
  int* e5 = (*cl_params).e5;
  int* e4 = (*cl_params).e4;
  uint8_t* e3 = (*cl_params).e3;
  {
    //collapse(2)
    #pragma omp parallel for collapse(2)
    for (int i_793 = 0; i_793 < n1; i_793 = 1 + i_793) {
      //#pragma omp parallel for
      for (int i_794 = 0; i_794 < n2; i_794 = 1 + i_794) {
        /* reduceSeq */
        {
          uint32_t x742;
          x742 = (uint32_t)0;
          /* unrolling loop of 9 */
          x742 = x742 + (((uint32_t)((i_794 < 1) ? ((uint8_t)0) : ((i_793 < 1) ? ((uint8_t)0) : e3[((-1 + i_794) + (-1 * n2)) + (i_793 * n2)]))) * ((uint32_t)e4[0]));
          x742 = x742 + (((uint32_t)((i_793 < 1) ? ((uint8_t)0) : e3[(i_794 + (-1 * n2)) + (i_793 * n2)])) * ((uint32_t)e4[1]));
          x742 = x742 + (((uint32_t)(((2 + i_794) < (1 + n2)) ? ((i_793 < 1) ? ((uint8_t)0) : e3[((1 + i_794) + (-1 * n2)) + (i_793 * n2)]) : ((uint8_t)0))) * ((uint32_t)e4[2]));
          x742 = x742 + (((uint32_t)((i_794 < 1) ? ((uint8_t)0) : e3[(-1 + i_794) + (i_793 * n2)])) * ((uint32_t)e4[3]));
          x742 = x742 + (((uint32_t)e3[i_794 + (i_793 * n2)]) * ((uint32_t)e4[4]));
          x742 = x742 + (((uint32_t)(((2 + i_794) < (1 + n2)) ? e3[(1 + i_794) + (i_793 * n2)] : ((uint8_t)0))) * ((uint32_t)e4[5]));
          x742 = x742 + (((uint32_t)((i_794 < 1) ? ((uint8_t)0) : (((2 + i_793) < (1 + n1)) ? e3[((-1 + i_794) + n2) + (i_793 * n2)] : ((uint8_t)0)))) * ((uint32_t)e4[6]));
          x742 = x742 + (((uint32_t)(((2 + i_793) < (1 + n1)) ? e3[(i_794 + n2) + (i_793 * n2)] : ((uint8_t)0))) * ((uint32_t)e4[7]));
          x742 = x742 + (((uint32_t)(((2 + i_794) < (1 + n2)) ? (((2 + i_793) < (1 + n1)) ? e3[((1 + i_794) + n2) + (i_793 * n2)] : ((uint8_t)0)) : ((uint8_t)0))) * ((uint32_t)e4[8]));
          /* reduceSeq */
          {
            uint32_t x716;
            x716 = (uint32_t)0;
            /* unrolling loop of 9 */
            x716 = x716 + (((uint32_t)((i_794 < 1) ? ((uint8_t)0) : ((i_793 < 1) ? ((uint8_t)0) : e3[((-1 + i_794) + (-1 * n2)) + (i_793 * n2)]))) * ((uint32_t)e5[0]));
            x716 = x716 + (((uint32_t)((i_793 < 1) ? ((uint8_t)0) : e3[(i_794 + (-1 * n2)) + (i_793 * n2)])) * ((uint32_t)e5[1]));
            x716 = x716 + (((uint32_t)(((2 + i_794) < (1 + n2)) ? ((i_793 < 1) ? ((uint8_t)0) : e3[((1 + i_794) + (-1 * n2)) + (i_793 * n2)]) : ((uint8_t)0))) * ((uint32_t)e5[2]));
            x716 = x716 + (((uint32_t)((i_794 < 1) ? ((uint8_t)0) : e3[(-1 + i_794) + (i_793 * n2)])) * ((uint32_t)e5[3]));
            x716 = x716 + (((uint32_t)e3[i_794 + (i_793 * n2)]) * ((uint32_t)e5[4]));
            x716 = x716 + (((uint32_t)(((2 + i_794) < (1 + n2)) ? e3[(1 + i_794) + (i_793 * n2)] : ((uint8_t)0))) * ((uint32_t)e5[5]));
            x716 = x716 + (((uint32_t)((i_794 < 1) ? ((uint8_t)0) : (((2 + i_793) < (1 + n1)) ? e3[((-1 + i_794) + n2) + (i_793 * n2)] : ((uint8_t)0)))) * ((uint32_t)e5[6]));
            x716 = x716 + (((uint32_t)(((2 + i_793) < (1 + n1)) ? e3[(i_794 + n2) + (i_793 * n2)] : ((uint8_t)0))) * ((uint32_t)e5[7]));
            x716 = x716 + (((uint32_t)(((2 + i_794) < (1 + n2)) ? (((2 + i_793) < (1 + n1)) ? e3[((1 + i_794) + n2) + (i_793 * n2)] : ((uint8_t)0)) : ((uint8_t)0))) * ((uint32_t)e5[8]));
            output[i_794 + (i_793 * n2)] = (uint8_t)gap_sqrt((x742 * x742) + (x716 * x716));
          }
          
        }
        
      }
      
    }
    
  }
  /* Stop the counter and print # active cycles */
  pi_perf_stop();
  int time2 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);
  printf("Total cycles: %d\n", time2 - time1);
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

void foo_run(Context ctx, foo_t* self, Buffer moutput, int n1, int n2, Buffer me3, Buffer me4, Buffer me5){
  {
    DeviceBuffer b0 = deviceBufferSync(ctx, moutput, n1 * (n2 * sizeof(uint8_t)), 0);
    int b1 = n2;
    int b2 = n1;
    DeviceBuffer b3 = deviceBufferSync(ctx, me5, 3 * (3 * sizeof(int)), 0);
    DeviceBuffer b4 = deviceBufferSync(ctx, me4, 3 * (3 * sizeof(int)), 0);
    DeviceBuffer b5 = deviceBufferSync(ctx, me3, n1 * (n2 * sizeof(uint8_t)), 0);
    struct cluster_params* cl_params = (struct cluster_params*)pmsis_l2_malloc(sizeof(struct cluster_params));
    (*cl_params).output = b0;
    (*cl_params).n2 = b1;
    (*cl_params).n1 = b2;
    (*cl_params).e5 = b3;
    (*cl_params).e4 = b4;
    (*cl_params).e3 = b5;
    launchKernel(ctx, (*self).cluster_core_task, 8, cl_params);
  }
  
}

void foo_init_run(Context ctx, Buffer moutput, int n1, int n2, Buffer me3, Buffer me4, Buffer me5){
  foo_t foo;
  foo_init(&foo);
  foo_run(ctx, &foo, moutput, n1, n2, me3, me4, me5);
  foo_destroy(ctx, &foo);
}


Buffer ImageInBuffer;
Buffer ImageOutBuffer;

Buffer gxBuffer;
Buffer gyBuffer;

int G_X[] = {
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1};

int G_Y[] = {
    -1, -2, -1,
    0, 0, 0,
    1, 2, 1};

#define IMG_LINES 240
#define IMG_COLS 320

struct pi_device gpio_a1;
struct pi_gpio_conf gpio_conf;

#define FREQ_FC (250*1000000)
#define FREQ_CL (175*1000000)

void __main(int argc, char **argv)
{
    printf("Main FC entry point\n");

    pi_pad_set_function(PI_PAD_12_A3_RF_PACTRL0, PI_PAD_12_A3_GPIO_A0_FUNC1);
    pi_gpio_e gpio_out_a1 = PI_GPIO_A0_PAD_12_A3;
    pi_gpio_flags_e cfg_flags = PI_GPIO_OUTPUT;
    pi_gpio_pin_configure(&gpio_a1, gpio_out_a1, cfg_flags);
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);

    char *in_image_file_name = "valve.pgm";
    char path_to_in_image[64];
    sprintf(path_to_in_image, "../../../%s", in_image_file_name);

    Context ctx = createDefaultContext();

    ImageInBuffer = createBuffer(ctx, IMG_COLS * IMG_LINES * sizeof(unsigned char), 0);
    ImageOutBuffer = createBuffer(ctx, IMG_COLS * IMG_LINES * sizeof(unsigned char), 0);
    gxBuffer = createBuffer(ctx, 9, HOST_READ);
    gyBuffer = createBuffer(ctx, 9, HOST_READ);

    gxBuffer->inner = G_X;
    gyBuffer->inner = G_Y;

    if (ReadImageFromFile(path_to_in_image, IMG_COLS, IMG_LINES, 1, ImageInBuffer->inner, IMG_COLS * IMG_LINES * sizeof(unsigned char), IMGIO_OUTPUT_CHAR, 0))
    {
        printf("Failed to load image %s\n", path_to_in_image);
        pmsis_exit(-1);
    }

    printf("FC FREQ: %d\n", rt_freq_get(RT_FREQ_DOMAIN_FC));
    printf("CL FREQ: %d\n", rt_freq_get(RT_FREQ_DOMAIN_CL));

    printf("Start \n");
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 1);
    long time_usec1 = rt_time_get_us();

    foo_init_run(ctx, ImageOutBuffer, IMG_LINES, IMG_COLS, ImageInBuffer, gxBuffer, gyBuffer);

    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);
    long time_usec2 = rt_time_get_us();
    printf("Wall clock time: %ld usec\n", time_usec2 - time_usec1);
    printf("End \n");

    /* Write image to file */
    char *out_image_file_name = "img_out.ppm";
    char path_to_out_image[50];
    sprintf(path_to_out_image, "../../../%s", out_image_file_name);
    printf("Path to output image: %s\n", path_to_out_image);
    WriteImageToFile(path_to_out_image, IMG_COLS, IMG_LINES, 1, ImageOutBuffer->inner, GRAY_SCALE_IO);

    destroyBuffer(ctx, ImageInBuffer);
    destroyBuffer(ctx, ImageOutBuffer);
    destroyBuffer(ctx, gxBuffer);
    destroyBuffer(ctx, gyBuffer);
    destroyContext(ctx);

    pmsis_exit(0);
}

int main(int argc, char** argv){
  printf("\n\n\t *** Sobel Filter (RISE) ***\n\n");
  return pmsis_kickoff((void*)__main);
}
