package nvidia

import (
	"context"
	"errors"
	"fmt"
	"net"
	"os"
	"path/filepath"
	"syscall"
	"time"
	"uni-share/deviceInfo"
	"uni-share/podWatch"

	"google.golang.org/grpc"
	v1 "k8s.io/api/core/v1"
	"k8s.io/klog"
	pluginapi "k8s.io/kubelet/pkg/apis/deviceplugin/v1beta1"
)

const (
	vnvidiaCoreSocketName = "vnvidiacore.sock"
	vmluCoreSocketName    = "vmlucore.sock"
)

type VcoreResourceServer struct {
	kubeMessenger *podWatch.KubeMessenger
	srv           *grpc.Server
	socketFile    string
	resourceName  string
	devInfo       *deviceInfo.GpuInfo
}

var _ pluginapi.DevicePluginServer = &VcoreResourceServer{} //检测是否实现了ListAndWatch等接口
var _ ResourceServer = &VcoreResourceServer{}               //检测是否实现了SocketName等接口

func NewVcoreResourceServer(kubeMessenger *podWatch.KubeMessenger, devInfo *deviceInfo.GpuInfo) ResourceServer {
	socketFile := filepath.Join("/var/lib/kubelet/device-plugins/", NvidiaCoreSocketName)

	return &VcoreResourceServer{
		kubeMessenger: kubeMessenger,
		//srv:           grpc.NewServer(),
		socketFile:   socketFile,
		resourceName: ResourceCore,
		devInfo:      devInfo,
	}
}

func (vr *VcoreResourceServer) SocketName() string {
	return vr.socketFile
}

func (vr *VcoreResourceServer) ResourceName() string {
	return vr.resourceName
}

func (vr *VcoreResourceServer) Stop() {
	vr.srv.Stop()
}

func (vr *VcoreResourceServer) Run() error {
	vr.srv = grpc.NewServer()
	pluginapi.RegisterDevicePluginServer(vr.srv, vr)

	err := syscall.Unlink(vr.socketFile)
	if err != nil && !os.IsNotExist(err) {
		return err
	}

	l, err := net.Listen("unix", vr.socketFile)
	if err != nil {
		return err
	}

	klog.V(2).Infof("Server %s is ready at %s", vr.resourceName, vr.socketFile)

	return vr.srv.Serve(l)
}

/** device plugin interface */
func (vr *VcoreResourceServer) Allocate(ctx context.Context, reqs *pluginapi.AllocateRequest) (*pluginapi.AllocateResponse, error) {

	fmt.Println("%+v allocation request for vcore", reqs)
	allocResp := &pluginapi.AllocateResponse{}

	req := reqs.ContainerRequests[0]
	allReqDevice := len(req.DevicesIDs)

	//获取未创建的gpuPod
	var reqDevCount int
	pendingPod := vr.kubeMessenger.GetPendingPodOnNode()
	var candidatePodList []*v1.Pod
	var flag bool = false
	for _, v := range pendingPod {
		for _, cont := range v.Spec.Containers {
			if _, ok := cont.Resources.Limits[ResourceCore]; ok {
				flag = true
				break
			}

		}
		if flag {
			candidatePodList = append(candidatePodList, v)
		}
	}
	var confirmPod v1.Pod
	for _, v := range candidatePodList {
		for _, cont := range v.Spec.Containers {
			temp := cont.Resources.Limits[ResourceCore]
			reqDevCount += int(temp.Value())
		}
		if reqDevCount == allReqDevice {
			confirmPod = *v.DeepCopy()
			break
		}
	}

	//等待那边server建好文件夹，这里确定是哪个pod，再挂对应pod文件夹进去
	isOk := false
	for i := 0; i < 100; i++ {
		pod := vr.kubeMessenger.GetPodOnNode(confirmPod.Namespace, confirmPod.Name)
		if pod == nil {
			fmt.Printf("Failed to get pod %s, on ns %s.\n", confirmPod.Namespace, confirmPod.Name)
			time.Sleep(time.Millisecond * 200)
			continue
		}
		ready := getVCUDAReadyFromPodAnnotation(pod)
		if ready != "" {
			isOk = true
			confirmPod = *pod
			break
		}
		time.Sleep(time.Millisecond * 2000)
	}

	if !isOk {
		return nil, errors.New("vcuda not ready")
	}

	conAllocResp := &pluginapi.ContainerAllocateResponse{
		Envs:        make(map[string]string),
		Mounts:      make([]*pluginapi.Mount, 0),
		Devices:     make([]*pluginapi.DeviceSpec, 0), //代表/dev下的设备,这个设备和pluginapi.Device有什么映射关系吗
		Annotations: make(map[string]string),
	}

	conAllocResp.Envs["LD_LIBRARY_PATH"] = "/usr/local/nvidia/lib64"
	conAllocResp.Envs["NVIDIA_VISIBLE_DEVICES"] = "0"
	conAllocResp.Mounts = append(conAllocResp.Mounts, &pluginapi.Mount{
		ContainerPath: "/usr/local/nvidia",
		HostPath:      "/usr/local/nvidia",
		ReadOnly:      true,
	})
	conAllocResp.Devices = append(conAllocResp.Devices, &pluginapi.DeviceSpec{
		ContainerPath: "/dev/nvidiactl",
		HostPath:      "/dev/nvidiactl",
		Permissions:   "rwm",
	})
	conAllocResp.Devices = append(conAllocResp.Devices, &pluginapi.DeviceSpec{
		ContainerPath: "/dev/nvidia-uvm",
		HostPath:      "/dev/nvidia-uvm",
		Permissions:   "rwm",
	})
	conAllocResp.Devices = append(conAllocResp.Devices, &pluginapi.DeviceSpec{
		ContainerPath: "/dev/nvidia-uvm-tools",
		HostPath:      "/dev/nvidia-uvm-tools",
		Permissions:   "rwm",
	})
	//根据调度器分配的uuid，或者gpuid，再决定挂哪个设备上去
	conAllocResp.Devices = append(conAllocResp.Devices, &pluginapi.DeviceSpec{
		ContainerPath: "/dev/nvidia0",
		HostPath:      "/dev/nvidia0",
		Permissions:   "rwm",
	})

	confirmPod.Annotations[AnnAssignedFlag] = "true"
	err := vr.kubeMessenger.UpdatePodAnnotations(&confirmPod)
	if err != nil {
		fmt.Printf("Failed to update pod annotation for pod %s on ns %s.", confirmPod.Name, confirmPod.Namespace)
		return nil, errors.New("failed to update pod annotation")
	}

	allocResp.ContainerResponses = append(allocResp.ContainerResponses, conAllocResp)

	return allocResp, nil
}

