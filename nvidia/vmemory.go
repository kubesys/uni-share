package nvidia

import (
	"context"
	"fmt"
	"net"
	"os"
	"path/filepath"
	"syscall"
	"uni-share/deviceInfo"
	"uni-share/podWatch"

	"google.golang.org/grpc"
	pluginapi "k8s.io/kubelet/pkg/apis/deviceplugin/v1beta1"
)

type VmemResourceServer struct {
	srv          *grpc.Server
	socketFile   string
	resourceName string
	devInfo      *deviceInfo.GpuInfo
}

var _ pluginapi.DevicePluginServer = &VmemResourceServer{}
var _ ResourceServer = &VmemResourceServer{}

func NewVmemResourceServer(kubeMessenger *podWatch.KubeMessenger, devInfo *deviceInfo.GpuInfo) ResourceServer {
	socketFile := filepath.Join(pluginapi.DevicePluginPath, NvidiaMemSocketName)

	return &VmemResourceServer{
		devInfo:      devInfo,
		socketFile:   socketFile,
		resourceName: ResourceMemory,
	}
}

func (vm *VmemResourceServer) SocketName() string {

	return vm.socketFile
}

func (vm *VmemResourceServer) ResourceName() string {

	return vm.resourceName
}

func (vm *VmemResourceServer) Stop() {
	vm.srv.Stop()
}

func (vm *VmemResourceServer) Run() error {
	vm.srv = grpc.NewServer()
	pluginapi.RegisterDevicePluginServer(vm.srv, vm)

	err := syscall.Unlink(vm.socketFile)
	if err != nil && !os.IsNotExist(err) {
		return err
	}

	l, err := net.Listen("unix", vm.socketFile)
	if err != nil {
		return err
	}

	return vm.srv.Serve(l)

}

func (vm *VmemResourceServer) GetDevicePluginOptions(ctx context.Context, e *pluginapi.Empty) (*pluginapi.DevicePluginOptions, error) {
	return &pluginapi.DevicePluginOptions{}, nil
}

func (vm *VmemResourceServer) ListAndWatch(e *pluginapi.Empty, s pluginapi.DevicePlugin_ListAndWatchServer) error {
	devs := make([]*pluginapi.Device, 0)

	memInfo := vm.devInfo.GetMemInfo()
	var totalMemOnNode uint64
	for _, v := range memInfo {
		totalMemOnNode += v.Total
	}
	memResourceCount := totalMemOnNode / 268435456 //256M = 268435456Bytes
	for i := uint64(0); i < memResourceCount; i++ {
		memDevices := &pluginapi.Device{
			ID:     fmt.Sprintf("%s-%d", ResourceMemory, i),
			Health: pluginapi.Healthy,
		}
		devs = append(devs, memDevices)
	}

	s.Send(&pluginapi.ListAndWatchResponse{Devices: devs})

	return nil
}

func (vm *VmemResourceServer) GetPreferredAllocation(ctx context.Context, p *pluginapi.PreferredAllocationRequest) (*pluginapi.PreferredAllocationResponse, error) {
	return &pluginapi.PreferredAllocationResponse{}, nil
}

func (vm *VmemResourceServer) Allocate(ctx context.Context, req *pluginapi.AllocateRequest) (*pluginapi.AllocateResponse, error) {
	return &pluginapi.AllocateResponse{}, nil
}

func (vm *VmemResourceServer) PreStartContainer(ctx context.Context, req *pluginapi.PreStartContainerRequest) (*pluginapi.PreStartContainerResponse, error) {
	return &pluginapi.PreStartContainerResponse{}, nil
}
