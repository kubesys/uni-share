package main

import (
	"fmt"
	"log"
	"net"
	"os"
	"path"
	"syscall"
	"time"
	"uni-share/deviceInfo"
	"uni-share/pluginRegister"
	"uni-share/podWatch"
	"uni-share/resourceManager"

	"github.com/fsnotify/fsnotify"
	"github.com/kubesys/client-go/pkg/kubesys"
	"github.com/spf13/pflag"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	pluginapi "k8s.io/kubelet/pkg/apis/deviceplugin/v1beta1"
)

var (
	url         = pflag.String("url", "", "https://ip:port")
	token       = pflag.String("token", "", "master node token")
	serverFlag  = false
	resourceSrv = make(map[string]pluginRegister.ResourceServer, 0)
)

func main() {
	pflag.Parse()

	if *url == "" || *token == "" {
		log.Fatal("get url token error")
	}

	//client := kubesys.NewKubernetesClientWithDefaultKubeConfig()
	client := kubesys.NewKubernetesClient(*url, *token)
	client.Init()

	env := os.Getenv("DAEMON_NODE_NAME")
	podMgr := podWatch.NewPodManager()
	mes := podWatch.NewKubeMessenger(client, env)
	podWatcher := kubesys.NewKubernetesWatcher(client, podMgr)
	go client.WatchResources("Pod", "", podWatcher)
	virtMgr := resourceManager.NewVirtualManager(client, podMgr)
	go virtMgr.Run()

	devInfo := deviceInfo.NewGpuInfo(client)
	devInfo.NvmlTest()
	devInfo.CreateCRD(env)

	rscFactory := pluginRegister.NewresourceFactory()
	vcoreServer, _ := rscFactory.CreateResource("nvidiaCore", mes, devInfo)
	go vcoreServer.Run()
	vmemServer, _ := rscFactory.CreateResource("nvidiaMem", mes, devInfo)
	go vmemServer.Run()
	resourceSrv[pluginRegister.NvidiaCoreSocketName] = vcoreServer
	resourceSrv[pluginRegister.NvidiaMemSocketName] = vmemServer

	//结束处理
	sigChan := NewOSWatcher(syscall.SIGHUP, syscall.SIGINT, syscall.SIGTERM) //注册信号处理函数，接受到这几个信号将信号通知发送给sigChan
	go func() {
		select {
		case sig := <-sigChan:
			vcoreServer.Stop()
			log.Fatalf("Received signal %v, shutting down.", sig)
		}
	}()
	//处理kubelet重启
	watcher, err := NewFileWatcher(pluginapi.DevicePluginPath) //监视/var/lib/kubelet/device-plugins下的文件
	if err != nil {
		log.Fatalf("Failed to created file watcher: %s.", err)
	}

restart:

	if serverFlag {
		vcoreServer.Stop()
		if err := os.Remove(pluginapi.DevicePluginPath + "core.sock"); err != nil && !os.IsNotExist(err) {
			log.Fatalf("Failed to remove sock: %s.", err)
		}
		go vcoreServer.Run()

	}

	err = registerToKubelet()
	if err != nil {
		serverFlag = false
		goto restart
	}

	for {
		select {
		case event := <-watcher.Events:
			if event.Name == pluginapi.KubeletSocket && event.Op&fsnotify.Create == fsnotify.Create {
				//log.Infof("Inotify: %s created, restarting.", pluginapi.KubeletSocket)
				serverFlag = true
				goto restart
			}
		case err := <-watcher.Errors:
			//log.Warningf("Inotify: %s", err)
			fmt.Println(err)
		}
	}
}

func registerToKubelet() error {
	socketFile := "/var/lib/kubelet/device-plugins/kubelet.sock"
	dialOptions := []grpc.DialOption{grpc.WithInsecure(), grpc.WithDialer(UnixDial), grpc.WithBlock(), grpc.WithTimeout(time.Second * 5)}

	conn, err := grpc.Dial(socketFile, dialOptions...)
	if err != nil {
		return err
	}
	defer conn.Close()

	client := pluginapi.NewRegistrationClient(conn)

	for _, v := range resourceSrv {
		req := &pluginapi.RegisterRequest{
			Version:      pluginapi.Version,
			Endpoint:     path.Base(v.SocketName()),
			ResourceName: v.ResourceName(),
			Options:      &pluginapi.DevicePluginOptions{PreStartRequired: true},
		}
		fmt.Println("Register to kubelet with endpoint %s", req.Endpoint)
		_, err = client.Register(context.Background(), req)
		if err != nil {
			return err
		}
	}

	return nil
}

func UnixDial(addr string, timeout time.Duration) (net.Conn, error) {
	return net.DialTimeout("unix", addr, timeout)
}
