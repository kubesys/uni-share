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


#define _GNU_SOURCE
#include "hijack.h"
#include "monitor.h"
#include "control.h"
#include "container.h"

pthread_once_t mlu_lib_ptr_init = PTHREAD_ONCE_INIT;
pthread_once_t mlu_prod = PTHREAD_ONCE_INIT;
pthread_once_t reg = PTHREAD_ONCE_INIT;
cn_uint64_t total_mem = 0;

entry_t CNDrv_entry[] = {
	{.name = "cnGetErrorString"},
	{.name = "cnGetErrorName"},
	{.name = "cnInit"},
	{.name = "cnDriverGetVersion"},
	{.name = "cnGetLibVersion"},
	{.name = "cnGetDriverVersion"},
	{.name = "cnDeviceGet"},
	{.name = "cnDeviceGetCount"},
	{.name = "cnDeviceGetName"},
	{.name = "cnDeviceTotalMem"},
	{.name = "cnDeviceGetAttribute"},
	{.name = "cnDeviceGetByPCIBusId"},
	{.name = "cnDeviceGetPCIBusId"},
	{.name = "cnDeviceGetUuid"},
	{.name = "cnDeviceGetUuidStr"},
	{.name = "cnDeviceGetByUuidStr"},
	{.name = "cnCtxCreate"},
	{.name = "cnCtxDestroy"},
	{.name = "cnCtxGetFlags"},
	{.name = "cnCtxGetApiVersion"},
	{.name = "cnCtxGetCurrent"},
	{.name = "cnCtxSetCurrent"},
	{.name = "cnCtxGetDevice"},
	{.name = "cnCtxGetQueuePriorityRange"},
	{.name = "cnCtxGetConfig"},
	{.name = "cnCtxSetConfig"},
	{.name = "cnSetCtxConfigParam_pt"},
	{.name = "cnGetCtxConfigParam_pt"},
	{.name = "cnGetCtxMaxParallelUnionTasks"},
	{.name = "cnCtxSync"},
	{.name = "cnCtxResetPersistingL2Cache"},
	{.name = "cnSharedContextGetState"},
	{.name = "cnSharedContextSetFlags"},
	{.name = "cnSharedContextAcquire"},
	{.name = "cnSharedContextRelease"},
	{.name = "cnSharedContextReset"},
	{.name = "cnMemGetInfo"},
	{.name = "cnMemGetNodeInfo"},
	{.name = "cnMalloc"},
	{.name = "cnMallocSecurity"},
	{.name = "cnMallocNode"},
	{.name = "cnZmalloc"},
	{.name = "cnZmallocNode"},
	{.name = "cnMallocConstant"},
	{.name = "cnMallocNodeConstant"},
	{.name = "cnMallocFrameBuffer"},
	{.name = "cnMallocPeerAble"},
	{.name = "cnFree"},
	{.name = "cnMemMerge"},
	{.name = "cnMemGetAddressRange"},
	{.name = "cnMallocHost"},
	{.name = "cnFreeHost"},
	{.name = "cnIpcGetMemHandle"},
	{.name = "cnIpcOpenMemHandle"},
	{.name = "cnIpcCloseMemHandle"},
	{.name = "cnMemcpy"},
	{.name = "cnMemcpyPeer"},
	{.name = "cnMemcpyHtoD"},
	{.name = "cnMemcpyDtoH"},
	{.name = "cnMemcpyDtoD"},
	{.name = "cnMemcpyDtoD2D"},
	{.name = "cnMemcpyDtoD3D"},
	{.name = "cnMemcpy2D"},
	{.name = "cnMemcpy3D"},
	{.name = "cnMemcpyAsync"},
	{.name = "cnMemcpyAsync_V2"},
	{.name = "cnMemcpyPeerAsync"},
	{.name = "cnMemcpyHtoDAsync"},
	{.name = "cnMemcpyHtoDAsync_V2"},
	{.name = "cnMemcpyDtoHAsync"},
	{.name = "cnMemcpyDtoHAsync_V2"},
	{.name = "cnMemcpyDtoDAsync"},
	{.name = "cnMemsetD8"},
	{.name = "cnMemsetD16"},
	{.name = "cnMemsetD32"},
	{.name = "cnMemsetD8Async"},
	{.name = "cnMemsetD16Async"},
	{.name = "cnMemsetD32Async"},
	{.name = "cnDeviceCanPeerAble"},
	{.name = "cnGetMemAttribute"},
	{.name = "cnGetMemAttributes"},
	{.name = "cnSetMemAttribute"},
	{.name = "cnSetMemRangeAttribute"},
	{.name = "cnCreateQueue"},
	{.name = "cnCreateQueueWithPriority"},
	{.name = "cnQueueGetPriority"},
	{.name = "cnDestroyQueue"},
	{.name = "cnQueryQueue"},
	{.name = "cnQueueSync"},
	{.name = "cnQueueWaitNotifier"},
	{.name = "cnQueueGetContext"},
	{.name = "cnQueueSetAttribute"},
	{.name = "cnQueueGetAttribute"},
	{.name = "cnQueueCopyAttributes"},
	{.name = "cnQueueAddCallback"},
	{.name = "cnQueueBeginCapture"},
	{.name = "cnQueueEndCapture"},
	{.name = "cnQueueIsCapturing"},
	{.name = "cnQueueGetCaptureInfo"},
	{.name = "cnQueueUpdateCaptureDependencies"},
	{.name = "cnCreateNotifier"},
	{.name = "cnDestroyNotifier"},
	{.name = "cnWaitNotifier"},
	{.name = "cnQueryNotifier"},
	{.name = "cnPlaceNotifier"},
	{.name = "cnNotifierElapsedTime"},
	{.name = "cnNotifierElapsedExecTime"},
	{.name = "cnIpcGetNotifierHandle"},
	{.name = "cnIpcOpenNotifierHandle"},
	{.name = "cnQueueAtomicOperation"},
	{.name = "cnAtomicOperation"},
	{.name = "cnAtomicReadOps"},
	{.name = "cnModuleLoadFatBinary"},
	{.name = "cnModuleLoad"},
	{.name = "cnModuleUnload"},
	{.name = "cnModuleQueryMemoryUsage"},
	{.name = "cnModuleQueryFatBinaryMemoryUsage"},
	{.name = "cnModuleGetKernel"},
	{.name = "cnModuleGetSymbol"},
	{.name = "cnKernelGetAttribute"},
	{.name = "cnModuleGetLoadingMode"},
	{.name = "cnInvokeKernel"},
	{.name = "cnInvokeHostFunc"},
	{.name = "cnGetExportFunction"},
	{.name = "cnCacheOperation"},
	{.name = "cnMmap"},
	{.name = "cnMmapCached"},
	{.name = "cnMunmap"},
	{.name = "cnMemCreate"},
	{.name = "cnMemRelease"},
	{.name = "cnMemAddressReserve"},
	{.name = "cnMemAddressFree"},
	{.name = "cnMemMap"},
	{.name = "cnMemSetAccess"},
	{.name = "cnMemUnmap"},
	{.name = "cnMemGetAllocationGranularity"},
	{.name = "cnMemGetAllocationPropertiesFromHandle"},
	{.name = "cnMemGetAccess"},
	{.name = "cnMemRetainAllocationHandle"},
	{.name = "cnMemExportToShareableHandle"},
	{.name = "cnMemImportFromShareableHandle"},
	{.name = "cnTaskTopoCreate"},
	{.name = "cnTaskTopoDestroy"},
	{.name = "cnTaskTopoClone"},
	{.name = "cnTaskTopoNodeFindInClone"},
	{.name = "cnTaskTopoDestroyNode"},
	{.name = "cnTaskTopoGetEdges"},
	{.name = "cnTaskTopoGetNodes"},
	{.name = "cnTaskTopoGetRootNodes"},
	{.name = "cnTaskTopoAddDependencies"},
	{.name = "cnTaskTopoRemoveDependencies"},
	{.name = "cnTaskTopoNodeGetDependencies"},
	{.name = "cnTaskTopoNodeGetDependentNodes"},
	{.name = "cnTaskTopoNodeGetType"},
	{.name = "cnTaskTopoDebugDotPrint"},
	{.name = "cnTaskTopoKernelNodeGetAttribute"},
	{.name = "cnTaskTopoKernelNodeSetAttribute"},
	{.name = "cnTaskTopoKernelNodeCopyAttributes"},
	{.name = "cnUserObjectCreate"},
	{.name = "cnUserObjectAcquire"},
	{.name = "cnUserObjectRelease"},
	{.name = "cnTaskTopoAcquireUserObject"},
	{.name = "cnTaskTopoReleaseUserObject"},
	{.name = "cnTaskTopoAddEmptyNode"},
	{.name = "cnTaskTopoAddHostNode"},
	{.name = "cnTaskTopoHostNodeGetParams"},
	{.name = "cnTaskTopoHostNodeSetParams"},
	{.name = "cnTaskTopoAddKernelNode"},
	{.name = "cnTaskTopoKernelNodeGetParams"},
	{.name = "cnTaskTopoKernelNodeSetParams"},
	{.name = "cnTaskTopoAddMemcpyNode"},
	{.name = "cnTaskTopoMemcpyNodeGetParams"},
	{.name = "cnTaskTopoMemcpyNodeSetParams"},
	{.name = "cnTaskTopoAddMemsetNode"},
	{.name = "cnTaskTopoMemsetNodeGetParams"},
	{.name = "cnTaskTopoMemsetNodeSetParams"},
	{.name = "cnTaskTopoAddChildTopoNode"},
	{.name = "cnTaskTopoChildTopoNodeGetTopo"},
	{.name = "cnTaskTopoInstantiate"},
	{.name = "cnTaskTopoEntityDestroy"},
	{.name = "cnTaskTopoEntityInvoke"},
	{.name = "cnTaskTopoEntityHostNodeSetParams"},
	{.name = "cnTaskTopoEntityKernelNodeSetParams"},
	{.name = "cnTaskTopoEntityMemcpyNodeSetParams"},
	{.name = "cnTaskTopoEntityMemsetNodeSetParams"},
	{.name = "cnTaskTopoEntityChildTopoNodeSetParams"},
	{.name = "cnTaskTopoEntityUpdate"},
	{.name = "cnTaskTopoUpload"},
};