func (vr *VcoreResourceServer) ListAndWatch(e *pluginapi.Empty, s pluginapi.DevicePlugin_ListAndWatchServer) error {
	klog.V(2).Infof("ListAndWatch request for vcore")
	fmt.Println("ListAndWatch request for vcore")

	devs := make([]*pluginapi.Device, 0)
	devs = vr.getNvidiaDevice()

	s.Send(&pluginapi.ListAndWatchResponse{Devices: devs})

	// We don't send unhealthy state
	for {
		time.Sleep(time.Second)
	}

	return nil
}

func (vr *VcoreResourceServer) GetDevicePluginOptions(ctx context.Context, e *pluginapi.Empty) (*pluginapi.DevicePluginOptions, error) {
	klog.V(2).Infof("GetDevicePluginOptions request for vcore")
	return &pluginapi.DevicePluginOptions{PreStartRequired: true}, nil
}

func (vr *VcoreResourceServer) PreStartContainer(ctx context.Context, req *pluginapi.PreStartContainerRequest) (*pluginapi.PreStartContainerResponse, error) {
	klog.V(2).Infof("PreStartContainer request for vcore")
	return &pluginapi.PreStartContainerResponse{}, nil
}

func (vr *VcoreResourceServer) GetPreferredAllocation(context.Context, *pluginapi.PreferredAllocationRequest) (*pluginapi.PreferredAllocationResponse, error) {
	return &pluginapi.PreferredAllocationResponse{}, nil
}

func (vr *VcoreResourceServer) getMluDevice() []*pluginapi.Device {
	devs := make([]*pluginapi.Device, 0)

	mluDevices := &pluginapi.Device{
		ID:     fmt.Sprintf("%s-%d", vr.resourceName, 0),
		Health: pluginapi.Healthy,
	}

	devs = append(devs, mluDevices)

	return devs
}

func (vr *VcoreResourceServer) getNvidiaDevice() []*pluginapi.Device {

	devs := make([]*pluginapi.Device, 0)
	n, _ := vr.devInfo.GetInfo()
	for i := int(0); i < n; i++ {
		gpuDevices := &pluginapi.Device{
			ID:     fmt.Sprintf("%s-%d", ResourceCore, i),
			Health: pluginapi.Healthy,
		}
		devs = append(devs, gpuDevices)
	}

	return devs
}

func getVCUDAReadyFromPodAnnotation(pod *v1.Pod) string {
	ready := ""
	if len(pod.ObjectMeta.Annotations) > 0 {
		value, found := pod.ObjectMeta.Annotations[AnnVCUDAReady]
		if found {
			ready = value
		} else {
			fmt.Printf("Failed to get vcuda flag for pod %s in ns %s.\n", pod.Name, pod.Namespace)
		}
	}
	return ready
}

func allocFakeAnnotationForTest()
