#include <stdio.h>

/* PMSIS includes. */
#include "pmsis.h"
#include "means.h"

#define STACK_SIZE 2048
#define K 3
#define NUM_ITERS 1000

uint32_t SquareRoot(uint32_t a_nInput)
{
    uint32_t op  = a_nInput;
    uint32_t res = 0;
    uint32_t one = 1uL << 30;

    while (one > op)
    {
        one >>= 2;
    }

    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
}

uint32_t distance(uint32_t* first, uint32_t* second){
    uint32_t tmpsum = 0;
    for(int i = 0; i < NUM_FEATURES; ++i){
        tmpsum += ((first[i] - second[i]) * (first[i] - second[i]));
    }
    return SquareRoot(tmpsum);
}

uint32_t classify(uint32_t means[K][NUM_FEATURES], uint32_t item[NUM_FEATURES]){
    uint32_t mindistance = UINT32_MAX;
    uint32_t tmpindex = -1;

    for(int i = 0; i < K; ++i){
        uint32_t tmpdistance = distance(means[i], item);
        if(tmpdistance < mindistance){
            mindistance = tmpdistance;
            tmpindex = i;
        }
    }

    return tmpindex;
}

void updatemean(uint32_t clustersize, uint32_t mean[NUM_FEATURES], uint32_t item[NUM_FEATURES]){
    for(int i = 0; i < NUM_FEATURES; ++i){
        uint32_t tmpcalc = mean[i];
        tmpcalc = (tmpcalc * (clustersize - 1) + item[i]) / clustersize;
        mean[i] = tmpcalc;
    }
}

void cluster_entry_point(void* args)
{
    /* Benchmarking. Count active cycles */
    pi_perf_conf(1 << PI_PERF_ACTIVE_CYCLES);
    pi_perf_reset();
    pi_perf_start();
    int time1 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);

    uint32_t means[K][NUM_FEATURES] = {
        {1050, 1050},
        {5000, 5000},
        {9500, 9500}
    };

    uint32_t clustersizes[K] = {0};
    uint32_t belongsto[NUM_ITEMS] = {0};

    #pragma omp parallel for
    for(int itemindex = 0; itemindex < NUM_ITEMS; ++itemindex){
        uint32_t clusterindex = classify(means, items_2d[itemindex]);
        //printf("Closest cluster to item %d is cluster %ld\n", itemindex, clusterindex);
        clustersizes[clusterindex] += 1;
        //updatemean(clustersizes[clusterindex], means[clusterindex], items_2d[itemindex]);

        //if(clusterindex != belongsto[itemindex]){
        //    nochange = 0;
        //}

        belongsto[itemindex] = clusterindex;
    }


    /* Stop the counter and print # active cycles */
    pi_perf_stop();
    int time2 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);
    printf("Total cycles: %d\n", time2 - time1);

    /*uint8_t cnt[3] = {0};
    for(int i = 0; i < NUM_ITEMS; ++i){
        cnt[belongsto[i]]++;
    }
    for(int i = 0; i < 3; ++i){
        printf("Cluster %d contains %d items\n", i, cnt[i]);
    }*/
}

struct pi_device gpio_a1;
struct pi_gpio_conf gpio_conf;

#define FREQ_FC (250*1000000)
#define FREQ_CL (175*1000000)

/* Entry point - Executes on FC */
void sobel_filter_main()
{
    printf("Main FC entry point\n");

    pi_pad_set_function(PI_PAD_12_A3_RF_PACTRL0, PI_PAD_12_A3_GPIO_A0_FUNC1);
    pi_gpio_e gpio_out_a1 = PI_GPIO_A0_PAD_12_A3;
    pi_gpio_flags_e cfg_flags = PI_GPIO_OUTPUT;
    pi_gpio_pin_configure(&gpio_a1, gpio_out_a1, cfg_flags);
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);

    /* Prepare cluster description structure and open cluster */
    struct pi_device cl_device;
    struct pi_cluster_conf cl_configuration;
    pi_cluster_conf_init(&cl_configuration);
    cl_configuration.id = 0;
    pi_open_from_conf(&cl_device, &cl_configuration);
    if(pi_cluster_open(&cl_device))
    {
        printf("Cluster open failed\n");
        pmsis_exit(-1);
    }

    printf("FC FREQ: %d\n", rt_freq_get(RT_FREQ_DOMAIN_FC));
    printf("CL FREQ: %d\n", rt_freq_get(RT_FREQ_DOMAIN_CL));

    /* Prepare task description structure */
    struct pi_cluster_task * cl_task = pmsis_l2_malloc(sizeof(struct pi_cluster_task));
    memset(cl_task, 0, sizeof(struct pi_cluster_task));
    cl_task->entry = cluster_entry_point;
    cl_task->arg = NULL;
    cl_task->stack_size = (uint32_t) STACK_SIZE;

    printf("Start \n");
    /* Benchmarking. Count active cycles */
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 1);
    long time_usec1 = rt_time_get_us();

    /* Send task to cluster, block until completion */
    pi_cluster_send_task_to_cl(&cl_device, cl_task);

    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);
    long time_usec2 = rt_time_get_us();
    printf("Wall clock time: %ld usec\n", time_usec2 - time_usec1);
    printf("End \n");

    pi_cluster_close(&cl_device);

    printf("Cluster closed, over and out\n");

    pmsis_exit(0);
}

/* PMSIS main function */
int main(int argc, char *argv[])
{
    printf("\n\n\t *** KMeans (OMP) ***\n\n");
    return pmsis_kickoff((void *) sobel_filter_main);
}