void getMluDriverLibPtrDlsym(){
	int i = 0; 

	for (i=0;i<MLU_ENTRY_END;i++){
		CNDrv_entry[i].func_ptr = (void *)real_dlsym(__libc_dlopen_mode("libcndrv.so", RTLD_LAZY), CNDrv_entry[i].name);
		if (!CNDrv_entry[i].func_ptr) {
			printf("Error can't get func %s ptr\n", CNDrv_entry[i].name);
		}
	}
    
}

__CN_EXPORT CNresult cnInit(unsigned int flags) {
	printf("hijcaking cnInit\n");
	CNresult ret;
	read_controller_configuration();
	pthread_once(&mlu_lib_ptr_init, getMluDriverLibPtrDlsym);
	pthread_once(&mlu_prod, create_prod_mlu_thread);
	

	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnInit), flags);

	return ret;
}

__CN_EXPORT CNresult cnGetLibVersion(int *major, int *minor, int *patch){
	CNresult ret;
	
	//printf("hijcaking cnGetLibVersion\n");
	read_controller_configuration();
	pthread_once(&mlu_lib_ptr_init, getMluDriverLibPtrDlsym);
	pthread_once(&mlu_prod, create_prod_mlu_thread);
	
	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnGetLibVersion), major, minor, patch);

	return ret;
}

