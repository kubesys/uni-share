void initCudev() {
    nvmlReturn_t res;

    res = nvmlInit();
    if (NVML_SUCCESS != res) {
        LOGGER(4, "nvmlInit: %s", nvml_error(ret));
        return ;
    }

}

void get_gpu_uti() {
  nvmlProcessUtilizationSample_t processes_sample[MAX_PIDS];
  int processes_num = MAX_PIDS;
  unsigned int running_processes = MAX_PIDS;
  nvmlProcessInfo_t pids_on_device[MAX_PIDS];
  nvmlDevice_t dev;
  //utilization_t *top_result = (utilization_t *)arg;
  nvmlReturn_t ret;
  struct timeval cur;
  size_t microsec;
  int codec_util = 0;

  int i;

  ret = nvmlDeviceGetHandleByIndex(0, &dev);
  if (unlikely(ret)) {
    LOGGER(4, "nvmlDeviceGetHandleByIndex: %s", nvml_error(ret));
    return;
  }

  ret = nvmlDeviceGetComputeRunningProcesses(dev, &running_processes, pids_on_device);
  if (unlikely(ret)) {
    LOGGER(4, "nvmlDeviceGetComputeRunningProcesses: %s", nvml_error(ret));
    return;
  }

  top_result->sys_process_num = running_processes;

  gettimeofday(&cur, NULL);
  microsec = (cur.tv_sec - 1) * 1000UL * 1000UL + cur.tv_usec;
  top_result->checktime = microsec;
  ret = nvmlDeviceGetProcessUtilization(dev, processes_sample, &processes_num, microsec);
  if (unlikely(ret)) {
    LOGGER(4, "nvmlDeviceGetProcessUtilization: %s", nvml_error(ret));
    return;
  }

  top_result->user_current = 0;
  //top_result->sys_current = 0;
  for (i = 0; i < processes_num; i++) {
    if (processes_sample[i].timeStamp >= top_result->checktime) {
      top_result->valid = 1;
      top_result->user_current += GET_VALID_VALUE(processes_sample[i].smUtil);

      codec_util = GET_VALID_VALUE(processes_sample[i].encUtil) +
                   GET_VALID_VALUE(processes_sample[i].decUtil);
      top_result->user_current += CODEC_NORMALIZE(codec_util);

      }
    }
  }

  //LOGGER(5, "sys utilization: %d", top_result->sys_current);
  LOGGER(5, "used utilization: %d", top_result->user_current);
}