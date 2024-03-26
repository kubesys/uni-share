package resourceManager

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"time"
	"uni-share/podWatch"
	"uni-share/vcuda"
	"unsafe"

	"github.com/kubesys/client-go/pkg/kubesys"
	log "github.com/sirupsen/logrus"
	"github.com/tidwall/gjson"
	v1 "k8s.io/api/core/v1"
)

// #include <stdint.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <string.h>
// #include <sys/file.h>
// #include <time.h>
// #include <stdlib.h>
// #include <unistd.h>
//
// #ifndef NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE
// #define NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE 16
// #endif
//
// #ifndef FILENAME_MAX
// #define FILENAME_MAX 4096
// #endif
//
// struct version_t {
//  int major;
//  int minor;
// } __attribute__((packed, aligned(8)));
//
// struct resource_data_t {
//  char pod_uid[48];
//  char container_name[FILENAME_MAX];
//  uint64_t memory;
//  int utilization;
// } __attribute__((packed, aligned(8)));
//
// int setting_to_disk(const char* filename, struct resource_data_t* data) {
//  int fd = 0;
//  int wsize = 0;
//  int ret = 0;
//
//  fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 00777);
//  if (fd == -1) {
//    return 1;
//  }
//
//  wsize = (int)write(fd, (void*)data, sizeof(struct resource_data_t));
//  if (wsize != sizeof(struct resource_data_t)) {
//    ret = 2;
//	goto DONE;
//  }
//
// DONE:
//  close(fd);
//
//  return ret;
// }
//
// int pids_to_disk(const char* filename, int* data, int size) {
//  int fd = 0;
//  int wsize = 0;
//  struct timespec wait = {
//	.tv_sec = 0, .tv_nsec = 100 * 1000 * 1000,
//  };
//  int ret = 0;
//
//  fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 00777);
//  if (fd == -1) {
//    return 1;
//  }
//
//  while (flock(fd, LOCK_EX)) {
//    nanosleep(&wait, NULL);
//  }
//
//  wsize = (int)write(fd, (void*)data, sizeof(int) * size);
//  if (wsize != sizeof(int) * size) {
//	ret = 2;
//    goto DONE;
//  }
//
// DONE:
//  flock(fd, LOCK_UN);
//  close(fd);
//
//  return ret;
// }
import "C"

type VirtualManager struct {
	vcuda.UnimplementedVCUDAServiceServer
	PodByUID              map[string]gjson.Result //包含gpu资源的pod
	ContainerNameByUid    map[string]string
	ContainerUIDByName    map[string]string
	ContainerUIDInPodUID  map[string][]string
	PodVisitedByUID       map[string]bool
	PodDoByUID            map[string]bool
	CoreRequestByPodUID   map[string]int64
	MemoryRequestByPodUID map[string]int64
	client                *kubesys.KubernetesClient
	podMgr                *podWatch.PodManager
	mu                    sync.Mutex
}

func NewVirtualManager(client *kubesys.KubernetesClient, podMgr *podWatch.PodManager) *VirtualManager {
	return &VirtualManager{
		PodByUID:              make(map[string]gjson.Result),
		ContainerNameByUid:    make(map[string]string, 0),
		ContainerUIDByName:    make(map[string]string, 0),
		ContainerUIDInPodUID:  make(map[string][]string, 0),
		PodVisitedByUID:       make(map[string]bool),
		PodDoByUID:            make(map[string]bool),
		CoreRequestByPodUID:   make(map[string]int64),
		MemoryRequestByPodUID: make(map[string]int64),
		client:                client,
		podMgr:                podMgr,
	}
}

func (vm *VirtualManager) Run() {
	if err := os.MkdirAll(VirtualManagerPath, 0777); err != nil && !os.IsNotExist(err) {
		log.Fatalf("Failed to create %s, %s.", VirtualManagerPath, err)
	}
	fmt.Println("vitrualManager runing")
	for {
		if len(vm.podMgr.PodModObj) > 0 {
			jsonObj := vm.podMgr.PodModObj[0]
			vm.podMgr.MuOfModify.Lock()
			vm.podMgr.PodModObj = vm.podMgr.PodModObj[1:]
			vm.podMgr.MuOfModify.Unlock()
			meta := jsonObj.Get("metadata")
			if meta.Get("name").String() == "testpod117" {
				fmt.Println(jsonObj)
			}
			go vm.creatVcudaServer(jsonObj)
		}
		if len(vm.podMgr.PodDelObj) > 0 {
			jsonObj := vm.podMgr.PodDelObj[0]
			vm.podMgr.MuOfDelete.Lock()
			vm.podMgr.PodDelObj = vm.podMgr.PodDelObj[1:]
			vm.podMgr.MuOfDelete.Unlock()
			go vm.deletePodPath(jsonObj)
		}
	}

}