__CN_EXPORT CNresult cnGetExportFunction(const char *name, const void **func) {
	CNresult ret;
	printf("hijacking cnGetExportFunction,want %s 's fptr\n", name);
	read_controller_configuration();
	pthread_once(&mlu_lib_ptr_init, getMluDriverLibPtrDlsym);
    pthread_once(&mlu_prod, create_prod_mlu_thread);
	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnGetExportFunction), name, func);
	
	return ret;
}


//Memory Management
/*ptotalBytes这里应该返回指定配额*/
__CN_EXPORT CNresult cnMemGetInfo(cn_uint64_t *pfreeBytes, cn_uint64_t *ptotalBytes) {
	CNresult ret;
	cn_uint64_t use = 0;

	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnMemGetInfo), pfreeBytes, ptotalBytes);
	*ptotalBytes = total_mem;
	use = get_mlu_mem();
	*pfreeBytes = total_mem - use;

	return ret;
}


__CN_EXPORT CNresult cnMalloc(CNaddr *pmluAddr, cn_uint64_t bytes) {
	CNresult ret;
	cn_uint64_t request_bytes = bytes; 
	cn_uint64_t use = 0;


	//获取容器内所有进程使用已使用的内存和配额比较
	use = get_mlu_mem();
	if (use + request_bytes >= total_mem){
		ret = CN_MEMORY_ERROR_OUT_OF_MEMORY; 
		goto DONE;
	}
		
	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnMalloc), pmluAddr, bytes);

DONE:	
	return ret;
}

__CN_EXPORT CNresult cnMallocSecurity(CNaddr *pmluAddr, cn_uint64_t bytes) {
	CNresult ret;
	cn_uint64_t request_bytes = bytes; 
	cn_uint64_t use = 0;
	//printf("hijacking cnMallocSecurity\n");

	use = get_mlu_mem();
	if (use + request_bytes >= total_mem){
		ret = CN_MEMORY_ERROR_OUT_OF_MEMORY; 
		goto DONE;
	}

	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnMallocSecurity), pmluAddr, bytes);

