#ifndef __MONITOR_H
#define __MONITOR_H

#include <nvml.h>
#include <cndev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

#define GET_VALID_VALUE(x) (((x) >= 0 && (x) <= 100) ? (x) : 0)
#define CODEC_NORMALIZE(x) (x * 85 / 100)

#define MAX_PIDS 1024

typedef struct {
  __uint32_t user_current;
  //__uint32_t sys_current;
  int valid;
  uint64_t checktime;
  int sys_process_num;
} utilization_t;


void initCndev();
void initCudev();
__uint32_t get_mlu_uti();
__uint32_t get_gpu_uti();
__uint32_t get_uti(const char* device);


#endif