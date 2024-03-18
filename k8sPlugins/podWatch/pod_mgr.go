/**
 * Copyright (2021, ) Institute of Software, Chinese Academy of Sciences
 **/

package podWatch

import (
	"encoding/json"
	"sync"

	"github.com/kubesys/client-go/pkg/kubesys"
	"github.com/tidwall/gjson"
)

type PodManager struct {
	PodModObj  []gjson.Result
	PodDelObj  []gjson.Result
	MuOfModify sync.Mutex
	MuOfDelete sync.Mutex
}

func NewPodManager() *PodManager {
	return &PodManager{
		PodModObj: make([]gjson.Result, 0),
		PodDelObj: make([]gjson.Result, 0),
	}
}

func (podMgr *PodManager) DoAdded(obj map[string]interface{}) {

	// bytes, _ := json.Marshal(obj)
	// fmt.Println(kubesys.ToJsonObject(bytes))
	// for k, _ := range obj {
	// 	fmt.Println(k)
	// }
}

// 删除pod和创建pod都会有多次状态变更，触发函数往PodModObj放数据
func (podMgr *PodManager) DoModified(obj map[string]interface{}) {
	bytes, _ := json.Marshal(obj)
	podMgr.MuOfModify.Lock()
	podMgr.PodModObj = append(podMgr.PodModObj, kubesys.ToJsonObject(bytes))
	podMgr.MuOfModify.Unlock()
}

func (podMgr *PodManager) DoDeleted(obj map[string]interface{}) {
	bytes, _ := json.Marshal(obj)
	podMgr.MuOfDelete.Lock()
	podMgr.PodDelObj = append(podMgr.PodDelObj, kubesys.ToJsonObject(bytes))
	podMgr.MuOfDelete.Unlock()

}
