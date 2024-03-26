package pluginRegister

import (
	"fmt"
	"uni-share/deviceInfo"
	"uni-share/podWatch"
)

type ResourceServer interface {
	SocketName() string
	ResourceName() string
	Stop()
	Run() error
}

type resourceCreator func(kubeMessenger *podWatch.KubeMessenger, devInfo *deviceInfo.GpuInfo) ResourceServer //定义了resourceCreator，指向(参数)返回值类型的函数

type resourceFactory struct {
	creators map[string]resourceCreator
}

func NewresourceFactory() *resourceFactory {
	return &resourceFactory{
		creators: map[string]resourceCreator{
			"nvidiaCore": NewVcoreResourceServer, //
			"nvidiaMem":  NewVmemResourceServer,
			//"cambriconCore": ,
			//"cambriconMem": ,
		},
	}
}

func (f *resourceFactory) CreateResource(name string, kubeMessenger *podWatch.KubeMessenger, devInfo *deviceInfo.GpuInfo) (ResourceServer, error) { //CreateResource的参数要包含具体类构造函数的参数
	resourceCreator, ok := f.creators[name]
	if !ok {
		return nil, fmt.Errorf("resource type: %s is not supported yet", name)
	}

	return resourceCreator(kubeMessenger, devInfo), nil
}
