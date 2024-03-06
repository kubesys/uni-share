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

#include "hijack.h"

//void *libdlHandle = __libc_dlopen_mode("libdl.so", RTLD_LAZY);
//void *libcudaHandle = __libc_dlopen_mode("libcndrv.so", RTLD_LAZY);

void *real_dlsym(void *handle, const char *symbol) {
      static fnDlsym internal_dlsym;
	  internal_dlsym = (fnDlsym)__libc_dlsym(__libc_dlopen_mode("libdl.so", RTLD_LAZY), "dlsym");
  return (*internal_dlsym)(handle, symbol);
}

void* dlsym(void* handle, const char* symbol) {
  // Early out if not a CUDA driver symbol
  if (strncmp(symbol, "cn", 2) != 0  && strncmp(symbol, "cu", 2) != 0) {
    return (real_dlsym(handle, symbol));
  }

  if (strcmp(symbol, "cnGetLibVersion") == 0) {
    return (void*)(&cnGetLibVersion);
  } else if (strcmp(symbol, "cnInit") == 0) {
    return (void*)(&cnInit);
  } else if (strcmp(symbol, "cnGetExportFunction") == 0) {
    return (void*)(&cnGetExportFunction);
  } else if (strcmp(symbol, "cnInvokeKernel") == 0) {
    return (void*)(&cnInvokeKernel);
  } else if (strcmp(symbol, "cnMalloc") == 0) {
    return (void*)(&cnMalloc);
  }
  return (real_dlsym(handle, symbol));
}