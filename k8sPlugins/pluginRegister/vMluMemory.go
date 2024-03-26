package pluginRegister

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

type MluMemSrv struct {
	srv          *grpc.Server
	socketFile   string
	resourceName string
	devInfo      *deviceInfo.MluInfo
}

var _ pluginapi.DevicePluginServer = &MluMemSrv{}
var _ ResourceServer = &MluMemSrv{}

func NewMluMemSrv(kubeMessenger *podWatch.KubeMessenger, devInfo *deviceInfo.MluInfo) ResourceServer {
	socketFile := filepath.Join(pluginapi.DevicePluginPath, CambriconMemSocketName)

	return &MluMemSrv{
		devInfo:      devInfo,
		socketFile:   socketFile,
		resourceName: MluResourceMemory,
	}
}

func (vm *MluMemSrv) SocketName() string {

	return vm.socketFile
}

func (vm *MluMemSrv) ResourceName() string {

	return vm.resourceName
}

func (vm *MluMemSrv) Stop() {
	vm.srv.Stop()
}

func (vm *MluMemSrv) Run() error {
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

func (vm *MluMemSrv) GetDevicePluginOptions(ctx context.Context, e *pluginapi.Empty) (*pluginapi.DevicePluginOptions, error) {
	return &pluginapi.DevicePluginOptions{}, nil
}

func (vm *MluMemSrv) ListAndWatch(e *pluginapi.Empty, s pluginapi.DevicePlugin_ListAndWatchServer) error {
	devs := make([]*pluginapi.Device, 0)

	vm.devInfo.GetInfo()
	var totalMemOnNode int64
	for _, v := range vm.devInfo.GetMemInfo() {
		totalMemOnNode += v
	}
	memResourceCount := totalMemOnNode / MemoryBlockSize //256M = 268435456Bytes
	for i := int64(0); i < memResourceCount; i++ {
		memDevices := &pluginapi.Device{
			ID:     fmt.Sprintf("%s-%d", MluResourceMemory, i),
			Health: pluginapi.Healthy,
		}
		devs = append(devs, memDevices)
	}

	s.Send(&pluginapi.ListAndWatchResponse{Devices: devs})

	return nil
}

func (vm *MluMemSrv) GetPreferredAllocation(ctx context.Context, p *pluginapi.PreferredAllocationRequest) (*pluginapi.PreferredAllocationResponse, error) {
	return &pluginapi.PreferredAllocationResponse{}, nil
}

func (vm *MluMemSrv) Allocate(ctx context.Context, req *pluginapi.AllocateRequest) (*pluginapi.AllocateResponse, error) {
	return &pluginapi.AllocateResponse{}, nil
}

func (vm *MluMemSrv) PreStartContainer(ctx context.Context, req *pluginapi.PreStartContainerRequest) (*pluginapi.PreStartContainerResponse, error) {
	return &pluginapi.PreStartContainerResponse{}, nil
}
