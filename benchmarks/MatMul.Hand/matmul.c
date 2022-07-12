#include <stdio.h>

/* PMSIS includes. */
#include "pmsis.h"

#define STACK_SIZE 2048

#define ROWS 250
#define COLS 250

/* Pointers in memory for input and output matrices */
uint32_t inmat[ROWS][COLS];
uint32_t outmat[ROWS][COLS];

/* Main cluster entry point, executed on core 0 */
void cluster_entry_point(void* args)
{    

    pi_perf_conf(1 << PI_PERF_ACTIVE_CYCLES);
    pi_perf_reset();
    pi_perf_start();
    int time_cycles1 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);

    /* This directive will parallelize on all of the cluster cores */
    #pragma omp parallel for collapse(2)
    for(int i = 0; i < ROWS; ++i){
        for(int j = 0; j < COLS; ++j){
            uint32_t tmp = 0;
            for(int k = 0; k < ROWS; ++k){
                tmp += inmat[i][k] * inmat[k][j];
            }
            //#pragma omp atomic
            outmat[i][j] = tmp;
        }
    }

    pi_perf_stop();
    int time_cycles2 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);
    printf("Total cycles: %d\n", time_cycles2 - time_cycles1);
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

    for(int i = 0; i < ROWS; ++i) {
        for(int j = 0; j < COLS; ++j) {
            inmat[i][j] = i + j;
        }
    }

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

    /* Stop the counter and print # active cycles */
    
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
    printf("\n\n\t *** MatMul - Hand (OMP) ***\n\n");
    return pmsis_kickoff((void *) sobel_filter_main);
}