DONE:
	return ret;
}

__CN_EXPORT CNresult cnZmalloc(CNaddr *pmluAddr, cn_uint64_t bytes) {
	CNresult ret;
	cn_uint64_t request_bytes = bytes; 
	cn_uint64_t use = 0;
	//printf("hijacking cnZmalloc\n");


	use = get_mlu_mem();
	if (use + request_bytes >= total_mem){
		ret = CN_MEMORY_ERROR_OUT_OF_MEMORY; 
		goto DONE;
	}

	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnZmalloc), pmluAddr, bytes);

DONE:	
	return ret;
}

__CN_EXPORT CNresult cnMallocConstant(CNaddr *pmluAddr, cn_uint64_t bytes) {
	CNresult ret;
	cn_uint64_t request_bytes = bytes; 
	cn_uint64_t use = 0;
	//printf("hijacking cnMallocConstant\n");

	use = get_mlu_mem();
	if (use + request_bytes >= total_mem){
		ret = CN_MEMORY_ERROR_OUT_OF_MEMORY; 
		goto DONE;
	}

	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnMallocConstant), pmluAddr, bytes);

DONE:	
	return ret;
}

// /*cpu和gpu共享内存？*/
// __CN_EXPORT CNresult cnMallocPeerAble(CNaddr *pmluAddr, cn_uint64_t bytes) {
// 	CNresult ret;
// 	//printf("hijacking cnMallocPeerAble\n");
// 	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnMallocPeerAble), pmluAddr, bytes);
// 	return ret;
// }

// //node的内存是什么？？numa node的内存？
// __CN_EXPORT CNresult cnMemGetNodeInfo(cn_uint64_t *pfreeBytes, cn_uint64_t *ptotalBytes,
//                                              int node) {
// 	CNresult ret;
// 	//printf("hijacking cnMemGetNodeInfo\n");
// 	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnMemGetNodeInfo), pfreeBytes, ptotalBytes, node);
// 	return ret;
// }

// __CN_EXPORT CNresult cnMallocNode(CNaddr *pmluAddr, cn_uint64_t bytes, int node) {
// 	CNresult ret;
// 	//printf("hijacking cnMallocNode\n");
// 	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnMallocNode), pmluAddr, bytes, node);
// 	return ret;
// }

// __CN_EXPORT CNresult cnZmallocNode(CNaddr *pmluAddr, cn_uint64_t bytes, int node) {
// 	CNresult ret;
// 	//printf("hijacking cnZmallocNode\n");
// 	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnZmallocNode), pmluAddr, bytes, node);
// 	return ret;
// }

// __CN_EXPORT CNresult cnMallocNodeConstant(CNaddr *pmluAddr, cn_uint64_t bytes, int node) {
// 	CNresult ret;
// 	//printf("hijacking cnMallocNodeConstant\n");
// 	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnMallocNodeConstant), pmluAddr, bytes, node);
// 	return ret;
// }

// __CN_EXPORT CNresult cnMallocFrameBuffer(CNaddr *pmluAddr, cn_uint64_t bytes, int node) {
// 	CNresult ret;
// 	//printf("hijacking cnMallocFrameBuffer\n");
// 	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnMallocFrameBuffer), pmluAddr, bytes, node);
// 	return ret;
// }

//Virtual Memory Management

//cnTack的要拦截吗？

/*不用管内存释放，如果和nvml一样有可以实时获得进程用了多少内存的话*/


//kernel
__CN_EXPORT CNresult cnInvokeKernel(CNkernel hkernel, unsigned int dimx, unsigned int dimy,
                                           unsigned int dimz, KernelClass c, unsigned int reserve,
                                           CNqueue hqueue, void **kernelParams, void **extra) {
	CNresult ret;
	printf("hijacking cnInvokeKernel\n");
	rate_limiter(dimx * dimy, dimx * dimy * dimz);
	ret = MY_CALL_ENTRY(REAL_MLU_FUNC_PTR(cnInvokeKernel), hkernel, dimx, dimy, dimz, c, reserve, hqueue, kernelParams, extra);
	return ret;
}

void create_prod_mlu_thread() {
	pthread_t s_ctl;
	int ret;
	const char *acce_type = "mlu";

	ret = pthread_create(&s_ctl, NULL, source_control, (void*)acce_type);
	if (ret !=0) {
		perror("mlu pthread_create");
        exit(EXIT_FAILURE);
	}
}