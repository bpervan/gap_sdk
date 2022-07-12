#include <stdio.h>
#include "Gap8.h"
#include "CNN_HwCE.h"

#define STACK_SIZE 1024
#define MOUNT 1
#define UNMOUNT 0
#define CID 0

#define IMG_W 12
#define IMG_H 12

void cluster_core_task(void* args)
{
    pi_perf_conf(1 << PI_PERF_ACTIVE_CYCLES);
    pi_perf_reset();
    pi_perf_start();
    int time_cycles1 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);

    // initialize the performance clock
    eu_evt_maskSet(1 << ARCHI_CL_EVT_ACC0);

    HWCE_Enable();
    //Conv format, WStride, QNorm
    HWCE_GenericInit((unsigned int)(HWCE_CONV3x3), (unsigned int)(0), (unsigned int)(0));
    //HwCE_SetInputBias(0);
    HwCE_SetYinMode((unsigned int)(1));

    short int In[] = {
        -54, -42, -84, -72, -36,  -89, -56, -37,  69,  -9,  79,  64,
        89,  90, -31,  -6, -21,    7,  -8, -18,  98, -12, -43,  74,
        45,  -2,  83,  36, -22,   72,  -7,  52,  23, -27,  39,  65,
        91, -61, -30, -15, -67,   96,  29,  85, -71,  97, -59, -57,
        -23, -69, -76,  38,  70,   67, -79, -47,  71,  73, -39, -97,
        6, -81, -63,  86,  15,  -25,   5,   3,  99, -26, -99,  42,
        13, -95, -78,  88, -80,   84, -10, -44, -20,  92,  62,  94,
        20,  57,  26, -55, -17,  -14,  56,  93, -86, -77, -40,  31,
        66,  50,  54, -49, -73, -100, -94, 100,   1, -90,  11,  30,
        55, -34,  51, -96,  81,    0,  47, -11,  78,  60,   4,  40,
        18, -46,  46,  43, -62,   37,  48,  19, -87, -88, -16, -19,
        -93,  58,  24,  59,  61,  -32,  17, -41,  14,  -5,  34, -35
    };

    short int Filter3x3_X[] = {
        1, 0, 1,
        2, 0, 2,
        1, 0, 1,
        0
    };

    short int Out[(IMG_W - 2) * (IMG_H -2)] = {-1};

    HWCE_ProcessOneTile3x3_MultiOut(In, Out, NULL, NULL, Filter3x3_X, 0, IMG_W, IMG_H, HWCE_ONE_OUT);

    HWCE_Disable();

    pi_perf_stop();
    int time_cycles2 = pi_perf_read(PI_PERF_ACTIVE_CYCLES);
    printf("Total cycles: %d\n", time_cycles2 - time_cycles1);

    /*for(int i = 0; i < (IMG_H - 2) * (IMG_W - 2); ++i){
        printf("%d ", Out[i]);
        if(((i + 1) % (IMG_H - 2) * (IMG_W - 2)) == 0){
            printf("\n");
        }
    }*/
}

struct pi_device gpio_a1;
struct pi_gpio_conf gpio_conf;

#define FREQ_FC (250*1000000)
#define FREQ_CL (175*1000000)

int main()

{
    printf("\n\n\t *** Convolution (HWCE) ***\n\n");
    printf("Main FC entry point\n");

    pi_pad_set_function(PI_PAD_12_A3_RF_PACTRL0, PI_PAD_12_A3_GPIO_A0_FUNC1);
    pi_gpio_e gpio_out_a1 = PI_GPIO_A0_PAD_12_A3;
    pi_gpio_flags_e cfg_flags = PI_GPIO_OUTPUT;
    pi_gpio_pin_configure(&gpio_a1, gpio_out_a1, cfg_flags);
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);
    
    if (rt_event_alloc(NULL, 8))
        return -1;

    rt_cluster_mount(MOUNT, CID, 0, NULL);

    printf("FC FREQ: %d\n", rt_freq_get(RT_FREQ_DOMAIN_FC));
    printf("CL FREQ: %d\n", rt_freq_get(RT_FREQ_DOMAIN_CL));

    // Allocate some stacks for cluster in L1, rt_nb_pe returns how many cores exist.
    void *stacks = rt_alloc(RT_ALLOC_CL_DATA, STACK_SIZE * rt_nb_pe());
    if (stacks == NULL)
        return -1;

    printf("Start \n");
    /* Benchmarking. Count active cycles */
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 1);
    long time_usec1 = rt_time_get_us();

    rt_cluster_call(NULL, CID, (void *)cluster_core_task, NULL, stacks, STACK_SIZE, STACK_SIZE, rt_nb_pe(), NULL);
    
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);
    long time_usec2 = rt_time_get_us();
    printf("Wall clock time: %ld usec\n", time_usec2 - time_usec1);
    printf("End \n");
    
    
    // Close the cluster
    rt_cluster_mount(UNMOUNT, CID, 0, NULL);

    //Add Buffer Free
    printf("Cluster closed, over and out\n");

    return 0;
}
