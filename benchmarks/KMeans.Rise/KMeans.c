//Accelerator functions
//moved here
#include "means.h"
#include "gap8/gap8.h"
#include <stdint.h>
struct Record_uint32_t_uint32_t
{
    uint32_t _fst;
    uint32_t _snd;
};

struct Record_uint32_t__uint32_t_uint32_t_
{
    uint32_t _fst;
    struct Record_uint32_t_uint32_t _snd;
};

struct Record_uint32_t__uint32_t_uint32_t_ test(uint32_t dist, struct Record_uint32_t__uint32_t_uint32_t_ tuple)
{
    uint32_t min_dist = tuple._fst;
    uint32_t i = tuple._snd._fst;
    uint32_t index = tuple._snd._snd;
    if (dist < min_dist)
    {
        return (struct Record_uint32_t__uint32_t_uint32_t_){dist, {i + 1, i}};
    }
    else
    {
        return (struct Record_uint32_t__uint32_t_uint32_t_){min_dist, {i + 1, index}};
    }
}
struct cluster_params
{
    uint32_t *output;
    int n5;
    int n4;
    int n6;
    uint32_t *e7;
    uint32_t *e8;
};

void cluster_core_task(void *args)
{
    pi_perf_conf(1 << PI_PERF_ACTIVE_CYCLES);
    pi_perf_reset();
    pi_perf_start();
    int time1 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);

    struct cluster_params *cl_params = (struct cluster_params *)args;
    uint32_t *output = (*cl_params).output;
    int n5 = (*cl_params).n5;
    int n4 = (*cl_params).n4;
    int n6 = (*cl_params).n6;
    uint32_t *e7 = (*cl_params).e7;
    uint32_t *e8 = (*cl_params).e8;
    {
        #pragma omp parallel for
        for (int i_199 = 0; i_199 < n4; i_199 = 1 + i_199)
        {
            /* reduceSeq */
            {
                struct Record_uint32_t__uint32_t_uint32_t_ x155;
                x155._fst = (uint32_t)4294967295;
                x155._snd._fst = (uint32_t)0;
                x155._snd._snd = (uint32_t)0;
                for (int i_200 = 0; i_200 < n5; i_200 = 1 + i_200)
                {
                    /* reduceSeq */
                    {
                        uint32_t x168;
                        x168 = (uint32_t)0;
                        for (int i_201 = 0; i_201 < n6; i_201 = 1 + i_201)
                        {
                            x168 = x168 + ((e7[i_201 + (i_199 * n6)] - e8[i_201 + (i_200 * n6)]) * (e7[i_201 + (i_199 * n6)] - e8[i_201 + (i_200 * n6)]));
                        }

                        {
                            struct Record_uint32_t__uint32_t_uint32_t_ x166;
                            x166 = test(x168, x155);
                            x155._fst = x166._fst;
                            x155._snd._fst = x166._snd._fst;
                            x155._snd._snd = x166._snd._snd;
                        }
                    }
                }

                output[i_199] = x155._snd._snd;
            }
        }
    }
    pi_perf_stop();
    int time2 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);
    printf("Total cycles: %d\n", time2 - time1);

    /*uint8_t cnt[3] = {0};
    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        cnt[output[i]]++;
    }
    for (int i = 0; i < 3; ++i)
    {
        printf("Cluster %d contains %d items\n", i, cnt[i]);
    }*/
}

//Host code

//#include "gap8/gap8.h"
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

void foo_run(Context ctx, foo_t *self, Buffer moutput, int n4, int n5, int n6, Buffer me7, Buffer me8)
{
    {
        DeviceBuffer b0 = deviceBufferSync(ctx, moutput, n4 * sizeof(uint32_t), 0);
        int b1 = n5;
        int b2 = n4;
        int b3 = n6;
        DeviceBuffer b4 = deviceBufferSync(ctx, me7, n4 * (n6 * sizeof(uint32_t)), 0);
        DeviceBuffer b5 = deviceBufferSync(ctx, me8, n5 * (n6 * sizeof(uint32_t)), 0);
        struct cluster_params *cl_params = (struct cluster_params *)pmsis_l2_malloc(sizeof(struct cluster_params));
        (*cl_params).output = b0;
        (*cl_params).n5 = b1;
        (*cl_params).n4 = b2;
        (*cl_params).n6 = b3;
        (*cl_params).e7 = b4;
        (*cl_params).e8 = b5;
        launchKernel(ctx, (*self).cluster_core_task, 8, cl_params);
    }
}

void foo_init_run(Context ctx, Buffer moutput, int n4, int n5, int n6, Buffer me7, Buffer me8)
{
    foo_t foo;
    foo_init(&foo);
    foo_run(ctx, &foo, moutput, n4, n5, n6, me7, me8);
    foo_destroy(ctx, &foo);
}

Buffer points;
Buffer clusters;
Buffer assignments;

#define STACK_SIZE 2048
#define K 3
#define NUM_ITERS 1000

uint32_t means[K * NUM_FEATURES] = {
    1050, 1050,
    5000, 5000,
    9500, 9500};

uint32_t whatishere[NUM_ITEMS] = {0};

struct pi_device gpio_a1;
struct pi_gpio_conf gpio_conf;

#define FREQ_FC (250*1000000)
#define FREQ_CL (175*1000000)

/* Entry point - Executes on FC */
void __main()
{
    printf("Main FC entry point\n");

    pi_pad_set_function(PI_PAD_12_A3_RF_PACTRL0, PI_PAD_12_A3_GPIO_A0_FUNC1);
    pi_gpio_e gpio_out_a1 = PI_GPIO_A0_PAD_12_A3;
    pi_gpio_flags_e cfg_flags = PI_GPIO_OUTPUT;
    pi_gpio_pin_configure(&gpio_a1, gpio_out_a1, cfg_flags);
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);

    Context ctx = createDefaultContext();
    points = createBuffer(ctx, NUM_FEATURES * NUM_ITEMS, 0);
    clusters = createBuffer(ctx, K * NUM_FEATURES, 0);
    assignments = createBuffer(ctx, NUM_ITEMS, 0);
    points->inner = items_2d;
    clusters->inner = means;
    assignments->inner = whatishere;

    printf("FC FREQ: %d\n", rt_freq_get(RT_FREQ_DOMAIN_FC));
    printf("CL FREQ: %d\n", rt_freq_get(RT_FREQ_DOMAIN_CL));

    printf("Start \n");
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 1);
    long time_usec1 = rt_time_get_us();

    foo_init_run(ctx, assignments, NUM_ITEMS, K, NUM_FEATURES, points, clusters);

    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);
    long time_usec2 = rt_time_get_us();
    printf("Wall clock time: %ld usec\n", time_usec2 - time_usec1);
    printf("End \n");

    destroyBuffer(ctx, points);
    destroyBuffer(ctx, assignments);
    destroyBuffer(ctx, clusters);
    destroyContext(ctx);

    pmsis_exit(0);
}

int main(int argc, char **argv)
{
    printf("\n\n\t *** KMeans (RISE) ***\n\n");
    return pmsis_kickoff((void *)__main);
}
