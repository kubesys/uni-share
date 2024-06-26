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
#include "control.h"
#include "hijack.h"
#include "container.h"

static int cur_cores = 0;
static int total_cores = 0;
extern int total_mlu_cores;
extern int total_gpu_cores;

extern int card_core_count;
extern int card_thread_pre_core;

extern int sm_num;
extern int max_thread_per_sm;

extern rc_data_t config_data;

static const struct timespec g_cycle = {
        .tv_sec = 0,
        .tv_nsec = TIME_TICK * MILLISEC,
};
static const struct timespec g_wait = {
        .tv_sec = 0,
        .tv_nsec = 200 * MILLISEC,
};

void *source_control(void *arg) {
	int yaml_limit = config_data.utilization;
	int share = 0;
	__uint32_t container_total_uti = 0;

  const char *accelerator = (const char *)arg;


    if (strcmp(accelerator, "mlu") == 0) {
        initCndev();
        total_cores = total_mlu_cores;
    }
    else {
        initCudev();
        total_cores = total_gpu_cores;
    }

	while(1) {
        nanosleep(&g_wait, NULL);
        if (strcmp(accelerator, "mlu") == 0)
        {
            container_total_uti = get_uti("mlu");
            if (container_total_uti < yaml_limit / 10) {
        	    cur_cores = delta_for_mlu(yaml_limit, container_total_uti, share);
        	    continue;
            }
      	    share = delta_for_mlu(yaml_limit, container_total_uti, share);
        }
        else if (strcmp(accelerator, "gpu") == 0) {
            container_total_uti = get_uti("gpu");
            if (container_total_uti < yaml_limit / 10) {
        	    cur_cores = delta_for_gpu(yaml_limit, container_total_uti, share);
        	    continue;
            }
      	    share = delta_for_gpu(yaml_limit, container_total_uti, share);
        }
		
	  	change_token(share);
	}
}

int delta_for_mlu(int up_limit, int user_current, int share) {
  int utilization_diff =
          abs(up_limit - user_current) < 5 ? 5 : abs(up_limit - user_current);
  int increment =
          card_core_count * card_core_count * card_thread_pre_core * utilization_diff / 256;
  
  if (utilization_diff > up_limit / 2) {
    increment = increment * utilization_diff * 2 / (up_limit + 1);
  }

  if (user_current <= up_limit) {
    share = share + increment > total_cores ? total_cores
                                                   : share + increment;
  } else {
    share = share - increment < 0 ? 0 : share - increment;
  }

  return share;
}

int delta_for_gpu(int up_limit, int user_current, int share) {
  int utilization_diff =
          abs(up_limit - user_current) < 5 ? 5 : abs(up_limit - user_current);
  int increment =
          sm_num * sm_num * max_thread_per_sm * utilization_diff / 2560;
  
  if (utilization_diff > up_limit / 2) {
    increment = increment * utilization_diff * 2 / (up_limit + 1);
  }

  if (user_current <= up_limit) {
    share = share + increment > total_cores ? total_cores
                                                   : share + increment;
  } else {
    share = share - increment < 0 ? 0 : share - increment;
  }

  return share;
}

static void change_token(int delta) {
  int cores_before = 0, cores_after = 0;

  LOGGER(5, "delta: %d, curr: %d", delta, cur_cores);
  do {
    cores_before = cur_cores;
    cores_after = cores_before + delta;

    if (unlikely(cores_after > total_cores)) {
      cores_after = total_cores;
    }
  } while (!CAS(&cur_cores, cores_before, cores_after));
}

void rate_limiter(int grids, int blocks) {
    int before_cores = 0;
    int after_cores = 0;
    int kernel_size = grids;
    
    LOGGER(5, "grid: %d, blocks: %d", grids, blocks);
    LOGGER(5, "launch kernel %d, curr core: %d", kernel_size, cur_cores);
    
        do {
        CHECK:
        before_cores = cur_cores;
        LOGGER(8, "current core: %d", cur_cores);
        if (before_cores < 0) {
          nanosleep(&g_cycle, NULL);
          goto CHECK;
        }
        after_cores = before_cores - kernel_size;
      } while (!CAS(&cur_cores, before_cores, after_cores));

}