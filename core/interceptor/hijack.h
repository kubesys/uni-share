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
 */
#define _GNU_SOURCE
#ifndef __HIJACK_H
#define __HIJACK_H

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <stdint.h>
#include "cn_api.h"
#include <cndev.h>
#include <time.h>
#include <pthread.h>

void *__libc_dlsym(void *map, const char *name);
void *__libc_dlopen_mode(const char *name, int mode);

void *real_dlsym(void *handle, const char *symbol);
void getMluDriverLibPtrDlsym();
void getGpuDriverLibPtrDlsym();

typedef void *(*fnDlsym)(void *, const char *);

#define MLU_ENTRY_ENUM(x) ENTRY_##x
#define REAL_FUNC_PTR(x) ({ CNDrv_entry[MLU_ENTRY_ENUM(x)].func_ptr; })

#define CUDA_ENTRY_ENUM(x) ENTRY_##x

//#define REAL_FUNC_PTR(table, x) ({ (table)[MLU_ENTRY_ENUM(x)].func_ptr; })
//#define MY_CALL_ENTRY(ptr, params, ...) ({ ((CUresult (*)params)ptr)(__VA_ARGS__); })
typedef CNresult (*cnfunc_type)();
#define MY_CALL_ENTRY(ptr, ...) 					\
	({ 												\
		cnfunc_type _entry = ptr;					\
		_entry(__VA_ARGS__);						\
	})

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define CAS(ptr, old, new) __sync_bool_compare_and_swap((ptr), (old), (new))

#define MILLISEC (1000UL * 1000UL)
#define TIME_TICK (10)

typedef enum {
  INFO = 0,
  ERROR = 1,
  WARNING = 2,
  FATAL = 3,
  VERBOSE = 4,
} log_level_enum_t;

