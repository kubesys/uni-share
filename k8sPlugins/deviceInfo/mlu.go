package deviceInfo

import (
	"encoding/json"
	"fmt"
	"log"
	"strconv"

	"github.com/kubesys/client-go/pkg/kubesys"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
)

/*
#cgo CFLAGS: -I/usr/local/neuware/include
#cgo LDFLAGS: -L/usr/local/neuware/lib64 -lcndev
#include <stdio.h>
#include <stdlib.h>
#include <cndev.h>
#include <string.h>

#define MAX_MLU_NUM 8

typedef struct {
	int mluNum;
	char uuid[MAX_MLU_NUM][1024];
	long long int memory[MAX_MLU_NUM];
}mluInfo_t;

mluInfo_t initCndev() {
	mluInfo_t mluInfo;
    cndevDevice_t devHandle;
    cndevRet_t ret = cndevInit(0);
	int i = 0;

    // you can compare cndevRet manually, or just use cndevCheckErrors
    if (CNDEV_SUCCESS != ret) {
      printf("cndev init failed: %s.\n", cndevGetErrorString(ret));
      // should exit now
      exit(0);
    }
	// get card count
  	cndevCardInfo_t cardInfo;
  	cardInfo.version = CNDEV_VERSION_5;
  	ret = cndevGetDeviceCount(&cardInfo);
	mluInfo.mluNum = cardInfo.number;
	for (i=0;i<cardInfo.number && i < MAX_MLU_NUM;i++) {
		cndevDevice_t devHandle;
    	ret = cndevGetDeviceHandleByIndex(i, &devHandle);
   		cndevCheckErrors(ret);

		cndevUUID_t uuidInfo;
		uuidInfo.version = CNDEV_VERSION_5;
		ret = cndevGetUUID(&uuidInfo, devHandle);
		if (ret == CNDEV_ERROR_NOT_SUPPORTED) {
			printf("cndevGetUUID is not supported\n");
		}
		else {
			cndevCheckErrors(ret);
			strcpy(mluInfo.uuid[i], uuidInfo.uuid);
			printf("UUID: %s\n", uuidInfo.uuid);
		}

		// get card[x]'s memory info
		cndevMemoryInfo_t memInfo;
		memInfo.version = CNDEV_VERSION_5;
		cndevCheckErrors(cndevGetMemoryUsage(&memInfo, devHandle));
		mluInfo.memory[i] = memInfo.physicalMemoryTotal;
		// printf("Phy mem total:%ldMiB, Phy mem used:%ldMiB, Vir mem total:%ldMiB, Vir mem used:%ldMiB\n"
		// 		"Global mem:%ldMiB\n",
		// 		memInfo.physicalMemoryTotal,
		// 		memInfo.physicalMemoryUsed,
		// 		memInfo.virtualMemoryTotal,
		// 		memInfo.virtualMemoryUsed,
		// 		memInfo.globalMemory);
		}

    return mluInfo;
}
*/
import "C"

const (
	MLUCRDAPIVersion = "doslab.io/v1"
	MLUCRDNamespace  = "default"
)

type MluInfo struct {
	count   int
	mluUid  []string
	memInfo []int64
	client  *kubesys.KubernetesClient
}

func NewMluInfo(client *kubesys.KubernetesClient) *MluInfo {
	return &MluInfo{
		count:   0,
		mluUid:  make([]string, 0),
		memInfo: make([]int64, 0),
		client:  client,
	}
}

func (m *MluInfo) GetInfo() {
	var cStruct C.struct_mluInfo_t
	cStruct = C.initCndev()
	m.count = int(cStruct.mluNum)
	for i := 0; i < m.count; i++ {
		m.mluUid[i] = C.GoString(&cStruct.uuid[i][0])
		m.memInfo[i] = int64(cStruct.memory[i])
	}
}

func (m *MluInfo) GetMemInfo() []int64 {
	return m.memInfo
}

func (m *MluInfo) CreateCRD(hostname string) {
	for i := 0; i < m.count; i++ {
		mlu := MLU{
			TypeMeta: metav1.TypeMeta{
				Kind:       "MLU",
				APIVersion: GPUCRDAPIVersion,
			},
			ObjectMeta: metav1.ObjectMeta{
				Name:      fmt.Sprintf("%s-gpu-%d", hostname, i),
				Namespace: GPUCRDNamespace,
			},
			Spec: MLUSpec{
				UUID: m.mluUid[i],
				Capacity: R{
					Core:   "100",
					Memory: strconv.Itoa(int(m.memInfo[i] / 268435456)),
				},
				Used: R{
					Core:   "0",
					Memory: "0",
				},
				Node: hostname,
			},
		}
		jb, err := json.Marshal(mlu)
		if err != nil {
			log.Fatalf("Failed to marshal gpu struct, %s.", err)
		}
		_, err = m.client.CreateResource(string(jb))
		if err != nil && err.Error() != "request status 201 Created" {
			log.Fatalf("Failed to create gpu %s, %s.", mlu.Name, err)
		}
	}
}

func (m *MluInfo) GetCount() int {
	return m.count
}

func (m *MluInfo) GetUUID() []string {
	return m.mluUid
}