/*
在容器创建前建好，多少个server？
pod传指针还是实体
创建一个pod调了5次。。。。
*/
func (vm *VirtualManager) creatVcudaServer(pod gjson.Result) {
	var resFlag bool = false
	meta := pod.Get("metadata")
	fmt.Println("createVcudaServer", meta.Get("name").String())
	if !meta.Get("annotations").Exists() {
		log.Errorln("Failed to get pod annotation.")
		return
	}
	specTest := pod.Get("spec")
	specTest.Get("containers").ForEach(func(key, value gjson.Result) bool {
		if !(value.Get("resources").Exists()) {
			return true
		}
		resources := value.Get("resources")
		if !(resources.Get("requests").Exists()) {
			return true
		}

		resFlag = true
		return true
	})
	if !resFlag {
		fmt.Println("not need gpu resource")
		return
	}

	annotations := meta.Get("annotations")
	// if !annotations.Get(AnnAssumeTime).Exists() { //调度时分配的
	// 	fmt.Println(meta.Get("name").String(), "not need gpu resource")
	// 	return
	// }
	flag := annotations.Get(`doslab\.io/gpu-assigned`).String() //调度成功分配的false，容器创建后改成true，这里只检查是否有annotation
	// if flag == "" {
	// 	log.Errorln("Failed to get assigned flag.")
	// 	return
	// }

	podName := meta.Get("name").String()
	if podName == "" {
		log.Errorln("Failed to get pod name.")
		return
	}
	namespace := meta.Get("namespace").String()
	if namespace == "" {
		log.Errorln("Failed to get pod namespace.")
		return
	}
	podUID := meta.Get("uid").String()
	if podUID == "" {
		log.Errorln("Failed to get pod podUID.")
		return
	}
	fmt.Println("flag is", flag, "cont func be visit?", vm.PodDoByUID[podUID])
	if flag == "true" && !vm.PodDoByUID[podUID] { //容器创建后执行，PodDoByUID保证这些语句只执行一次（pod创建会多次状态变更会多次调用creatVcudaServer）
		status := pod.Get("status")

		ready := false
		for i := 0; i < 100; i++ {
			if !status.Get("containerStatuses").Exists() {
				log.Errorf("Pod %s on ns %s has no containerStatuses, try later.", podName, namespace)
				time.Sleep(time.Millisecond * 200)
				podByte, err := vm.client.GetResource("Pod", namespace, podName)
				if err != nil {
					log.Errorf("Failed to get pod %s on ns %, %s.", podName, namespace, err)
					return
				}
				pod := kubesys.ToJsonObject(podByte)
				status = pod.Get("status")
			} else {
				ready = true
				break
			}
		}

		if !ready {
			log.Errorf("Pod %s on ns %s has no containerStatuses.", podName, namespace)
			return
		}

		vm.mu.Lock() //这里往下直接return会导致死锁
		fmt.Println(status.Get("containerStatuses"))
		var contStatusFlag bool = false
		status.Get("containerStatuses").ForEach(func(key, value gjson.Result) bool {
			uidStr := value.Get("containerID").String()
			fmt.Println("uidStr is", uidStr)
			if uidStr == "" {
				return true
			}
			name := value.Get("name").String()
			fmt.Println("cont name is ", name)
			if name == "" {
				return true
			}
			uid := strings.Split(uidStr, "docker://")[1]
			fmt.Println("uid is", uid)
			vm.ContainerNameByUid[uid] = name
			vm.ContainerUIDByName[name] = uid
			vm.ContainerUIDInPodUID[podUID] = append(vm.ContainerUIDInPodUID[podUID], uid)
			contStatusFlag = true
			return true
		})
		vm.mu.Unlock()
		if !contStatusFlag {
			return
		}
		var baseDir string
		contUidList := vm.ContainerUIDInPodUID[podUID]
		for _, v := range contUidList {
			if isCgroupVersionV2() {
				baseDir = filepath.Join(VirtualManagerPath, podUID, vm.ContainerNameByUid[v])
			} else {
				baseDir = filepath.Join(VirtualManagerPath, podUID, v)
			}
			if err := os.MkdirAll(baseDir, 0777); err != nil && !os.IsNotExist(err) {
				log.Errorf("Failed to create %s, %s.", baseDir, err)
			}

			/*
				Create vcuda.config file
				/etc/uni-share/vm
				├── podUID1
				├── podUID2
				│   ├── containerName1(id)
				│   └── containerName2(id)
			*/
			vcudaFileName := filepath.Join(baseDir, ConfigFileName)
			//标记修改
			err := vm.createVCUDAFile(vcudaFileName, podUID, vm.ContainerNameByUid[v])
			if err != nil {
				log.Errorf("Failed to create %s, %s.", vcudaFileName, err)
			}
		}
		fmt.Println("basedir is ", baseDir)

		vm.PodDoByUID[podUID] = true
	}

	if vm.PodVisitedByUID[podUID] {
		return
	}
	vm.mu.Lock()
	vm.PodVisitedByUID[podUID] = true
	vm.PodByUID[podUID] = pod
	vm.mu.Unlock()
	baseDir := filepath.Join(VirtualManagerPath, podUID)
	if err := os.MkdirAll(baseDir, 0777); err != nil && !os.IsExist(err) {
		log.Errorf("Failed to create %s, %s.", baseDir, err)
		return
	}
	spec := pod.Get("spec")
	requestMemory, requestCore := int64(0), int64(0)
	spec.Get("containers").ForEach(func(key, value gjson.Result) bool {
		if !(value.Get("resources").Exists()) {
			return true
		}
		resources := value.Get("resources")
		if !(resources.Get("limits").Exists()) {
			return true
		}
		limits := resources.Get("limits")
		//必须指定limit的core和memory
		if limits.Get(ResourceCore).Exists() && limits.Get(ResourceMemory).Exists() {
			core := limits.Get(ResourceCore).String()
			mem := limits.Get(ResourceMemory).String()
			m, _ := strconv.ParseInt(core, 10, 64)
			requestCore += m
			m, _ = strconv.ParseInt(mem, 10, 64)
			requestMemory += m
		} else if limits.Get(MluResourceCore).Exists() && limits.Get(MluResourceMemory).Exists() {
			core := limits.Get(MluResourceCore).String()
			mem := limits.Get(MluResourceMemory).String()
			m, _ := strconv.ParseInt(core, 10, 64)
			requestCore += m
			m, _ = strconv.ParseInt(mem, 10, 64)
			requestMemory += m
		}

		return true
	})

	vm.mu.Lock()
	vm.CoreRequestByPodUID[podUID] = requestCore
	vm.MemoryRequestByPodUID[podUID] = requestMemory
	vm.mu.Unlock()
	//Update annotation
	time.Sleep(time.Second)
	copyPodBytes, err := vm.client.GetResource("Pod", namespace, podName)
	if err != nil {
		log.Errorf("Failed to get copy pod %s on ns %s, %s.", podName, namespace, err)
		return
	}
	var v1Pod v1.Pod
	json.Unmarshal(copyPodBytes, &v1Pod)
	copyPod := v1Pod.DeepCopy()
	copyPod.Annotations[AnnVCUDAReady] = "yes"
	jsonCopyPod, err := json.Marshal(copyPod)
	_, err = vm.client.UpdateResource(string(jsonCopyPod))
	if err != nil {
		log.Errorf("Failed to set pod %s's annotations, %s.", podName, err)
		return
	}
}

