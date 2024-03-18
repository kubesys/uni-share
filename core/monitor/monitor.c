#include "monitor.h"

pthread_once_t mlu_dev_init = PTHREAD_ONCE_INIT;
pthread_once_t gpu_dev_init = PTHREAD_ONCE_INIT;

__uint32_t get_uti(const char* device) {
    if (!strcmp(device, "mlu")) {
        pthread_once(&mlu_dev_init, initCndev);
        return get_mlu_uti();
    }
    else if (!strcmp(device, "gpu")) {
        pthread_once(&gpu_dev_init, initCudev);
        return get_gpu_uti();
    }
    else {
        printf("Not support!\n");
        exit(0);
    }

}