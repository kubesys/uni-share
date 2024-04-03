#include "monitor.h"
#include "hijack.h"

int sm_num = 0;
int max_thread_per_sm = 0;
int total_gpu_cores = 0;
extern entry_t cuda_library_entry[];

void initCudev() {
    nvmlReturn_t res;
    CUresult ret;

    res = nvmlInit();
    if (NVML_SUCCESS != res) {
        LOGGER(4, "nvmlInit: %s", nvmlErrorString(ret));
        return ;
    }
    ret = MY_CALL_ENTRY(REAL_GPU_FUNC_PTR(cuInit), 0);
    if (unlikely(ret)) {
        LOGGER(FATAL, "cuInit failed");
    }
    ret = MY_CALL_ENTRY(REAL_GPU_FUNC_PTR(cuDeviceGetAttribute), &sm_num, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, 0);
    if (unlikely(ret)) {
        LOGGER(FATAL, "can't get processor number");
    }
    ret = MY_CALL_ENTRY(REAL_GPU_FUNC_PTR(cuDeviceGetAttribute), &max_thread_per_sm, CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR, 0);
    if (unlikely(ret)) {
        LOGGER(FATAL, "can't get max thread per processor");     
    }

    total_gpu_cores = sm_num * max_thread_per_sm * 32;


}

__uint32_t get_gpu_uti() {
  nvmlProcessUtilizationSample_t processes_sample[MAX_PIDS];
  int processes_num = MAX_PIDS;
  unsigned int running_processes = MAX_PIDS;
  nvmlProcessInfo_t pids_on_device[MAX_PIDS];
  nvmlDevice_t dev;
  utilization_t utiInfo;
  nvmlReturn_t ret;
  struct timeval cur;
  size_t microsec;
  int codec_util = 0;

  int i;

  ret = nvmlDeviceGetHandleByIndex(0, &dev);
  if (unlikely(ret)) {
    LOGGER(4, "nvmlDeviceGetHandleByIndex: %s", nvmlErrorString(ret));
    exit(1);
  }

  ret = nvmlDeviceGetComputeRunningProcesses(dev, &running_processes, pids_on_device);
  if (unlikely(ret)) {
    LOGGER(4, "nvmlDeviceGetComputeRunningProcesses: %s", nvmlErrorString(ret));
    //exit(1);
  }

  utiInfo.sys_process_num = running_processes;

  gettimeofday(&cur, NULL);
  microsec = (cur.tv_sec - 1) * 1000UL * 1000UL + cur.tv_usec;
  utiInfo.checktime = microsec;
  ret = nvmlDeviceGetProcessUtilization(dev, processes_sample, &processes_num, microsec);
  if (unlikely(ret)) {
    LOGGER(4, "nvmlDeviceGetProcessUtilization: %s", nvmlErrorString(ret));
    //exit(1);
  }

  utiInfo.user_current = 0;
  //utiInfo.sys_current = 0;
  for (i = 0; i < processes_num; i++) {
    if (processes_sample[i].timeStamp >= utiInfo.checktime) {
      utiInfo.valid = 1;
      utiInfo.user_current += GET_VALID_VALUE(processes_sample[i].smUtil);

      codec_util = GET_VALID_VALUE(processes_sample[i].encUtil) +
                   GET_VALID_VALUE(processes_sample[i].decUtil);
      utiInfo.user_current += CODEC_NORMALIZE(codec_util);

      }
    }
  

  //LOGGER(5, "sys utilization: %d", utiInfo.sys_current);
  LOGGER(5, "used utilization: %d", utiInfo.user_current);

  return utiInfo.user_current;
}