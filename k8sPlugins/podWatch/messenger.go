package podWatch

import (
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"strconv"
	"uni-share/deviceInfo"

	"github.com/kubesys/client-go/pkg/kubesys"
	"github.com/tidwall/gjson"
	v1 "k8s.io/api/core/v1"
)

type KubeMessenger struct {
	Client   *kubesys.KubernetesClient
	nodeName string
}

type gpuresource struct {
	UUID      string
	memTotal  int64
	coreTotal int64
	memUsed   int64
	coreUsed  int64
	crdName   string
}

func NewKubeMessenger(Client *kubesys.KubernetesClient, nodeName string) *KubeMessenger {
	return &KubeMessenger{
		Client:   Client,
		nodeName: nodeName,
	}
}

func (m *KubeMessenger) getNodeInfo() *v1.Node {
	node, err := m.Client.GetResource("Node", "", m.nodeName)
	if err != nil {
		fmt.Println("getNodeInfo error")
		return nil
	}
	var nodeInfo v1.Node
	err = json.Unmarshal(node, &nodeInfo)
	if err != nil {
		fmt.Println("Unmarshal error")
		return nil
	}

	return &nodeInfo
}

func (m *KubeMessenger) updateNodeStatus(nodeInfo *v1.Node) error {
	nodeInfoJson, err := json.Marshal(nodeInfo) //??这里要解引用吗
	if err != nil {
		fmt.Println("getNodeInfo error")
		return err
	}
	_, err = m.Client.UpdateResourceStatus(string(nodeInfoJson))
	if err != nil {
		fmt.Println("getNodeInfo error")
		return err
	}
	return nil
}

func (m *KubeMessenger) updateNode(nodeInfo *v1.Node) error {
	nodeInfoJson, err := json.Marshal(nodeInfo)
	if err != nil {
		fmt.Println("getNodeInfo error")
		return err
	}
	_, err = m.Client.UpdateResource(string(nodeInfoJson))
	if err != nil {
		fmt.Println("getNodeInfo error")
		return err
	}
	return nil
}

func (m *KubeMessenger) UpdateCrd(gpu deviceInfo.GPU) error {
	jb, err := json.Marshal(gpu)
	if err != nil {
		log.Fatalf("Failed to marshal gpu struct, %s.", err)
	}
	_, err = m.Client.UpdateResource(string(jb))
	if err != nil && err.Error() != "request status 201 Created" {
		log.Fatalf("Failed to create gpu %s, %s.", gpu.Name, err)
	}

	return err
}

func (m *KubeMessenger) GetGpuResource(podCoreReq int64, podMemReq int64) (string, error) {
	gpuByteList, err := m.Client.GetResource("GPU", "default", "")
	if err != nil {
		log.Fatal("get gpu resource error")
	}
	gpuJson := kubesys.ToJsonObject(gpuByteList)
	gpudataList := make([]*gpuresource, 0)
	gpuJson.Get("items").ForEach(func(key, value gjson.Result) bool {
		gpudataList = append(gpudataList, &gpuresource{
			UUID:      value.Get("spec").Get("uuid").String(),
			memTotal:  value.Get("spec").Get("capacity").Get("core").Int(),
			coreTotal: value.Get("spec").Get("capacity").Get("memory").Int(),
			memUsed:   value.Get("spec").Get("used").Get("core").Int(),
			coreUsed:  value.Get("spec").Get("used").Get("memory").Int(),
			crdName:   value.Get("metadata").Get("name").String(),
		})

		return true
	})
	statifyArr := make([]int, 0)
	for i := 0; i < len(gpudataList); i++ {
		if (gpudataList[i].coreTotal-gpudataList[i].coreUsed >= podCoreReq) && (gpudataList[i].memTotal-gpudataList[i].memUsed >= podMemReq) {
			statifyArr = append(statifyArr, i)
		}
	}
	if len(statifyArr) == 0 {
		return "", errors.New("no gpu match")
	}
	var sum int64 = 99999
	candidata := 0
	for _, v := range statifyArr {
		if (gpudataList[v].coreTotal - gpudataList[v].coreUsed + gpudataList[v].memTotal - gpudataList[v].memUsed) < sum {
			sum = gpudataList[v].coreTotal - gpudataList[v].coreUsed + gpudataList[v].memTotal - gpudataList[v].memUsed
			candidata = v
		}
	}
	//update crd
	gpuByte, err := m.Client.GetResource("GPU", "default", gpudataList[candidata].crdName)
	var data deviceInfo.GPU
	err = json.Unmarshal(gpuByte, &data)
	//gpudata := kubesys.ToJsonObject(gpuByte)
	curCoreUsed, _ := strconv.ParseInt(data.Spec.Used.Core, 10, 64)
	curCoreUsed += podCoreReq
	data.Spec.Used.Core = strconv.FormatInt(curCoreUsed, 10)
	curMemUsed, _ := strconv.ParseInt(data.Spec.Used.Memory, 10, 64)
	curMemUsed += podMemReq
	data.Spec.Used.Memory = strconv.FormatInt(curMemUsed, 10)
	jb, _ := json.Marshal(data)
	_, err = m.Client.UpdateResource(string(jb))
	if err != nil {
		return "", errors.New("update crd failed")
	}

	return gpudataList[candidata].UUID, nil
}

func (m *KubeMessenger) UpdatePodAnnotations(pod *v1.Pod) error {
	podJson, err := json.Marshal(pod)
	if err != nil {
		return err
	}
	_, err = m.Client.UpdateResource(string(podJson))
	if err != nil {
		return err
	}
	return nil
}

func (m *KubeMessenger) GetPodOnNode(nameSpace string, podName string) *v1.Pod {
	podByteInfo, err := m.Client.GetResource("Pod", nameSpace, podName)
	if err != nil {
		fmt.Println("getpodByteInfo error")
		return nil
	}
	var podInfo v1.Pod
	err = json.Unmarshal(podByteInfo, &podInfo)
	if err != nil {
		fmt.Println("Unmarshal error")
		return nil
	}

	return &podInfo
}

func (m *KubeMessenger) GetPendingPodOnNode() []*v1.Pod {
	pendingPodList := make([]*v1.Pod, 0)
	podByteList, err := m.Client.ListResources("Pod", "")
	if err != nil {
		return nil
	}

	var podList v1.PodList
	err = json.Unmarshal(podByteList, &podList)
	if err != nil {
		return nil
	}
	for _, pod := range podList.Items {
		if pod.Spec.NodeName == m.nodeName && pod.Status.Phase == "Pending" {
			podCopy := pod.DeepCopy() //不深拷贝的话podList数据在函数结束就销毁了
			podCopy.APIVersion = "v1"
			podCopy.Kind = "Pod"
			pendingPodList = append(pendingPodList, podCopy)
		}
	}

	return pendingPodList
}