// 删除已经删除GPUpod留下的文件（pid，vcuda，sock）和文件夹
func (vm *VirtualManager) deletePodPath(pod gjson.Result) {
	meta := pod.Get("metadata")
	if !meta.Get("annotations").Exists() {
		return
	}
	annotations := meta.Get("annotations")
	if !annotations.Get(`doslab\.io/gpu-assigned`).Exists() {
		return
	}

	podUID := meta.Get("uid").String()

	log.Infof("Clean for pod %s.", podUID)
	vm.mu.Lock()
	defer vm.mu.Unlock()

	if !vm.PodVisitedByUID[podUID] {
		return
	}

	vm.PodVisitedByUID[podUID] = false

	delete(vm.CoreRequestByPodUID, podUID)
	delete(vm.MemoryRequestByPodUID, podUID)
	delete(vm.PodByUID, podUID)
	delete(vm.PodDoByUID, podUID)

	for _, uid := range vm.ContainerUIDInPodUID[podUID] {
		name := vm.ContainerNameByUid[uid]
		delete(vm.ContainerNameByUid, uid)
		delete(vm.ContainerUIDByName, name)
	}
	delete(vm.ContainerUIDInPodUID, podUID)

	os.RemoveAll(filepath.Clean(filepath.Join(VirtualManagerPath, podUID)))

}

func (vm *VirtualManager) createVCUDAFile(configFileName, podUID, containerName string) error {
	log.Infof("Write %s", configFileName)
	requestMemory, requestCore := int64(0), int64(0)
	vm.mu.Lock()
	requestMemory = vm.MemoryRequestByPodUID[podUID]
	requestCore = vm.CoreRequestByPodUID[podUID]
	vm.mu.Unlock()

	var vConfig C.struct_resource_data_t

	cPodUID := C.CString(podUID)
	cContName := C.CString(containerName)
	cFileName := C.CString(configFileName)

	defer C.free(unsafe.Pointer(cPodUID))
	defer C.free(unsafe.Pointer(cContName))
	defer C.free(unsafe.Pointer(cFileName))

	C.strcpy(&vConfig.pod_uid[0], (*C.char)(unsafe.Pointer(cPodUID)))
	C.strcpy(&vConfig.container_name[0], (*C.char)(unsafe.Pointer(cContName)))
	vConfig.memory = C.uint64_t(requestMemory) * MemoryBlockSize
	vConfig.utilization = C.int(requestCore)

	if C.setting_to_disk(cFileName, &vConfig) != 0 {
		return errors.New("create vcuda.config file error")
	}

	return nil

}
