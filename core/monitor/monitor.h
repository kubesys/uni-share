#ifndef __MONITOR_H
#define __MONITOR_H

#include <nvml.h>
#include <cndev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define GET_VALID_VALUE(x) (((x) >= 0 && (x) <= 100) ? (x) : 0)
#define CODEC_NORMALIZE(x) (x * 85 / 100)

typedef struct {
  int user_current;
  //int sys_current;
  int valid;
  uint64_t checktime;
  int sys_process_num;
} utilization_t;


void initCndev();
void initCudev();
static __uint32_t get_mlu_uti();
static void get_gpu_uti(int fd, void *arg)


#endif