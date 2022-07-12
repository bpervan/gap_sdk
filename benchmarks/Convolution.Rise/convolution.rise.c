//Moved here
#include "gap8/gap8.h"

#define IMG_W 12
#define IMG_H 12

//Accelerator functions

#include <stdint.h>
struct cluster_params
{
    int16_t *output;
    int n1199;
    int n1198;
    int16_t *e1201;
    int16_t *e1200;
};

void cluster_core_task(void *args)
{
    pi_perf_conf(1 << PI_PERF_ACTIVE_CYCLES);
    pi_perf_reset();
    pi_perf_start();
    int time_cycles1 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);

    struct cluster_params *cl_params = (struct cluster_params *)args;
    int16_t *output = (*cl_params).output;
    int n1199 = (*cl_params).n1199;
    int n1198 = (*cl_params).n1198;
    int16_t *e1201 = (*cl_params).e1201;
    int16_t *e1200 = (*cl_params).e1200;
    {
        //collapse
        #pragma omp parallel for collapse(2)
        for (int i_1372 = 0; i_1372 < (-2 + n1198); i_1372 = 1 + i_1372)
        {
            //#pragma omp parallel for
            for (int i_1373 = 0; i_1373 < (-2 + n1199); i_1373 = 1 + i_1373)
            {
                /* reduceSeq */
                {
                    int16_t x1336;
                    x1336 = (int16_t)0;
                    for (int i_1374 = 0; i_1374 < 9; i_1374 = 1 + i_1374)
                    {
                        x1336 = x1336 + (e1200[((i_1373 + (i_1374 % 3)) + (i_1372 * n1199)) + (n1199 * (i_1374 / 3))] * e1201[i_1374]);
                    }

                    output[(i_1373 + (-2 * i_1372)) + (i_1372 * n1199)] = x1336;
                }
            }
        }
    }

    pi_perf_stop();
    int time_cycles2 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);
    printf("Total cycles: %d\n", time_cycles2 - time_cycles1);

    /*for(int i = 0; i < (IMG_H - 2) * (IMG_W - 2); ++i){
        printf("%d ", output[i]);
        if(((i + 1) % (IMG_H - 2) * (IMG_W - 2)) == 0){
            printf("\n");
        }
    }*/
}

//Host code

#include "gap8/gap8.h"
struct foo_t
{
    Kernel cluster_core_task;
};

typedef struct foo_t foo_t;

void foo_init(foo_t *self)
{
    (*self).cluster_core_task = loadKernel(cluster_core_task, 2048);
}

void foo_destroy(Context ctx, foo_t *self)
{
    destroyKernel(ctx, (*self).cluster_core_task);
}

void foo_run(Context ctx, foo_t *self, Buffer moutput, int n1198, int n1199, Buffer me1200, Buffer me1201)
{
    {
        DeviceBuffer b0 = deviceBufferSync(ctx, moutput, (-2 + n1198) * ((-2 + n1199) * sizeof(int16_t)), 0);
        int b1 = n1199;
        int b2 = n1198;
        DeviceBuffer b3 = deviceBufferSync(ctx, me1201, 3 * (3 * sizeof(int16_t)), 0);
        DeviceBuffer b4 = deviceBufferSync(ctx, me1200, n1198 * (n1199 * sizeof(int16_t)), 0);
        struct cluster_params *cl_params = (struct cluster_params *)pmsis_l2_malloc(sizeof(struct cluster_params));
        (*cl_params).output = b0;
        (*cl_params).n1199 = b1;
        (*cl_params).n1198 = b2;
        (*cl_params).e1201 = b3;
        (*cl_params).e1200 = b4;
        launchKernel(ctx, (*self).cluster_core_task, 8, cl_params);
    }
}

void foo_init_run(Context ctx, Buffer moutput, int n1198, int n1199, Buffer me1200, Buffer me1201)
{
    foo_t foo;
    foo_init(&foo);
    foo_run(ctx, &foo, moutput, n1198, n1199, me1200, me1201);
    foo_destroy(ctx, &foo);
}

short int In[] = {
    -54, -42, -84, -72, -36, -89, -56, -37, 69, -9, 79, 64,
    89, 90, -31, -6, -21, 7, -8, -18, 98, -12, -43, 74,
    45, -2, 83, 36, -22, 72, -7, 52, 23, -27, 39, 65,
    91, -61, -30, -15, -67, 96, 29, 85, -71, 97, -59, -57,
    -23, -69, -76, 38, 70, 67, -79, -47, 71, 73, -39, -97,
    6, -81, -63, 86, 15, -25, 5, 3, 99, -26, -99, 42,
    13, -95, -78, 88, -80, 84, -10, -44, -20, 92, 62, 94,
    20, 57, 26, -55, -17, -14, 56, 93, -86, -77, -40, 31,
    66, 50, 54, -49, -73, -100, -94, 100, 1, -90, 11, 30,
    55, -34, 51, -96, 81, 0, 47, -11, 78, 60, 4, 40,
    18, -46, 46, 43, -62, 37, 48, 19, -87, -88, -16, -19,
    -93, 58, 24, 59, 61, -32, 17, -41, 14, -5, 34, -35};

short int Filter3x3_X[] = {
    1, 0, 1,
    2, 0, 2,
    1, 0, 1};

short int Out[(IMG_H - 2) * (IMG_W - 2)] = {-1};

Buffer in;
Buffer filter;
Buffer out;

struct pi_device gpio_a1;
struct pi_gpio_conf gpio_conf;

#define FREQ_FC (250 * 1000000)
#define FREQ_CL (175 * 1000000)

void __main()
{
    printf("Main FC entry point (Manually written)\n");

    pi_pad_set_function(PI_PAD_12_A3_RF_PACTRL0, PI_PAD_12_A3_GPIO_A0_FUNC1);
    pi_gpio_e gpio_out_a1 = PI_GPIO_A0_PAD_12_A3;
    pi_gpio_flags_e cfg_flags = PI_GPIO_OUTPUT;
    pi_gpio_pin_configure(&gpio_a1, gpio_out_a1, cfg_flags);
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);

    Context ctx = createDefaultContext();

    in = createBuffer(ctx, IMG_H * IMG_W * sizeof(int16_t), 0);
    filter = createBuffer(ctx, 3 * 3 * sizeof(int16_t), 0);
    out = createBuffer(ctx, (IMG_H - 2) * (IMG_W - 2) * sizeof(int16_t), 0);

    in->inner = In;
    filter->inner = Filter3x3_X;
    out->inner = Out;

    printf("FC FREQ: %d\n", rt_freq_get(RT_FREQ_DOMAIN_FC));
    printf("CL FREQ: %d\n", rt_freq_get(RT_FREQ_DOMAIN_CL));

    printf("Start \n");
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 1);
    long time_usec1 = rt_time_get_us();

    foo_init_run(ctx, out, IMG_W, IMG_H, in, filter);

    /* Stop the counter and print # active cycles */

    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);
    long time_usec2 = rt_time_get_us();
    printf("Wall clock time: %ld usec\n", time_usec2 - time_usec1);
    printf("End \n");

    destroyBuffer(ctx, in);
    destroyBuffer(ctx, filter);
    destroyBuffer(ctx, out);
    destroyContext(ctx);

    printf("Over and out\n");
    pmsis_exit(0);
}

int main(int argc, char **argv)
{
    printf("\n\n\t *** Convolution (RISE) ***\n\n");
    return pmsis_kickoff((void *)__main);
}