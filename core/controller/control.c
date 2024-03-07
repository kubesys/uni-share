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

static void *source_control() {
	int up_limit = 20;
	int yaml_limit = 20;
	int share = 0;
	__uint32_t container_total_uti = 0;

	while(1) {
		container_total_uti = get_uti("mlu"); 
		printf("container_total_uti is %d%%\n", container_total_uti);
		
			if (container_total_uti < up_limit / 10) {
        		cur_mlu_cores = delta(yaml_limit, container_total_uti, share);
        		continue;
        	}
      		share = delta(yaml_limit, container_total_uti, share);
	  		change_token(share);
		
	}
}

int delta(int up_limit, int user_current, int share) {
  int utilization_diff =
          abs(up_limit - user_current) < 5 ? 5 : abs(up_limit - user_current);
  int increment =
          card_core_count * card_core_count * card_thread_pre_core * utilization_diff / 256;
  
  if (utilization_diff > up_limit / 2) {
    increment = increment * utilization_diff * 2 / (up_limit + 1);
  }

  if (user_current <= up_limit) {
    share = share + increment > total_mlu_cores ? total_mlu_cores
                                                   : share + increment;
  } else {
    share = share - increment < 0 ? 0 : share - increment;
  }

  return share;
}

static void change_token(int delta) {
  int mlu_cores_before = 0, mlu_cores_after = 0;

  LOGGER(5, "delta: %d, curr: %d", delta, cur_mlu_cores);
  do {
    mlu_cores_before = cur_mlu_cores;
    mlu_cores_after = mlu_cores_before + delta;

    if (unlikely(mlu_cores_after > total_mlu_cores)) {
      mlu_cores_after = total_mlu_cores;
    }
  } while (!CAS(&cur_mlu_cores, mlu_cores_before, mlu_cores_after));
}

static void rate_limiter(int grids, int blocks) {
  int before_mlu_cores = 0;
  int after_mlu_cores = 0;
  int kernel_size = grids * blocks;

  LOGGER(5, "grid: %d, blocks: %d", grids, blocks);
  LOGGER(5, "launch kernel %d, curr core: %d", kernel_size, cur_mlu_cores);

    do {
      CHECK:
      before_mlu_cores = cur_mlu_cores;
      LOGGER(8, "current core: %d", cur_mlu_cores);
      if (before_mlu_cores < 0) {
        nanosleep(&g_cycle, NULL);
        goto CHECK;
      }
      after_mlu_cores = before_mlu_cores - kernel_size;
    } while (!CAS(&cur_mlu_cores, before_mlu_cores, after_mlu_cores));

}