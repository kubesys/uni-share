/**
 * Copyright (2024, ) Institute of Software, Chinese Academy of Sciences
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * author: wuyangyang99@otcaix.iscas.ac.cn 
 *         wuheng@iscas.ac.cn
 * since:  0.1
 */
#include "monitor.h"

int card_core_count = 0;
int total_mlu_cores = 0;
int card_thread_pre_core = 256;


void initCndev() {
    cndevDevice_t devHandle;
    cndevRet_t ret = cndevInit(0);
    // you can compare cndevRet manually, or just use cndevCheckErrors
    if (CNDEV_SUCCESS != ret) {
      printf("cndev init failed: %s.\n", cndevGetErrorString(ret));
      // should exit now
      exit(0);
    }
    ret = cndevGetDeviceHandleByIndex(0, &devHandle);
    // get card[x]'s core count
    cndevCardCoreCount_t cardCoreCount;
    cardCoreCount.version = CNDEV_VERSION_5;
    cndevCheckErrors(cndevGetCoreCount(&cardCoreCount, devHandle));
    card_core_count = cardCoreCount.count;

    total_mlu_cores = card_core_count * card_thread_pre_core * 32;
}

__uint32_t get_mlu_uti() {
    cndevDevice_t devHandle;
    cndevRet_t ret;
    unsigned int pCount = 10;
    __uint32_t container_total_uti = 0;
    int sys_total_uti = 0;
    cndevUtilizationInfo_t utilInfo;
    cndevProcessUtilization_t *procUtil = NULL;

    ret = cndevGetDeviceHandleByIndex(0, &devHandle);

    // get card[x]'s MLU utilization
    utilInfo.version = CNDEV_VERSION_5;
    cndevCheckErrors(cndevGetDeviceUtilizationInfo(&utilInfo, devHandle));
    sys_total_uti = utilInfo.averageCoreUtilization;
      //printf("Util:%d%%\n", sys_total_uti);
    
      procUtil = (cndevProcessUtilization_t *) malloc(pCount * sizeof(cndevProcessUtilization_t));
      procUtil->version = CNDEV_VERSION_5;
      ret = cndevGetProcessUtilization(&pCount, procUtil, devHandle); 
      if (ret == CNDEV_ERROR_NOT_SUPPORTED) {
        printf("cndevGetProcessUtilization is not supported.\n");
      } else {
        // if ret is CNDEV_ERROR_INSUFFICIENT_SPACE, should get again
        while (ret == CNDEV_ERROR_INSUFFICIENT_SPACE) {
          procUtil = (cndevProcessUtilization_t *) realloc(procUtil, pCount * sizeof(cndevProcessUtilization_t));
          ret = cndevGetProcessUtilization(&pCount, procUtil, devHandle);
        }
        cndevCheckErrors(ret);
        //printf("process: count: %d\n", pCount);
        for (unsigned int i = 0; i < pCount; i++) {
          printf("process %d:pid:%u, IPU Util:%u %%, JPU Util:%u %%, VPU Decoder Util:%u %%, VPU Encoder Util:%u %%, Memory Util:%u %%\n",
                i, (procUtil+i)->pid, (procUtil + i)->ipuUtil, (procUtil + i)->jpuUtil,
                (procUtil + i)->vpuDecUtil,
                (procUtil + i)->vpuEncUtil,
                (procUtil + i)->memUtil);
      container_total_uti += (procUtil+i)->ipuUtil;
        }

      }
      free(procUtil);

    return container_total_uti;
}

unsigned long get_mlu_mem() {
    unsigned long proc_used_mem = 0;
    cndevDevice_t devHandle;
    cndevRet_t ret;
      // get card[x]'s process info
    // in most cases, the number of processes running on a card will not exceed 10
    // but if cndevProcessInfo's space is not enough, CNDEV_ERROR_INSUFFICIENT_SPACE will be returned
    unsigned tCount = 10;
    cndevProcessInfo_t *procInfo = NULL;

    ret = cndevGetDeviceHandleByIndex(0, &devHandle);
    procInfo = (cndevProcessInfo_t *) malloc(tCount * sizeof(cndevProcessInfo_t));
    procInfo->version = CNDEV_VERSION_5;
    ret = cndevGetProcessInfo(&tCount, procInfo, devHandle);
    // if ret is CNDEV_ERROR_INSUFFICIENT_SPACE, should get again
    while (ret == CNDEV_ERROR_INSUFFICIENT_SPACE) {
      procInfo = (cndevProcessInfo_t *) realloc(procInfo, tCount * sizeof(cndevProcessInfo_t));
      ret = cndevGetProcessInfo(&tCount, procInfo, devHandle);
    }
    cndevCheckErrors(ret);
    printf("process: count: %d\n", tCount);

    for (unsigned int i = 0; i < tCount; i++) {
      printf("process %d:pid:%u, PhyMem:%luKiB, VirMem:%luKiB\n",
             i,
             (procInfo + i)->pid,
             (procInfo + i)->physicalMemoryUsed,
             (procInfo + i)->virtualMemoryUsed);
      proc_used_mem += (procInfo + i)->physicalMemoryUsed;
    }
    free(procInfo);

    return proc_used_mem;
}