#define LOGGER(level, format, ...)                              \
  ({                                                            \
    char *_print_level_str = getenv("LOGGER_LEVEL");            \
    int _print_level = 3;                                       \
    if (_print_level_str) {                                     \
      _print_level = (int)strtoul(_print_level_str, NULL, 10);  \
      _print_level = _print_level < 0 ? 3 : _print_level;       \
    }                                                           \
    if (level <= _print_level) {                                \
      fprintf(stderr, "%s:%d " format "\n", __FILE__, __LINE__, \
              ##__VA_ARGS__);                                   \
    }                                                           \
    if (level == FATAL) {                                       \
      exit(-1);                                                 \
    }                                                           \
  })


typedef enum {
	MLU_ENTRY_ENUM(cnGetErrorString),
	MLU_ENTRY_ENUM(cnGetErrorName),
	MLU_ENTRY_ENUM(cnInit),
	MLU_ENTRY_ENUM(cnDriverGetVersion),
	MLU_ENTRY_ENUM(cnGetLibVersion),
	MLU_ENTRY_ENUM(cnGetDriverVersion),
	MLU_ENTRY_ENUM(cnDeviceGet),
	MLU_ENTRY_ENUM(cnDeviceGetCount),
	MLU_ENTRY_ENUM(cnDeviceGetName),
	MLU_ENTRY_ENUM(cnDeviceTotalMem),
	MLU_ENTRY_ENUM(cnDeviceGetAttribute),
	MLU_ENTRY_ENUM(cnDeviceGetByPCIBusId),
	MLU_ENTRY_ENUM(cnDeviceGetPCIBusId),
	MLU_ENTRY_ENUM(cnDeviceGetUuid),
	MLU_ENTRY_ENUM(cnDeviceGetUuidStr),
	MLU_ENTRY_ENUM(cnDeviceGetByUuidStr),
	MLU_ENTRY_ENUM(cnCtxCreate),
	MLU_ENTRY_ENUM(cnCtxDestroy),
	MLU_ENTRY_ENUM(cnCtxGetFlags),
	MLU_ENTRY_ENUM(cnCtxGetApiVersion),
	MLU_ENTRY_ENUM(cnCtxGetCurrent),
	MLU_ENTRY_ENUM(cnCtxSetCurrent),
	MLU_ENTRY_ENUM(cnCtxGetDevice),
	MLU_ENTRY_ENUM(cnCtxGetQueuePriorityRange),
	MLU_ENTRY_ENUM(cnCtxGetConfig),
	MLU_ENTRY_ENUM(cnCtxSetConfig),
	MLU_ENTRY_ENUM(cnSetCtxConfigParam_pt),
	MLU_ENTRY_ENUM(cnGetCtxConfigParam_pt),
	MLU_ENTRY_ENUM(cnGetCtxMaxParallelUnionTasks),
	MLU_ENTRY_ENUM(cnCtxSync),
	MLU_ENTRY_ENUM(cnCtxResetPersistingL2Cache),
	MLU_ENTRY_ENUM(cnSharedContextGetState),
	MLU_ENTRY_ENUM(cnSharedContextSetFlags),
	MLU_ENTRY_ENUM(cnSharedContextAcquire),
	MLU_ENTRY_ENUM(cnSharedContextRelease),
	MLU_ENTRY_ENUM(cnSharedContextReset),
	MLU_ENTRY_ENUM(cnMemGetInfo),
	MLU_ENTRY_ENUM(cnMemGetNodeInfo),
	MLU_ENTRY_ENUM(cnMalloc),
	MLU_ENTRY_ENUM(cnMallocSecurity),
	MLU_ENTRY_ENUM(cnMallocNode),
	MLU_ENTRY_ENUM(cnZmalloc),
	MLU_ENTRY_ENUM(cnZmallocNode),
	MLU_ENTRY_ENUM(cnMallocConstant),
	MLU_ENTRY_ENUM(cnMallocNodeConstant),
	MLU_ENTRY_ENUM(cnMallocFrameBuffer),
	MLU_ENTRY_ENUM(cnMallocPeerAble),
	MLU_ENTRY_ENUM(cnFree),
	MLU_ENTRY_ENUM(cnMemMerge),
	MLU_ENTRY_ENUM(cnMemGetAddressRange),
	MLU_ENTRY_ENUM(cnMallocHost),
	MLU_ENTRY_ENUM(cnFreeHost),
	MLU_ENTRY_ENUM(cnIpcGetMemHandle),
	MLU_ENTRY_ENUM(cnIpcOpenMemHandle),
	MLU_ENTRY_ENUM(cnIpcCloseMemHandle),
	MLU_ENTRY_ENUM(cnMemcpy),
	MLU_ENTRY_ENUM(cnMemcpyPeer),
	MLU_ENTRY_ENUM(cnMemcpyHtoD),
	MLU_ENTRY_ENUM(cnMemcpyDtoH),
	MLU_ENTRY_ENUM(cnMemcpyDtoD),
	MLU_ENTRY_ENUM(cnMemcpyDtoD2D),
	MLU_ENTRY_ENUM(cnMemcpyDtoD3D),
	MLU_ENTRY_ENUM(cnMemcpy2D),
	MLU_ENTRY_ENUM(cnMemcpy3D),
	MLU_ENTRY_ENUM(cnMemcpyAsync),
	MLU_ENTRY_ENUM(cnMemcpyAsync_V2),
	MLU_ENTRY_ENUM(cnMemcpyPeerAsync),
	MLU_ENTRY_ENUM(cnMemcpyHtoDAsync),
	MLU_ENTRY_ENUM(cnMemcpyHtoDAsync_V2),
	MLU_ENTRY_ENUM(cnMemcpyDtoHAsync),
	MLU_ENTRY_ENUM(cnMemcpyDtoHAsync_V2),
	MLU_ENTRY_ENUM(cnMemcpyDtoDAsync),
	MLU_ENTRY_ENUM(cnMemsetD8),
	MLU_ENTRY_ENUM(cnMemsetD16),
	MLU_ENTRY_ENUM(cnMemsetD32),
	MLU_ENTRY_ENUM(cnMemsetD8Async),
	MLU_ENTRY_ENUM(cnMemsetD16Async),
	MLU_ENTRY_ENUM(cnMemsetD32Async),
	MLU_ENTRY_ENUM(cnDeviceCanPeerAble),
	MLU_ENTRY_ENUM(cnGetMemAttribute),
	MLU_ENTRY_ENUM(cnGetMemAttributes),
	MLU_ENTRY_ENUM(cnSetMemAttribute),
	MLU_ENTRY_ENUM(cnSetMemRangeAttribute),
	MLU_ENTRY_ENUM(cnCreateQueue),
	MLU_ENTRY_ENUM(cnCreateQueueWithPriority),
	MLU_ENTRY_ENUM(cnQueueGetPriority),
	MLU_ENTRY_ENUM(cnDestroyQueue),
	MLU_ENTRY_ENUM(cnQueryQueue),
	MLU_ENTRY_ENUM(cnQueueSync),
	MLU_ENTRY_ENUM(cnQueueWaitNotifier),
	MLU_ENTRY_ENUM(cnQueueGetContext),
	MLU_ENTRY_ENUM(cnQueueSetAttribute),
	MLU_ENTRY_ENUM(cnQueueGetAttribute),
	MLU_ENTRY_ENUM(cnQueueCopyAttributes),
	MLU_ENTRY_ENUM(cnQueueAddCallback),
	MLU_ENTRY_ENUM(cnQueueBeginCapture),
	MLU_ENTRY_ENUM(cnQueueEndCapture),
	MLU_ENTRY_ENUM(cnQueueIsCapturing),
	MLU_ENTRY_ENUM(cnQueueGetCaptureInfo),
	MLU_ENTRY_ENUM(cnQueueUpdateCaptureDependencies),
	MLU_ENTRY_ENUM(cnCreateNotifier),
	MLU_ENTRY_ENUM(cnDestroyNotifier),
	MLU_ENTRY_ENUM(cnWaitNotifier),
	MLU_ENTRY_ENUM(cnQueryNotifier),
	MLU_ENTRY_ENUM(cnPlaceNotifier),
	MLU_ENTRY_ENUM(cnNotifierElapsedTime),
	MLU_ENTRY_ENUM(cnNotifierElapsedExecTime),
	MLU_ENTRY_ENUM(cnIpcGetNotifierHandle),
	MLU_ENTRY_ENUM(cnIpcOpenNotifierHandle),
	MLU_ENTRY_ENUM(cnQueueAtomicOperation),
	MLU_ENTRY_ENUM(cnAtomicOperation),
	MLU_ENTRY_ENUM(cnAtomicReadOps),
	MLU_ENTRY_ENUM(cnModuleLoadFatBinary),
	MLU_ENTRY_ENUM(cnModuleLoad),
	MLU_ENTRY_ENUM(cnModuleUnload),
	MLU_ENTRY_ENUM(cnModuleQueryMemoryUsage),
	MLU_ENTRY_ENUM(cnModuleQueryFatBinaryMemoryUsage),
	MLU_ENTRY_ENUM(cnModuleGetKernel),
	MLU_ENTRY_ENUM(cnModuleGetSymbol),
	MLU_ENTRY_ENUM(cnKernelGetAttribute),
	MLU_ENTRY_ENUM(cnModuleGetLoadingMode),
	MLU_ENTRY_ENUM(cnInvokeKernel),
	MLU_ENTRY_ENUM(cnInvokeHostFunc),
	MLU_ENTRY_ENUM(cnGetExportFunction),
	MLU_ENTRY_ENUM(cnCacheOperation),
	MLU_ENTRY_ENUM(cnMmap),
	MLU_ENTRY_ENUM(cnMmapCached),
	MLU_ENTRY_ENUM(cnMunmap),
	MLU_ENTRY_ENUM(cnMemCreate),
	MLU_ENTRY_ENUM(cnMemRelease),
	MLU_ENTRY_ENUM(cnMemAddressReserve),
	MLU_ENTRY_ENUM(cnMemAddressFree),
	MLU_ENTRY_ENUM(cnMemMap),
	MLU_ENTRY_ENUM(cnMemSetAccess),
	MLU_ENTRY_ENUM(cnMemUnmap),
	MLU_ENTRY_ENUM(cnMemGetAllocationGranularity),
	MLU_ENTRY_ENUM(cnMemGetAllocationPropertiesFromHandle),
	MLU_ENTRY_ENUM(cnMemGetAccess),
	MLU_ENTRY_ENUM(cnMemRetainAllocationHandle),
	MLU_ENTRY_ENUM(cnMemExportToShareableHandle),
	MLU_ENTRY_ENUM(cnMemImportFromShareableHandle),
	MLU_ENTRY_ENUM(cnTaskTopoCreate),
	MLU_ENTRY_ENUM(cnTaskTopoDestroy),
	MLU_ENTRY_ENUM(cnTaskTopoClone),
	MLU_ENTRY_ENUM(cnTaskTopoNodeFindInClone),
	MLU_ENTRY_ENUM(cnTaskTopoDestroyNode),
	MLU_ENTRY_ENUM(cnTaskTopoGetEdges),
	MLU_ENTRY_ENUM(cnTaskTopoGetNodes),
	MLU_ENTRY_ENUM(cnTaskTopoGetRootNodes),
	MLU_ENTRY_ENUM(cnTaskTopoAddDependencies),
	MLU_ENTRY_ENUM(cnTaskTopoRemoveDependencies),
	MLU_ENTRY_ENUM(cnTaskTopoNodeGetDependencies),
	MLU_ENTRY_ENUM(cnTaskTopoNodeGetDependentNodes),
	MLU_ENTRY_ENUM(cnTaskTopoNodeGetType),
	MLU_ENTRY_ENUM(cnTaskTopoDebugDotPrint),
	MLU_ENTRY_ENUM(cnTaskTopoKernelNodeGetAttribute),
	MLU_ENTRY_ENUM(cnTaskTopoKernelNodeSetAttribute),
	MLU_ENTRY_ENUM(cnTaskTopoKernelNodeCopyAttributes),
	MLU_ENTRY_ENUM(cnUserObjectCreate),
	MLU_ENTRY_ENUM(cnUserObjectAcquire),
	MLU_ENTRY_ENUM(cnUserObjectRelease),
	MLU_ENTRY_ENUM(cnTaskTopoAcquireUserObject),
	MLU_ENTRY_ENUM(cnTaskTopoReleaseUserObject),
	MLU_ENTRY_ENUM(cnTaskTopoAddEmptyNode),
	MLU_ENTRY_ENUM(cnTaskTopoAddHostNode),
	MLU_ENTRY_ENUM(cnTaskTopoHostNodeGetParams),
	MLU_ENTRY_ENUM(cnTaskTopoHostNodeSetParams),
	MLU_ENTRY_ENUM(cnTaskTopoAddKernelNode),
	MLU_ENTRY_ENUM(cnTaskTopoKernelNodeGetParams),
	MLU_ENTRY_ENUM(cnTaskTopoKernelNodeSetParams),
	MLU_ENTRY_ENUM(cnTaskTopoAddMemcpyNode),
	MLU_ENTRY_ENUM(cnTaskTopoMemcpyNodeGetParams),
	MLU_ENTRY_ENUM(cnTaskTopoMemcpyNodeSetParams),
	MLU_ENTRY_ENUM(cnTaskTopoAddMemsetNode),
	MLU_ENTRY_ENUM(cnTaskTopoMemsetNodeGetParams),
	MLU_ENTRY_ENUM(cnTaskTopoMemsetNodeSetParams),
	MLU_ENTRY_ENUM(cnTaskTopoAddChildTopoNode),
	MLU_ENTRY_ENUM(cnTaskTopoChildTopoNodeGetTopo),
	MLU_ENTRY_ENUM(cnTaskTopoInstantiate),
	MLU_ENTRY_ENUM(cnTaskTopoEntityDestroy),
	MLU_ENTRY_ENUM(cnTaskTopoEntityInvoke),
	MLU_ENTRY_ENUM(cnTaskTopoEntityHostNodeSetParams),
	MLU_ENTRY_ENUM(cnTaskTopoEntityKernelNodeSetParams),
	MLU_ENTRY_ENUM(cnTaskTopoEntityMemcpyNodeSetParams),
	MLU_ENTRY_ENUM(cnTaskTopoEntityMemsetNodeSetParams),
	MLU_ENTRY_ENUM(cnTaskTopoEntityChildTopoNodeSetParams),
	MLU_ENTRY_ENUM(cnTaskTopoEntityUpdate),
	MLU_ENTRY_ENUM(cnTaskTopoUpload),
	MLU_ENTRY_END
} cndrv_entry_enum;

/**
 * CUDA library enumerator entry
 */
typedef enum {
  /** cuInit */
  CUDA_ENTRY_ENUM(cuInit),
  /** cuDeviceGet */
  CUDA_ENTRY_ENUM(cuDeviceGet),
  /** cuDeviceGetCount */
  CUDA_ENTRY_ENUM(cuDeviceGetCount),
  /** cuDeviceGetName */
  CUDA_ENTRY_ENUM(cuDeviceGetName),
  /** cuDeviceTotalMem_v2 */
  CUDA_ENTRY_ENUM(cuDeviceTotalMem_v2),
  /** cuDeviceGetAttribute */
  CUDA_ENTRY_ENUM(cuDeviceGetAttribute),
  /** cuDeviceGetP2PAttribute */
  CUDA_ENTRY_ENUM(cuDeviceGetP2PAttribute),
  /** cuDriverGetVersion */
  CUDA_ENTRY_ENUM(cuDriverGetVersion),
  /** cuDeviceGetByPCIBusId */
  CUDA_ENTRY_ENUM(cuDeviceGetByPCIBusId),
  /** cuDeviceGetPCIBusId */
  CUDA_ENTRY_ENUM(cuDeviceGetPCIBusId),
  /** cuDevicePrimaryCtxRetain */
  CUDA_ENTRY_ENUM(cuDevicePrimaryCtxRetain),
  /** cuDevicePrimaryCtxRelease */
  CUDA_ENTRY_ENUM(cuDevicePrimaryCtxRelease),
  /** cuDevicePrimaryCtxSetFlags */
  CUDA_ENTRY_ENUM(cuDevicePrimaryCtxSetFlags),
  /** cuDevicePrimaryCtxGetState */
  CUDA_ENTRY_ENUM(cuDevicePrimaryCtxGetState),
  /** cuDevicePrimaryCtxReset */
  CUDA_ENTRY_ENUM(cuDevicePrimaryCtxReset),
  /** cuCtxCreate_v2 */
  CUDA_ENTRY_ENUM(cuCtxCreate_v2),
  /** cuCtxGetFlags */
  CUDA_ENTRY_ENUM(cuCtxGetFlags),
  /** cuCtxSetCurrent */
  CUDA_ENTRY_ENUM(cuCtxSetCurrent),
  /** cuCtxGetCurrent */
  CUDA_ENTRY_ENUM(cuCtxGetCurrent),
  /** cuCtxDetach */
  CUDA_ENTRY_ENUM(cuCtxDetach),
  /** cuCtxGetApiVersion */
  CUDA_ENTRY_ENUM(cuCtxGetApiVersion),
  /** cuCtxGetDevice */
  CUDA_ENTRY_ENUM(cuCtxGetDevice),
  /** cuCtxGetLimit */
  CUDA_ENTRY_ENUM(cuCtxGetLimit),
  /** cuCtxSetLimit */
  CUDA_ENTRY_ENUM(cuCtxSetLimit),
  /** cuCtxGetCacheConfig */
  CUDA_ENTRY_ENUM(cuCtxGetCacheConfig),
  /** cuCtxSetCacheConfig */
  CUDA_ENTRY_ENUM(cuCtxSetCacheConfig),
  /** cuCtxGetSharedMemConfig */
  CUDA_ENTRY_ENUM(cuCtxGetSharedMemConfig),
  /** cuCtxGetStreamPriorityRange */
  CUDA_ENTRY_ENUM(cuCtxGetStreamPriorityRange),
  /** cuCtxSetSharedMemConfig */
  CUDA_ENTRY_ENUM(cuCtxSetSharedMemConfig),
  /** cuCtxSynchronize */
  CUDA_ENTRY_ENUM(cuCtxSynchronize),
  /** cuModuleLoad */
  CUDA_ENTRY_ENUM(cuModuleLoad),
  /** cuModuleLoadData */
  CUDA_ENTRY_ENUM(cuModuleLoadData),
  /** cuModuleLoadFatBinary */
  CUDA_ENTRY_ENUM(cuModuleLoadFatBinary),
  /** cuModuleUnload */
  CUDA_ENTRY_ENUM(cuModuleUnload),
  /** cuModuleGetFunction */
  CUDA_ENTRY_ENUM(cuModuleGetFunction),
  /** cuModuleGetGlobal_v2 */
  CUDA_ENTRY_ENUM(cuModuleGetGlobal_v2),
  /** cuModuleGetTexRef */
  CUDA_ENTRY_ENUM(cuModuleGetTexRef),
  /** cuModuleGetSurfRef */
  CUDA_ENTRY_ENUM(cuModuleGetSurfRef),
  /** cuLinkCreate */
  CUDA_ENTRY_ENUM(cuLinkCreate),
  /** cuLinkAddData */
  CUDA_ENTRY_ENUM(cuLinkAddData),
  /** cuLinkAddFile */
  CUDA_ENTRY_ENUM(cuLinkAddFile),
  /** cuLinkComplete */
  CUDA_ENTRY_ENUM(cuLinkComplete),
  /** cuLinkDestroy */
  CUDA_ENTRY_ENUM(cuLinkDestroy),
  /** cuMemGetInfo_v2 */
  CUDA_ENTRY_ENUM(cuMemGetInfo_v2),
  /** cuMemAllocManaged */
  CUDA_ENTRY_ENUM(cuMemAllocManaged),
  /** cuMemAlloc_v2 */
  CUDA_ENTRY_ENUM(cuMemAlloc_v2),
  /** cuMemAllocPitch_v2 */
  CUDA_ENTRY_ENUM(cuMemAllocPitch_v2),
  /** cuMemFree_v2 */
  CUDA_ENTRY_ENUM(cuMemFree_v2),
  /** cuMemGetAddressRange_v2 */
  CUDA_ENTRY_ENUM(cuMemGetAddressRange_v2),
  /** cuMemFreeHost */
  CUDA_ENTRY_ENUM(cuMemFreeHost),
  /** cuMemHostAlloc */
  CUDA_ENTRY_ENUM(cuMemHostAlloc),
  /** cuMemHostGetDevicePointer_v2 */
  CUDA_ENTRY_ENUM(cuMemHostGetDevicePointer_v2),
  /** cuMemHostGetFlags */
  CUDA_ENTRY_ENUM(cuMemHostGetFlags),
  /** cuMemHostRegister_v2 */
  CUDA_ENTRY_ENUM(cuMemHostRegister_v2),
  /** cuMemHostUnregister */
  CUDA_ENTRY_ENUM(cuMemHostUnregister),
  /** cuPointerGetAttribute */
  CUDA_ENTRY_ENUM(cuPointerGetAttribute),
  /** cuPointerGetAttributes */
  CUDA_ENTRY_ENUM(cuPointerGetAttributes),
  /** cuMemcpy */
  CUDA_ENTRY_ENUM(cuMemcpy),
  /** cuMemcpy_ptds */
  CUDA_ENTRY_ENUM(cuMemcpy_ptds),
  /** cuMemcpyAsync */
  CUDA_ENTRY_ENUM(cuMemcpyAsync),
  /** cuMemcpyAsync_ptsz */
  CUDA_ENTRY_ENUM(cuMemcpyAsync_ptsz),
  /** cuMemcpyPeer */
  CUDA_ENTRY_ENUM(cuMemcpyPeer),
  /** cuMemcpyPeer_ptds */
  CUDA_ENTRY_ENUM(cuMemcpyPeer_ptds),
  /** cuMemcpyPeerAsync */
  CUDA_ENTRY_ENUM(cuMemcpyPeerAsync),
  /** cuMemcpyPeerAsync_ptsz */
  CUDA_ENTRY_ENUM(cuMemcpyPeerAsync_ptsz),
  /** cuMemcpyHtoD_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyHtoD_v2),
  /** cuMemcpyHtoD_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemcpyHtoD_v2_ptds),
  /** cuMemcpyHtoDAsync_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyHtoDAsync_v2),
  /** cuMemcpyHtoDAsync_v2_ptsz */
  CUDA_ENTRY_ENUM(cuMemcpyHtoDAsync_v2_ptsz),
  /** cuMemcpyDtoH_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyDtoH_v2),
  /** cuMemcpyDtoH_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemcpyDtoH_v2_ptds),
  /** cuMemcpyDtoHAsync_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyDtoHAsync_v2),
  /** cuMemcpyDtoHAsync_v2_ptsz */
  CUDA_ENTRY_ENUM(cuMemcpyDtoHAsync_v2_ptsz),
  /** cuMemcpyDtoD_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyDtoD_v2),
  /** cuMemcpyDtoD_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemcpyDtoD_v2_ptds),
  /** cuMemcpyDtoDAsync_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyDtoDAsync_v2),
  /** cuMemcpyDtoDAsync_v2_ptsz */
  CUDA_ENTRY_ENUM(cuMemcpyDtoDAsync_v2_ptsz),
  /** cuMemcpy2DUnaligned_v2 */
  CUDA_ENTRY_ENUM(cuMemcpy2DUnaligned_v2),
  /** cuMemcpy2DUnaligned_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemcpy2DUnaligned_v2_ptds),
  /** cuMemcpy2DAsync_v2 */
  CUDA_ENTRY_ENUM(cuMemcpy2DAsync_v2),
  /** cuMemcpy2DAsync_v2_ptsz */
  CUDA_ENTRY_ENUM(cuMemcpy2DAsync_v2_ptsz),
  /** cuMemcpy3D_v2 */
  CUDA_ENTRY_ENUM(cuMemcpy3D_v2),
  /** cuMemcpy3D_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemcpy3D_v2_ptds),
  /** cuMemcpy3DAsync_v2 */
  CUDA_ENTRY_ENUM(cuMemcpy3DAsync_v2),
  /** cuMemcpy3DAsync_v2_ptsz */
  CUDA_ENTRY_ENUM(cuMemcpy3DAsync_v2_ptsz),
  /** cuMemcpy3DPeer */
  CUDA_ENTRY_ENUM(cuMemcpy3DPeer),
  /** cuMemcpy3DPeer_ptds */
  CUDA_ENTRY_ENUM(cuMemcpy3DPeer_ptds),
  /** cuMemcpy3DPeerAsync */
  CUDA_ENTRY_ENUM(cuMemcpy3DPeerAsync),
  /** cuMemcpy3DPeerAsync_ptsz */
  CUDA_ENTRY_ENUM(cuMemcpy3DPeerAsync_ptsz),
  /** cuMemsetD8_v2 */
  CUDA_ENTRY_ENUM(cuMemsetD8_v2),
  /** cuMemsetD8_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemsetD8_v2_ptds),
  /** cuMemsetD8Async */
  CUDA_ENTRY_ENUM(cuMemsetD8Async),
  /** cuMemsetD8Async_ptsz */
  CUDA_ENTRY_ENUM(cuMemsetD8Async_ptsz),
  /** cuMemsetD2D8_v2 */
  CUDA_ENTRY_ENUM(cuMemsetD2D8_v2),
  /** cuMemsetD2D8_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemsetD2D8_v2_ptds),
  /** cuMemsetD2D8Async */
  CUDA_ENTRY_ENUM(cuMemsetD2D8Async),
  /** cuMemsetD2D8Async_ptsz */
  CUDA_ENTRY_ENUM(cuMemsetD2D8Async_ptsz),
  /** cuFuncSetCacheConfig */
  CUDA_ENTRY_ENUM(cuFuncSetCacheConfig),
  /** cuFuncSetSharedMemConfig */
  CUDA_ENTRY_ENUM(cuFuncSetSharedMemConfig),
  /** cuFuncGetAttribute */
  CUDA_ENTRY_ENUM(cuFuncGetAttribute),
  /** cuArrayCreate_v2 */
  CUDA_ENTRY_ENUM(cuArrayCreate_v2),
  /** cuArrayGetDescriptor_v2 */
  CUDA_ENTRY_ENUM(cuArrayGetDescriptor_v2),
  /** cuArray3DCreate_v2 */
  CUDA_ENTRY_ENUM(cuArray3DCreate_v2),
  /** cuArray3DGetDescriptor_v2 */
  CUDA_ENTRY_ENUM(cuArray3DGetDescriptor_v2),
  /** cuArrayDestroy */
  CUDA_ENTRY_ENUM(cuArrayDestroy),
  /** cuMipmappedArrayCreate */
  CUDA_ENTRY_ENUM(cuMipmappedArrayCreate),
  /** cuMipmappedArrayGetLevel */
  CUDA_ENTRY_ENUM(cuMipmappedArrayGetLevel),
  /** cuMipmappedArrayDestroy */
  CUDA_ENTRY_ENUM(cuMipmappedArrayDestroy),
  /** cuTexRefCreate */
  CUDA_ENTRY_ENUM(cuTexRefCreate),
  /** cuTexRefDestroy */
  CUDA_ENTRY_ENUM(cuTexRefDestroy),
  /** cuTexRefSetArray */
  CUDA_ENTRY_ENUM(cuTexRefSetArray),
  /** cuTexRefSetMipmappedArray */
  CUDA_ENTRY_ENUM(cuTexRefSetMipmappedArray),
  /** cuTexRefSetAddress_v2 */
  CUDA_ENTRY_ENUM(cuTexRefSetAddress_v2),
  /** cuTexRefSetAddress2D_v3 */
  CUDA_ENTRY_ENUM(cuTexRefSetAddress2D_v3),
  /** cuTexRefSetFormat */
  CUDA_ENTRY_ENUM(cuTexRefSetFormat),
  /** cuTexRefSetAddressMode */
  CUDA_ENTRY_ENUM(cuTexRefSetAddressMode),
  /** cuTexRefSetFilterMode */
  CUDA_ENTRY_ENUM(cuTexRefSetFilterMode),
  /** cuTexRefSetMipmapFilterMode */
  CUDA_ENTRY_ENUM(cuTexRefSetMipmapFilterMode),
  /** cuTexRefSetMipmapLevelBias */
  CUDA_ENTRY_ENUM(cuTexRefSetMipmapLevelBias),
  /** cuTexRefSetMipmapLevelClamp */
  CUDA_ENTRY_ENUM(cuTexRefSetMipmapLevelClamp),
  /** cuTexRefSetMaxAnisotropy */
  CUDA_ENTRY_ENUM(cuTexRefSetMaxAnisotropy),
  /** cuTexRefSetFlags */
  CUDA_ENTRY_ENUM(cuTexRefSetFlags),
  /** cuTexRefSetBorderColor */
  CUDA_ENTRY_ENUM(cuTexRefSetBorderColor),
  /** cuTexRefGetBorderColor */
  CUDA_ENTRY_ENUM(cuTexRefGetBorderColor),
  /** cuSurfRefSetArray */
  CUDA_ENTRY_ENUM(cuSurfRefSetArray),
  /** cuTexObjectCreate */
  CUDA_ENTRY_ENUM(cuTexObjectCreate),
  /** cuTexObjectDestroy */
  CUDA_ENTRY_ENUM(cuTexObjectDestroy),
  /** cuTexObjectGetResourceDesc */
  CUDA_ENTRY_ENUM(cuTexObjectGetResourceDesc),
  /** cuTexObjectGetTextureDesc */
  CUDA_ENTRY_ENUM(cuTexObjectGetTextureDesc),
  /** cuTexObjectGetResourceViewDesc */
  CUDA_ENTRY_ENUM(cuTexObjectGetResourceViewDesc),
  /** cuSurfObjectCreate */
  CUDA_ENTRY_ENUM(cuSurfObjectCreate),
  /** cuSurfObjectDestroy */
  CUDA_ENTRY_ENUM(cuSurfObjectDestroy),
  /** cuSurfObjectGetResourceDesc */
  CUDA_ENTRY_ENUM(cuSurfObjectGetResourceDesc),
  /** cuLaunchKernel */
  CUDA_ENTRY_ENUM(cuLaunchKernel),
  /** cuLaunchKernel_ptsz */
  CUDA_ENTRY_ENUM(cuLaunchKernel_ptsz),
  /** cuEventCreate */
  CUDA_ENTRY_ENUM(cuEventCreate),
  /** cuEventRecord */
  CUDA_ENTRY_ENUM(cuEventRecord),
  /** cuEventRecord_ptsz */
  CUDA_ENTRY_ENUM(cuEventRecord_ptsz),
  /** cuEventQuery */
  CUDA_ENTRY_ENUM(cuEventQuery),
  /** cuEventSynchronize */
  CUDA_ENTRY_ENUM(cuEventSynchronize),
  /** cuEventDestroy_v2 */
  CUDA_ENTRY_ENUM(cuEventDestroy_v2),
  /** cuEventElapsedTime */
  CUDA_ENTRY_ENUM(cuEventElapsedTime),
  /** cuStreamWaitValue32 */
  CUDA_ENTRY_ENUM(cuStreamWaitValue32),
  /** cuStreamWaitValue32_ptsz */
  CUDA_ENTRY_ENUM(cuStreamWaitValue32_ptsz),
  /** cuStreamWriteValue32 */
  CUDA_ENTRY_ENUM(cuStreamWriteValue32),
  /** cuStreamWriteValue32_ptsz */
  CUDA_ENTRY_ENUM(cuStreamWriteValue32_ptsz),
  /** cuStreamBatchMemOp */
  CUDA_ENTRY_ENUM(cuStreamBatchMemOp),
  /** cuStreamBatchMemOp_ptsz */
  CUDA_ENTRY_ENUM(cuStreamBatchMemOp_ptsz),
  /** cuStreamCreate */
  CUDA_ENTRY_ENUM(cuStreamCreate),
  /** cuStreamCreateWithPriority */
  CUDA_ENTRY_ENUM(cuStreamCreateWithPriority),
  /** cuStreamGetPriority */
  CUDA_ENTRY_ENUM(cuStreamGetPriority),
  /** cuStreamGetPriority_ptsz */
  CUDA_ENTRY_ENUM(cuStreamGetPriority_ptsz),
  /** cuStreamGetFlags */
  CUDA_ENTRY_ENUM(cuStreamGetFlags),
  /** cuStreamGetFlags_ptsz */
  CUDA_ENTRY_ENUM(cuStreamGetFlags_ptsz),
  /** cuStreamDestroy_v2 */
  CUDA_ENTRY_ENUM(cuStreamDestroy_v2),
  /** cuStreamWaitEvent */
  CUDA_ENTRY_ENUM(cuStreamWaitEvent),
  /** cuStreamWaitEvent_ptsz */
  CUDA_ENTRY_ENUM(cuStreamWaitEvent_ptsz),
  /** cuStreamAddCallback */
  CUDA_ENTRY_ENUM(cuStreamAddCallback),
  /** cuStreamAddCallback_ptsz */
  CUDA_ENTRY_ENUM(cuStreamAddCallback_ptsz),
  /** cuStreamSynchronize */
  CUDA_ENTRY_ENUM(cuStreamSynchronize),
  /** cuStreamSynchronize_ptsz */
  CUDA_ENTRY_ENUM(cuStreamSynchronize_ptsz),
  /** cuStreamQuery */
  CUDA_ENTRY_ENUM(cuStreamQuery),
  /** cuStreamQuery_ptsz */
  CUDA_ENTRY_ENUM(cuStreamQuery_ptsz),
  /** cuStreamAttachMemAsync */
  CUDA_ENTRY_ENUM(cuStreamAttachMemAsync),
  /** cuStreamAttachMemAsync_ptsz */
  CUDA_ENTRY_ENUM(cuStreamAttachMemAsync_ptsz),
  /** cuDeviceCanAccessPeer */
  CUDA_ENTRY_ENUM(cuDeviceCanAccessPeer),
  /** cuCtxEnablePeerAccess */
  CUDA_ENTRY_ENUM(cuCtxEnablePeerAccess),
  /** cuCtxDisablePeerAccess */
  CUDA_ENTRY_ENUM(cuCtxDisablePeerAccess),
  /** cuIpcGetEventHandle */
  CUDA_ENTRY_ENUM(cuIpcGetEventHandle),
  /** cuIpcOpenEventHandle */
  CUDA_ENTRY_ENUM(cuIpcOpenEventHandle),
  /** cuIpcGetMemHandle */
  CUDA_ENTRY_ENUM(cuIpcGetMemHandle),
  /** cuIpcOpenMemHandle */
  CUDA_ENTRY_ENUM(cuIpcOpenMemHandle),
  /** cuIpcCloseMemHandle */
  CUDA_ENTRY_ENUM(cuIpcCloseMemHandle),
  /** cuGLCtxCreate_v2 */
  CUDA_ENTRY_ENUM(cuGLCtxCreate_v2),
  /** cuGLInit */
  CUDA_ENTRY_ENUM(cuGLInit),
  /** cuGLGetDevices */
  CUDA_ENTRY_ENUM(cuGLGetDevices),
  /** cuGLRegisterBufferObject */
  CUDA_ENTRY_ENUM(cuGLRegisterBufferObject),
  /** cuGLMapBufferObject_v2 */
  CUDA_ENTRY_ENUM(cuGLMapBufferObject_v2),
  /** cuGLMapBufferObject_v2_ptds */
  CUDA_ENTRY_ENUM(cuGLMapBufferObject_v2_ptds),
  /** cuGLMapBufferObjectAsync_v2 */
  CUDA_ENTRY_ENUM(cuGLMapBufferObjectAsync_v2),
  /** cuGLMapBufferObjectAsync_v2_ptsz */
  CUDA_ENTRY_ENUM(cuGLMapBufferObjectAsync_v2_ptsz),
  /** cuGLUnmapBufferObject */
  CUDA_ENTRY_ENUM(cuGLUnmapBufferObject),
  /** cuGLUnmapBufferObjectAsync */
  CUDA_ENTRY_ENUM(cuGLUnmapBufferObjectAsync),
  /** cuGLUnregisterBufferObject */
  CUDA_ENTRY_ENUM(cuGLUnregisterBufferObject),
  /** cuGLSetBufferObjectMapFlags */
  CUDA_ENTRY_ENUM(cuGLSetBufferObjectMapFlags),
  /** cuGraphicsGLRegisterImage */
  CUDA_ENTRY_ENUM(cuGraphicsGLRegisterImage),
  /** cuGraphicsGLRegisterBuffer */
  CUDA_ENTRY_ENUM(cuGraphicsGLRegisterBuffer),
  /** cuGraphicsUnregisterResource */
  CUDA_ENTRY_ENUM(cuGraphicsUnregisterResource),
  /** cuGraphicsMapResources */
  CUDA_ENTRY_ENUM(cuGraphicsMapResources),
  /** cuGraphicsMapResources_ptsz */
  CUDA_ENTRY_ENUM(cuGraphicsMapResources_ptsz),
  /** cuGraphicsUnmapResources */
  CUDA_ENTRY_ENUM(cuGraphicsUnmapResources),
  /** cuGraphicsUnmapResources_ptsz */
  CUDA_ENTRY_ENUM(cuGraphicsUnmapResources_ptsz),
  /** cuGraphicsResourceSetMapFlags_v2 */
  CUDA_ENTRY_ENUM(cuGraphicsResourceSetMapFlags_v2),
  /** cuGraphicsSubResourceGetMappedArray */
  CUDA_ENTRY_ENUM(cuGraphicsSubResourceGetMappedArray),
  /** cuGraphicsResourceGetMappedMipmappedArray */
  CUDA_ENTRY_ENUM(cuGraphicsResourceGetMappedMipmappedArray),
  /** cuGraphicsResourceGetMappedPointer_v2 */
  CUDA_ENTRY_ENUM(cuGraphicsResourceGetMappedPointer_v2),
  /** cuProfilerInitialize */
  CUDA_ENTRY_ENUM(cuProfilerInitialize),
  /** cuProfilerStart */
  CUDA_ENTRY_ENUM(cuProfilerStart),
  /** cuProfilerStop */
  CUDA_ENTRY_ENUM(cuProfilerStop),
  /** cuVDPAUGetDevice */
  CUDA_ENTRY_ENUM(cuVDPAUGetDevice),
  /** cuVDPAUCtxCreate_v2 */
  CUDA_ENTRY_ENUM(cuVDPAUCtxCreate_v2),
  /** cuGraphicsVDPAURegisterVideoSurface */
  CUDA_ENTRY_ENUM(cuGraphicsVDPAURegisterVideoSurface),
  /** cuGraphicsVDPAURegisterOutputSurface */
  CUDA_ENTRY_ENUM(cuGraphicsVDPAURegisterOutputSurface),
  /** cuGetExportTable */
  CUDA_ENTRY_ENUM(cuGetExportTable),
  /** cuOccupancyMaxActiveBlocksPerMultiprocessor */
  CUDA_ENTRY_ENUM(cuOccupancyMaxActiveBlocksPerMultiprocessor),
  /** cuMemAdvise */
  CUDA_ENTRY_ENUM(cuMemAdvise),
  /** cuMemPrefetchAsync */
  CUDA_ENTRY_ENUM(cuMemPrefetchAsync),
  /** cuMemPrefetchAsync_ptsz */
  CUDA_ENTRY_ENUM(cuMemPrefetchAsync_ptsz),
  /** cuMemRangeGetAttribute */
  CUDA_ENTRY_ENUM(cuMemRangeGetAttribute),
  /** cuMemRangeGetAttributes */
  CUDA_ENTRY_ENUM(cuMemRangeGetAttributes),
  /** cuGetErrorString */
  CUDA_ENTRY_ENUM(cuGetErrorString),
  /** cuGetErrorName */
  CUDA_ENTRY_ENUM(cuGetErrorName),
  /** cuArray3DCreate */
  CUDA_ENTRY_ENUM(cuArray3DCreate),
  /** cuArray3DGetDescriptor */
  CUDA_ENTRY_ENUM(cuArray3DGetDescriptor),
  /** cuArrayCreate */
  CUDA_ENTRY_ENUM(cuArrayCreate),
  /** cuArrayGetDescriptor */
  CUDA_ENTRY_ENUM(cuArrayGetDescriptor),
  /** cuCtxAttach */
  CUDA_ENTRY_ENUM(cuCtxAttach),
  /** cuCtxCreate */
  CUDA_ENTRY_ENUM(cuCtxCreate),
  /** cuCtxDestroy */
  CUDA_ENTRY_ENUM(cuCtxDestroy),
  /** cuCtxDestroy_v2 */
  CUDA_ENTRY_ENUM(cuCtxDestroy_v2),
  /** cuCtxPopCurrent */
  CUDA_ENTRY_ENUM(cuCtxPopCurrent),
  /** cuCtxPopCurrent_v2 */
  CUDA_ENTRY_ENUM(cuCtxPopCurrent_v2),
  /** cuCtxPushCurrent */
  CUDA_ENTRY_ENUM(cuCtxPushCurrent),
  /** cuCtxPushCurrent_v2 */
  CUDA_ENTRY_ENUM(cuCtxPushCurrent_v2),
  /** cudbgApiAttach */
  CUDA_ENTRY_ENUM(cudbgApiAttach),
  /** cudbgApiDetach */
  CUDA_ENTRY_ENUM(cudbgApiDetach),
  /** cudbgApiInit */
  CUDA_ENTRY_ENUM(cudbgApiInit),
  /** cudbgGetAPI */
  CUDA_ENTRY_ENUM(cudbgGetAPI),
  /** cudbgGetAPIVersion */
  CUDA_ENTRY_ENUM(cudbgGetAPIVersion),
  /** cudbgMain */
  CUDA_ENTRY_ENUM(cudbgMain),
  /** cudbgReportDriverApiError */
  CUDA_ENTRY_ENUM(cudbgReportDriverApiError),
  /** cudbgReportDriverInternalError */
  CUDA_ENTRY_ENUM(cudbgReportDriverInternalError),
  /** cuDeviceComputeCapability */
  CUDA_ENTRY_ENUM(cuDeviceComputeCapability),
  /** cuDeviceGetProperties */
  CUDA_ENTRY_ENUM(cuDeviceGetProperties),
  /** cuDeviceTotalMem */
  CUDA_ENTRY_ENUM(cuDeviceTotalMem),
  /** cuEGLInit */
  CUDA_ENTRY_ENUM(cuEGLInit),
  /** cuEGLStreamConsumerAcquireFrame */
  CUDA_ENTRY_ENUM(cuEGLStreamConsumerAcquireFrame),
  /** cuEGLStreamConsumerConnect */
  CUDA_ENTRY_ENUM(cuEGLStreamConsumerConnect),
  /** cuEGLStreamConsumerConnectWithFlags */
  CUDA_ENTRY_ENUM(cuEGLStreamConsumerConnectWithFlags),
  /** cuEGLStreamConsumerDisconnect */
  CUDA_ENTRY_ENUM(cuEGLStreamConsumerDisconnect),
  /** cuEGLStreamConsumerReleaseFrame */
  CUDA_ENTRY_ENUM(cuEGLStreamConsumerReleaseFrame),
  /** cuEGLStreamProducerConnect */
  CUDA_ENTRY_ENUM(cuEGLStreamProducerConnect),
  /** cuEGLStreamProducerDisconnect */
  CUDA_ENTRY_ENUM(cuEGLStreamProducerDisconnect),
  /** cuEGLStreamProducerPresentFrame */
  CUDA_ENTRY_ENUM(cuEGLStreamProducerPresentFrame),
  /** cuEGLStreamProducerReturnFrame */
  CUDA_ENTRY_ENUM(cuEGLStreamProducerReturnFrame),
  /** cuEventDestroy */
  CUDA_ENTRY_ENUM(cuEventDestroy),
  /** cuFuncSetAttribute */
  CUDA_ENTRY_ENUM(cuFuncSetAttribute),
  /** cuFuncSetBlockShape */
  CUDA_ENTRY_ENUM(cuFuncSetBlockShape),
  /** cuFuncSetSharedSize */
  CUDA_ENTRY_ENUM(cuFuncSetSharedSize),
  /** cuGLCtxCreate */
  CUDA_ENTRY_ENUM(cuGLCtxCreate),
  /** cuGLGetDevices_v2 */
  CUDA_ENTRY_ENUM(cuGLGetDevices_v2),
  /** cuGLMapBufferObject */
  CUDA_ENTRY_ENUM(cuGLMapBufferObject),
  /** cuGLMapBufferObjectAsync */
  CUDA_ENTRY_ENUM(cuGLMapBufferObjectAsync),
  /** cuGraphicsEGLRegisterImage */
  CUDA_ENTRY_ENUM(cuGraphicsEGLRegisterImage),
  /** cuGraphicsResourceGetMappedEglFrame */
  CUDA_ENTRY_ENUM(cuGraphicsResourceGetMappedEglFrame),
  /** cuGraphicsResourceGetMappedPointer */
  CUDA_ENTRY_ENUM(cuGraphicsResourceGetMappedPointer),
  /** cuGraphicsResourceSetMapFlags */
  CUDA_ENTRY_ENUM(cuGraphicsResourceSetMapFlags),
  /** cuLaunch */
  CUDA_ENTRY_ENUM(cuLaunch),
  /** cuLaunchCooperativeKernel */
  CUDA_ENTRY_ENUM(cuLaunchCooperativeKernel),
  /** cuLaunchCooperativeKernelMultiDevice */
  CUDA_ENTRY_ENUM(cuLaunchCooperativeKernelMultiDevice),
  /** cuLaunchCooperativeKernel_ptsz */
  CUDA_ENTRY_ENUM(cuLaunchCooperativeKernel_ptsz),
  /** cuLaunchGrid */
  CUDA_ENTRY_ENUM(cuLaunchGrid),
  /** cuLaunchGridAsync */
  CUDA_ENTRY_ENUM(cuLaunchGridAsync),
  /** cuLinkAddData_v2 */
  CUDA_ENTRY_ENUM(cuLinkAddData_v2),
  /** cuLinkAddFile_v2 */
  CUDA_ENTRY_ENUM(cuLinkAddFile_v2),
  /** cuLinkCreate_v2 */
  CUDA_ENTRY_ENUM(cuLinkCreate_v2),
  /** cuMemAlloc */
  CUDA_ENTRY_ENUM(cuMemAlloc),
  /** cuMemAllocHost */
  CUDA_ENTRY_ENUM(cuMemAllocHost),
  /** cuMemAllocHost_v2 */
  CUDA_ENTRY_ENUM(cuMemAllocHost_v2),
  /** cuMemAllocPitch */
  CUDA_ENTRY_ENUM(cuMemAllocPitch),
  /** cuMemcpy2D */
  CUDA_ENTRY_ENUM(cuMemcpy2D),
  /** cuMemcpy2DAsync */
  CUDA_ENTRY_ENUM(cuMemcpy2DAsync),
  /** cuMemcpy2DUnaligned */
  CUDA_ENTRY_ENUM(cuMemcpy2DUnaligned),
  /** cuMemcpy2D_v2 */
  CUDA_ENTRY_ENUM(cuMemcpy2D_v2),
  /** cuMemcpy2D_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemcpy2D_v2_ptds),
  /** cuMemcpy3D */
  CUDA_ENTRY_ENUM(cuMemcpy3D),
  /** cuMemcpy3DAsync */
  CUDA_ENTRY_ENUM(cuMemcpy3DAsync),
  /** cuMemcpyAtoA */
  CUDA_ENTRY_ENUM(cuMemcpyAtoA),
  /** cuMemcpyAtoA_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyAtoA_v2),
  /** cuMemcpyAtoA_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemcpyAtoA_v2_ptds),
  /** cuMemcpyAtoD */
  CUDA_ENTRY_ENUM(cuMemcpyAtoD),
  /** cuMemcpyAtoD_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyAtoD_v2),
  /** cuMemcpyAtoD_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemcpyAtoD_v2_ptds),
  /** cuMemcpyAtoH */
  CUDA_ENTRY_ENUM(cuMemcpyAtoH),
  /** cuMemcpyAtoHAsync */
  CUDA_ENTRY_ENUM(cuMemcpyAtoHAsync),
  /** cuMemcpyAtoHAsync_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyAtoHAsync_v2),
  /** cuMemcpyAtoHAsync_v2_ptsz */
  CUDA_ENTRY_ENUM(cuMemcpyAtoHAsync_v2_ptsz),
  /** cuMemcpyAtoH_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyAtoH_v2),
  /** cuMemcpyAtoH_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemcpyAtoH_v2_ptds),
  /** cuMemcpyDtoA */
  CUDA_ENTRY_ENUM(cuMemcpyDtoA),
  /** cuMemcpyDtoA_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyDtoA_v2),
  /** cuMemcpyDtoA_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemcpyDtoA_v2_ptds),
  /** cuMemcpyDtoD */
  CUDA_ENTRY_ENUM(cuMemcpyDtoD),
  /** cuMemcpyDtoDAsync */
  CUDA_ENTRY_ENUM(cuMemcpyDtoDAsync),
  /** cuMemcpyDtoH */
  CUDA_ENTRY_ENUM(cuMemcpyDtoH),
  /** cuMemcpyDtoHAsync */
  CUDA_ENTRY_ENUM(cuMemcpyDtoHAsync),
  /** cuMemcpyHtoA */
  CUDA_ENTRY_ENUM(cuMemcpyHtoA),
  /** cuMemcpyHtoAAsync */
  CUDA_ENTRY_ENUM(cuMemcpyHtoAAsync),
  /** cuMemcpyHtoAAsync_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyHtoAAsync_v2),
  /** cuMemcpyHtoAAsync_v2_ptsz */
  CUDA_ENTRY_ENUM(cuMemcpyHtoAAsync_v2_ptsz),
  /** cuMemcpyHtoA_v2 */
  CUDA_ENTRY_ENUM(cuMemcpyHtoA_v2),
  /** cuMemcpyHtoA_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemcpyHtoA_v2_ptds),
  /** cuMemcpyHtoD */
  CUDA_ENTRY_ENUM(cuMemcpyHtoD),
  /** cuMemcpyHtoDAsync */
  CUDA_ENTRY_ENUM(cuMemcpyHtoDAsync),
  /** cuMemFree */
  CUDA_ENTRY_ENUM(cuMemFree),
  /** cuMemGetAddressRange */
  CUDA_ENTRY_ENUM(cuMemGetAddressRange),
  // Deprecated
  // CUDA_ENTRY_ENUM(cuMemGetAttribute),
  // CUDA_ENTRY_ENUM(cuMemGetAttribute_v2),
  /** cuMemGetInfo */
  CUDA_ENTRY_ENUM(cuMemGetInfo),
  /** cuMemHostGetDevicePointer */
  CUDA_ENTRY_ENUM(cuMemHostGetDevicePointer),
  /** cuMemHostRegister */
  CUDA_ENTRY_ENUM(cuMemHostRegister),
  /** cuMemsetD16 */
  CUDA_ENTRY_ENUM(cuMemsetD16),
  /** cuMemsetD16Async */
  CUDA_ENTRY_ENUM(cuMemsetD16Async),
  /** cuMemsetD16Async_ptsz */
  CUDA_ENTRY_ENUM(cuMemsetD16Async_ptsz),
  /** cuMemsetD16_v2 */
  CUDA_ENTRY_ENUM(cuMemsetD16_v2),
  /** cuMemsetD16_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemsetD16_v2_ptds),
  /** cuMemsetD2D16 */
  CUDA_ENTRY_ENUM(cuMemsetD2D16),
  /** cuMemsetD2D16Async */
  CUDA_ENTRY_ENUM(cuMemsetD2D16Async),
  /** cuMemsetD2D16Async_ptsz */
  CUDA_ENTRY_ENUM(cuMemsetD2D16Async_ptsz),
  /** cuMemsetD2D16_v2 */
  CUDA_ENTRY_ENUM(cuMemsetD2D16_v2),
  /** cuMemsetD2D16_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemsetD2D16_v2_ptds),
  /** cuMemsetD2D32 */
  CUDA_ENTRY_ENUM(cuMemsetD2D32),
  /** cuMemsetD2D32Async */
  CUDA_ENTRY_ENUM(cuMemsetD2D32Async),
  /** cuMemsetD2D32Async_ptsz */
  CUDA_ENTRY_ENUM(cuMemsetD2D32Async_ptsz),
  /** cuMemsetD2D32_v2 */
  CUDA_ENTRY_ENUM(cuMemsetD2D32_v2),
  /** cuMemsetD2D32_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemsetD2D32_v2_ptds),
  /** cuMemsetD2D8 */
  CUDA_ENTRY_ENUM(cuMemsetD2D8),
  /** cuMemsetD32 */
  CUDA_ENTRY_ENUM(cuMemsetD32),
  /** cuMemsetD32Async */
  CUDA_ENTRY_ENUM(cuMemsetD32Async),
  /** cuMemsetD32Async_ptsz */
  CUDA_ENTRY_ENUM(cuMemsetD32Async_ptsz),
  /** cuMemsetD32_v2 */
  CUDA_ENTRY_ENUM(cuMemsetD32_v2),
  /** cuMemsetD32_v2_ptds */
  CUDA_ENTRY_ENUM(cuMemsetD32_v2_ptds),
  /** cuMemsetD8 */
  CUDA_ENTRY_ENUM(cuMemsetD8),
  /** cuModuleGetGlobal */
  CUDA_ENTRY_ENUM(cuModuleGetGlobal),
  /** cuModuleLoadDataEx */
  CUDA_ENTRY_ENUM(cuModuleLoadDataEx),
  /** cuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags */
  CUDA_ENTRY_ENUM(cuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags),
  /** cuOccupancyMaxPotentialBlockSize */
  CUDA_ENTRY_ENUM(cuOccupancyMaxPotentialBlockSize),
  /** cuOccupancyMaxPotentialBlockSizeWithFlags */
  CUDA_ENTRY_ENUM(cuOccupancyMaxPotentialBlockSizeWithFlags),
  /** cuParamSetf */
  CUDA_ENTRY_ENUM(cuParamSetf),
  /** cuParamSeti */
  CUDA_ENTRY_ENUM(cuParamSeti),
  /** cuParamSetSize */
  CUDA_ENTRY_ENUM(cuParamSetSize),
  /** cuParamSetTexRef */
  CUDA_ENTRY_ENUM(cuParamSetTexRef),
  /** cuParamSetv */
  CUDA_ENTRY_ENUM(cuParamSetv),
  /** cuPointerSetAttribute */
  CUDA_ENTRY_ENUM(cuPointerSetAttribute),
  /** cuStreamDestroy */
  CUDA_ENTRY_ENUM(cuStreamDestroy),
  /** cuStreamWaitValue64 */
  CUDA_ENTRY_ENUM(cuStreamWaitValue64),
  /** cuStreamWaitValue64_ptsz */
  CUDA_ENTRY_ENUM(cuStreamWaitValue64_ptsz),
  /** cuStreamWriteValue64 */
  CUDA_ENTRY_ENUM(cuStreamWriteValue64),
  /** cuStreamWriteValue64_ptsz */
  CUDA_ENTRY_ENUM(cuStreamWriteValue64_ptsz),
  /** cuSurfRefGetArray */
  CUDA_ENTRY_ENUM(cuSurfRefGetArray),
  /** cuTexRefGetAddress */
  CUDA_ENTRY_ENUM(cuTexRefGetAddress),
  /** cuTexRefGetAddressMode */
  CUDA_ENTRY_ENUM(cuTexRefGetAddressMode),
  /** cuTexRefGetAddress_v2 */
  CUDA_ENTRY_ENUM(cuTexRefGetAddress_v2),
  /** cuTexRefGetArray */
  CUDA_ENTRY_ENUM(cuTexRefGetArray),
  /** cuTexRefGetFilterMode */
  CUDA_ENTRY_ENUM(cuTexRefGetFilterMode),
  /** cuTexRefGetFlags */
  CUDA_ENTRY_ENUM(cuTexRefGetFlags),
  /** cuTexRefGetFormat */
  CUDA_ENTRY_ENUM(cuTexRefGetFormat),
  /** cuTexRefGetMaxAnisotropy */
  CUDA_ENTRY_ENUM(cuTexRefGetMaxAnisotropy),
  /** cuTexRefGetMipmapFilterMode */
  CUDA_ENTRY_ENUM(cuTexRefGetMipmapFilterMode),
  /** cuTexRefGetMipmapLevelBias */
  CUDA_ENTRY_ENUM(cuTexRefGetMipmapLevelBias),
  /** cuTexRefGetMipmapLevelClamp */
  CUDA_ENTRY_ENUM(cuTexRefGetMipmapLevelClamp),
  /** cuTexRefGetMipmappedArray */
  CUDA_ENTRY_ENUM(cuTexRefGetMipmappedArray),
  /** cuTexRefSetAddress */
  CUDA_ENTRY_ENUM(cuTexRefSetAddress),
  /** cuTexRefSetAddress2D */
  CUDA_ENTRY_ENUM(cuTexRefSetAddress2D),
  /** cuTexRefSetAddress2D_v2 */
  CUDA_ENTRY_ENUM(cuTexRefSetAddress2D_v2),
  /** cuVDPAUCtxCreate */
  CUDA_ENTRY_ENUM(cuVDPAUCtxCreate),
  /** cuEGLApiInit */
  CUDA_ENTRY_ENUM(cuEGLApiInit),
  /** cuDestroyExternalMemory */
  CUDA_ENTRY_ENUM(cuDestroyExternalMemory),
  /** cuDestroyExternalSemaphore */
  CUDA_ENTRY_ENUM(cuDestroyExternalSemaphore),
  /** cuDeviceGetUuid */
  CUDA_ENTRY_ENUM(cuDeviceGetUuid),
  /** cuExternalMemoryGetMappedBuffer */
  CUDA_ENTRY_ENUM(cuExternalMemoryGetMappedBuffer),
  /** cuExternalMemoryGetMappedMipmappedArray */
  CUDA_ENTRY_ENUM(cuExternalMemoryGetMappedMipmappedArray),
  /** cuGraphAddChildGraphNode */
  CUDA_ENTRY_ENUM(cuGraphAddChildGraphNode),
  /** cuGraphAddDependencies */
  CUDA_ENTRY_ENUM(cuGraphAddDependencies),
  /** cuGraphAddEmptyNode */
  CUDA_ENTRY_ENUM(cuGraphAddEmptyNode),
  /** cuGraphAddHostNode */
  CUDA_ENTRY_ENUM(cuGraphAddHostNode),
  /** cuGraphAddKernelNode */
  CUDA_ENTRY_ENUM(cuGraphAddKernelNode),
  /** cuGraphAddMemcpyNode */
  CUDA_ENTRY_ENUM(cuGraphAddMemcpyNode),
  /** cuGraphAddMemsetNode */
  CUDA_ENTRY_ENUM(cuGraphAddMemsetNode),
  /** cuGraphChildGraphNodeGetGraph */
  CUDA_ENTRY_ENUM(cuGraphChildGraphNodeGetGraph),
  /** cuGraphClone */
  CUDA_ENTRY_ENUM(cuGraphClone),
  /** cuGraphCreate */
  CUDA_ENTRY_ENUM(cuGraphCreate),
  /** cuGraphDestroy */
  CUDA_ENTRY_ENUM(cuGraphDestroy),
  /** cuGraphDestroyNode */
  CUDA_ENTRY_ENUM(cuGraphDestroyNode),
  /** cuGraphExecDestroy */
  CUDA_ENTRY_ENUM(cuGraphExecDestroy),
  /** cuGraphGetEdges */
  CUDA_ENTRY_ENUM(cuGraphGetEdges),
  /** cuGraphGetNodes */
  CUDA_ENTRY_ENUM(cuGraphGetNodes),
  /** cuGraphGetRootNodes */
  CUDA_ENTRY_ENUM(cuGraphGetRootNodes),
  /** cuGraphHostNodeGetParams */
  CUDA_ENTRY_ENUM(cuGraphHostNodeGetParams),
  /** cuGraphHostNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphHostNodeSetParams),
  /** cuGraphInstantiate */
  CUDA_ENTRY_ENUM(cuGraphInstantiate),
  /** cuGraphKernelNodeGetParams */
  CUDA_ENTRY_ENUM(cuGraphKernelNodeGetParams),
  /** cuGraphKernelNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphKernelNodeSetParams),
  /** cuGraphLaunch */
  CUDA_ENTRY_ENUM(cuGraphLaunch),
  /** cuGraphLaunch_ptsz */
  CUDA_ENTRY_ENUM(cuGraphLaunch_ptsz),
  /** cuGraphMemcpyNodeGetParams */
  CUDA_ENTRY_ENUM(cuGraphMemcpyNodeGetParams),
  /** cuGraphMemcpyNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphMemcpyNodeSetParams),
  /** cuGraphMemsetNodeGetParams */
  CUDA_ENTRY_ENUM(cuGraphMemsetNodeGetParams),
  /** cuGraphMemsetNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphMemsetNodeSetParams),
  /** cuGraphNodeFindInClone */
  CUDA_ENTRY_ENUM(cuGraphNodeFindInClone),
  /** cuGraphNodeGetDependencies */
  CUDA_ENTRY_ENUM(cuGraphNodeGetDependencies),
  /** cuGraphNodeGetDependentNodes */
  CUDA_ENTRY_ENUM(cuGraphNodeGetDependentNodes),
  /** cuGraphNodeGetType */
  CUDA_ENTRY_ENUM(cuGraphNodeGetType),
  /** cuGraphRemoveDependencies */
  CUDA_ENTRY_ENUM(cuGraphRemoveDependencies),
  /** cuImportExternalMemory */
  CUDA_ENTRY_ENUM(cuImportExternalMemory),
  /** cuImportExternalSemaphore */
  CUDA_ENTRY_ENUM(cuImportExternalSemaphore),
  /** cuLaunchHostFunc */
  CUDA_ENTRY_ENUM(cuLaunchHostFunc),
  /** cuLaunchHostFunc_ptsz */
  CUDA_ENTRY_ENUM(cuLaunchHostFunc_ptsz),
  /** cuSignalExternalSemaphoresAsync */
  CUDA_ENTRY_ENUM(cuSignalExternalSemaphoresAsync),
  /** cuSignalExternalSemaphoresAsync_ptsz */
  CUDA_ENTRY_ENUM(cuSignalExternalSemaphoresAsync_ptsz),
  /** cuStreamBeginCapture */
  CUDA_ENTRY_ENUM(cuStreamBeginCapture),
  /** cuStreamBeginCapture_ptsz */
  CUDA_ENTRY_ENUM(cuStreamBeginCapture_ptsz),
  /** cuStreamEndCapture */
  CUDA_ENTRY_ENUM(cuStreamEndCapture),
  /** cuStreamEndCapture_ptsz */
  CUDA_ENTRY_ENUM(cuStreamEndCapture_ptsz),
  /** cuStreamGetCtx */
  CUDA_ENTRY_ENUM(cuStreamGetCtx),
  /** cuStreamGetCtx_ptsz */
  CUDA_ENTRY_ENUM(cuStreamGetCtx_ptsz),
  /** cuStreamIsCapturing */
  CUDA_ENTRY_ENUM(cuStreamIsCapturing),
  /** cuStreamIsCapturing_ptsz */
  CUDA_ENTRY_ENUM(cuStreamIsCapturing_ptsz),
  /** cuWaitExternalSemaphoresAsync */
  CUDA_ENTRY_ENUM(cuWaitExternalSemaphoresAsync),
  /** cuWaitExternalSemaphoresAsync_ptsz */
  CUDA_ENTRY_ENUM(cuWaitExternalSemaphoresAsync_ptsz),
  /** cuGraphExecKernelNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphExecKernelNodeSetParams),
  /** cuStreamBeginCapture_v2 */
  CUDA_ENTRY_ENUM(cuStreamBeginCapture_v2),
  /** cuStreamBeginCapture_v2_ptsz */
  CUDA_ENTRY_ENUM(cuStreamBeginCapture_v2_ptsz),
  /** cuStreamGetCaptureInfo */
  CUDA_ENTRY_ENUM(cuStreamGetCaptureInfo),
  /** cuStreamGetCaptureInfo_ptsz */
  CUDA_ENTRY_ENUM(cuStreamGetCaptureInfo_ptsz),
  /** cuThreadExchangeStreamCaptureMode */
  CUDA_ENTRY_ENUM(cuThreadExchangeStreamCaptureMode),
  /** cuDeviceGetNvSciSyncAttributes */
  CUDA_ENTRY_ENUM(cuDeviceGetNvSciSyncAttributes),
  /** cuGraphExecHostNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphExecHostNodeSetParams),
  /** cuGraphExecMemcpyNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphExecMemcpyNodeSetParams),
  /** cuGraphExecMemsetNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphExecMemsetNodeSetParams),
  /** cuGraphExecUpdate */
  CUDA_ENTRY_ENUM(cuGraphExecUpdate),
  /** cuMemAddressFree */
  CUDA_ENTRY_ENUM(cuMemAddressFree),
  /** cuMemAddressReserve */
  CUDA_ENTRY_ENUM(cuMemAddressReserve),
  /** cuMemCreate */
  CUDA_ENTRY_ENUM(cuMemCreate),
  /** cuMemExportToShareableHandle */
  CUDA_ENTRY_ENUM(cuMemExportToShareableHandle),
  /** cuMemGetAccess */
  CUDA_ENTRY_ENUM(cuMemGetAccess),
  /** cuMemGetAllocationGranularity */
  CUDA_ENTRY_ENUM(cuMemGetAllocationGranularity),
  /** cuMemGetAllocationPropertiesFromHandle */
  CUDA_ENTRY_ENUM(cuMemGetAllocationPropertiesFromHandle),
  /** cuMemImportFromShareableHandle */
  CUDA_ENTRY_ENUM(cuMemImportFromShareableHandle),
  /** cuMemMap */
  CUDA_ENTRY_ENUM(cuMemMap),
  /** cuMemRelease */
  CUDA_ENTRY_ENUM(cuMemRelease),
  /** cuMemSetAccess */
  CUDA_ENTRY_ENUM(cuMemSetAccess),
  /** cuMemUnmap */
  CUDA_ENTRY_ENUM(cuMemUnmap),
  /** cuCtxResetPersistingL2Cache */
  CUDA_ENTRY_ENUM(cuCtxResetPersistingL2Cache),
  /** cuDevicePrimaryCtxRelease_v2 */
  CUDA_ENTRY_ENUM(cuDevicePrimaryCtxRelease_v2),
  /** cuDevicePrimaryCtxReset_v2 */
  CUDA_ENTRY_ENUM(cuDevicePrimaryCtxReset_v2),
  /** cuDevicePrimaryCtxSetFlags_v2 */
  CUDA_ENTRY_ENUM(cuDevicePrimaryCtxSetFlags_v2),
  /** cuFuncGetModule */
  CUDA_ENTRY_ENUM(cuFuncGetModule),
  /** cuGraphInstantiate_v2 */
  CUDA_ENTRY_ENUM(cuGraphInstantiate_v2),
  /** cuGraphKernelNodeCopyAttributes */
  CUDA_ENTRY_ENUM(cuGraphKernelNodeCopyAttributes),
  /** cuGraphKernelNodeGetAttribute */
  CUDA_ENTRY_ENUM(cuGraphKernelNodeGetAttribute),
  /** cuGraphKernelNodeSetAttribute */
  CUDA_ENTRY_ENUM(cuGraphKernelNodeSetAttribute),
  /** cuMemRetainAllocationHandle */
  CUDA_ENTRY_ENUM(cuMemRetainAllocationHandle),
  /** cuOccupancyAvailableDynamicSMemPerBlock */
  CUDA_ENTRY_ENUM(cuOccupancyAvailableDynamicSMemPerBlock),
  /** cuStreamCopyAttributes */
  CUDA_ENTRY_ENUM(cuStreamCopyAttributes),
  /** cuStreamCopyAttributes_ptsz */
  CUDA_ENTRY_ENUM(cuStreamCopyAttributes_ptsz),
  /** cuStreamGetAttribute */
  CUDA_ENTRY_ENUM(cuStreamGetAttribute),
  /** cuStreamGetAttribute_ptsz */
  CUDA_ENTRY_ENUM(cuStreamGetAttribute_ptsz),
  /** cuStreamSetAttribute */
  CUDA_ENTRY_ENUM(cuStreamSetAttribute),
  /** cuStreamSetAttribute_ptsz */
  CUDA_ENTRY_ENUM(cuStreamSetAttribute_ptsz),
  /** 11.2 */
  /** cuArrayGetPlane */
  CUDA_ENTRY_ENUM(cuArrayGetPlane),
  /** cuArrayGetSparseProperties */
  CUDA_ENTRY_ENUM(cuArrayGetSparseProperties),
  /** cuDeviceGetDefaultMemPool */
  CUDA_ENTRY_ENUM(cuDeviceGetDefaultMemPool),
  /** cuDeviceGetLuid */
  CUDA_ENTRY_ENUM(cuDeviceGetLuid),
  /** cuDeviceGetMemPool */
  CUDA_ENTRY_ENUM(cuDeviceGetMemPool),
  /** cuDeviceGetTexture1DLinearMaxWidth */
  CUDA_ENTRY_ENUM(cuDeviceGetTexture1DLinearMaxWidth),
  /** cuDeviceSetMemPool */
  CUDA_ENTRY_ENUM(cuDeviceSetMemPool),
  /** cuEventRecordWithFlags */
  CUDA_ENTRY_ENUM(cuEventRecordWithFlags),
  /** cuEventRecordWithFlags_ptsz */
  CUDA_ENTRY_ENUM(cuEventRecordWithFlags_ptsz),
  /** cuGraphAddEventRecordNode */
  CUDA_ENTRY_ENUM(cuGraphAddEventRecordNode),
  /** cuGraphAddEventWaitNode */
  CUDA_ENTRY_ENUM(cuGraphAddEventWaitNode),
  /** cuGraphAddExternalSemaphoresSignalNode */
  CUDA_ENTRY_ENUM(cuGraphAddExternalSemaphoresSignalNode),
  /** cuGraphAddExternalSemaphoresWaitNode */
  CUDA_ENTRY_ENUM(cuGraphAddExternalSemaphoresWaitNode),
  /** cuGraphEventRecordNodeGetEvent */
  CUDA_ENTRY_ENUM(cuGraphEventRecordNodeGetEvent),
  /** cuGraphEventRecordNodeSetEvent */
  CUDA_ENTRY_ENUM(cuGraphEventRecordNodeSetEvent),
  /** cuGraphEventWaitNodeGetEvent */
  CUDA_ENTRY_ENUM(cuGraphEventWaitNodeGetEvent),
  /** cuGraphEventWaitNodeSetEvent */
  CUDA_ENTRY_ENUM(cuGraphEventWaitNodeSetEvent),
  /** cuGraphExecChildGraphNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphExecChildGraphNodeSetParams),
  /** cuGraphExecEventRecordNodeSetEvent */
  CUDA_ENTRY_ENUM(cuGraphExecEventRecordNodeSetEvent),
  /** cuGraphExecEventWaitNodeSetEvent */
  CUDA_ENTRY_ENUM(cuGraphExecEventWaitNodeSetEvent),
  /** cuGraphExecExternalSemaphoresSignalNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphExecExternalSemaphoresSignalNodeSetParams),
  /** cuGraphExecExternalSemaphoresWaitNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphExecExternalSemaphoresWaitNodeSetParams),
  /** cuGraphExternalSemaphoresSignalNodeGetParams */
  CUDA_ENTRY_ENUM(cuGraphExternalSemaphoresSignalNodeGetParams),
  /** cuGraphExternalSemaphoresSignalNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphExternalSemaphoresSignalNodeSetParams),
  /** cuGraphExternalSemaphoresWaitNodeGetParams */
  CUDA_ENTRY_ENUM(cuGraphExternalSemaphoresWaitNodeGetParams),
  /** cuGraphExternalSemaphoresWaitNodeSetParams */
  CUDA_ENTRY_ENUM(cuGraphExternalSemaphoresWaitNodeSetParams),
  /** cuGraphUpload */
  CUDA_ENTRY_ENUM(cuGraphUpload),
  /** cuGraphUpload_ptsz */
  CUDA_ENTRY_ENUM(cuGraphUpload_ptsz),
  /** cuIpcOpenMemHandle_v2 */
  CUDA_ENTRY_ENUM(cuIpcOpenMemHandle_v2),
  /** memory pool should be concerned ? */
  /** cuMemAllocAsync */
  CUDA_ENTRY_ENUM(cuMemAllocAsync),
  /** cuMemAllocAsync_ptsz */
  CUDA_ENTRY_ENUM(cuMemAllocAsync_ptsz),
  /** cuMemAllocFromPoolAsync */
  CUDA_ENTRY_ENUM(cuMemAllocFromPoolAsync),
  /** cuMemAllocFromPoolAsync_ptsz */
  CUDA_ENTRY_ENUM(cuMemAllocFromPoolAsync_ptsz),
  /** cuMemFreeAsync */
  CUDA_ENTRY_ENUM(cuMemFreeAsync),
  /** cuMemFreeAsync_ptsz */
  CUDA_ENTRY_ENUM(cuMemFreeAsync_ptsz),
  /** cuMemMapArrayAsync */
  CUDA_ENTRY_ENUM(cuMemMapArrayAsync),
  /** cuMemMapArrayAsync_ptsz */
  CUDA_ENTRY_ENUM(cuMemMapArrayAsync_ptsz),
  /** cuMemPoolCreate */
  CUDA_ENTRY_ENUM(cuMemPoolCreate),
  /** cuMemPoolDestroy */
  CUDA_ENTRY_ENUM(cuMemPoolDestroy),
  /** cuMemPoolExportPointer */
  CUDA_ENTRY_ENUM(cuMemPoolExportPointer),
  /** cuMemPoolExportToShareableHandle */
  CUDA_ENTRY_ENUM(cuMemPoolExportToShareableHandle),
  /** cuMemPoolGetAccess */
  CUDA_ENTRY_ENUM(cuMemPoolGetAccess),
  /** cuMemPoolGetAttribute */
  CUDA_ENTRY_ENUM(cuMemPoolGetAttribute),
  /** cuMemPoolImportFromShareableHandle */
  CUDA_ENTRY_ENUM(cuMemPoolImportFromShareableHandle),
  /** cuMemPoolImportPointer */
  CUDA_ENTRY_ENUM(cuMemPoolImportPointer),
  /** cuMemPoolSetAccess */
  CUDA_ENTRY_ENUM(cuMemPoolSetAccess),
  /** cuMemPoolSetAttribute */
  CUDA_ENTRY_ENUM(cuMemPoolSetAttribute),
  /** cuMemPoolTrimTo */
  CUDA_ENTRY_ENUM(cuMemPoolTrimTo),
  /** cuMipmappedArrayGetSparseProperties */
  CUDA_ENTRY_ENUM(cuMipmappedArrayGetSparseProperties),
  CUDA_ENTRY_ENUM(cuCtxCreate_v3),
  CUDA_ENTRY_ENUM(cuCtxGetExecAffinity),
  CUDA_ENTRY_ENUM(cuDeviceGetExecAffinitySupport),
  CUDA_ENTRY_ENUM(cuDeviceGetGraphMemAttribute),
  CUDA_ENTRY_ENUM(cuDeviceGetUuid_v2),
  CUDA_ENTRY_ENUM(cuDeviceGraphMemTrim),
  CUDA_ENTRY_ENUM(cuDeviceSetGraphMemAttribute),
  CUDA_ENTRY_ENUM(cuFlushGPUDirectRDMAWrites),
  CUDA_ENTRY_ENUM(cuGetProcAddress),
  CUDA_ENTRY_ENUM(cuGraphAddMemAllocNode),
  CUDA_ENTRY_ENUM(cuGraphAddMemFreeNode),
  CUDA_ENTRY_ENUM(cuGraphDebugDotPrint),
  CUDA_ENTRY_ENUM(cuGraphInstantiateWithFlags),
  CUDA_ENTRY_ENUM(cuGraphMemAllocNodeGetParams),
  CUDA_ENTRY_ENUM(cuGraphMemFreeNodeGetParams),
  CUDA_ENTRY_ENUM(cuGraphReleaseUserObject),
  CUDA_ENTRY_ENUM(cuGraphRetainUserObject),
  CUDA_ENTRY_ENUM(cuStreamGetCaptureInfo_v2),
  CUDA_ENTRY_ENUM(cuStreamGetCaptureInfo_v2_ptsz),
  CUDA_ENTRY_ENUM(cuStreamUpdateCaptureDependencies),
  CUDA_ENTRY_ENUM(cuStreamUpdateCaptureDependencies_ptsz),
  CUDA_ENTRY_ENUM(cuUserObjectCreate),
  CUDA_ENTRY_ENUM(cuUserObjectRelease),
  CUDA_ENTRY_ENUM(cuUserObjectRetain),
  CUDA_ENTRY_ENUM(cuArrayGetMemoryRequirements),
  CUDA_ENTRY_ENUM(cuMipmappedArrayGetMemoryRequirements),
  CUDA_ENTRY_ENUM(cuStreamWaitValue32_v2),
  CUDA_ENTRY_ENUM(cuStreamWaitValue64_v2),
  CUDA_ENTRY_ENUM(cuStreamWriteValue32_v2),
  CUDA_ENTRY_ENUM(cuStreamWriteValue64_v2),
  CUDA_ENTRY_ENUM(cuStreamBatchMemOp_v2),
  CUDA_ENTRY_ENUM(cuGraphAddBatchMemOpNode),
  CUDA_ENTRY_ENUM(cuGraphBatchMemOpNodeGetParams),
  CUDA_ENTRY_ENUM(cuGraphBatchMemOpNodeSetParams),
  CUDA_ENTRY_ENUM(cuGraphExecBatchMemOpNodeSetParams),
  CUDA_ENTRY_ENUM(cuGraphNodeGetEnabled),
  CUDA_ENTRY_ENUM(cuGraphNodeSetEnabled),
  CUDA_ENTRY_ENUM(cuModuleGetLoadingMode),
  CUDA_ENTRY_ENUM(cuMemGetHandleForAddressRange),
  CUDA_ENTRY_END
} cuda_entry_enum_t;



typedef struct {
	char *name;
	void *func_ptr;
}entry_t;

#endif
