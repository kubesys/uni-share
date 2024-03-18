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
	"uni-share/podWatch"
	"uni-share/register"
	"uni-share/server"

	"github.com/fsnotify/fsnotify"
	"github.com/kubesys/client-go/pkg/kubesys"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	pluginapi "k8s.io/kubelet/pkg/apis/deviceplugin/v1beta1"
)

var (
	//url        = pflag.String("url", "", "https://ip:port")
	//token      = pflag.String("token", "", "master node token")
	serverFlag  = false
	resourceSrv = make(map[string]register.ResourceServer, 0)
)

const (
	url   = "https://133.133.135.73:6443"
	token = "s"
)

func main() {
	//pflag.Parse()

	//fmt.Println("String Flag:", *url)
	//fmt.Println("String Flag:", *token)

	//client := kubesys.NewKubernetesClientWithDefaultKubeConfig()
	client := kubesys.NewKubernetesClient(url, token)
	client.Init()

	podMgr := podWatch.NewPodManager()
	mes := podWatch.NewKubeMessenger(client, "133.133.135.73")
	podWatcher := kubesys.NewKubernetesWatcher(client, podMgr)
	go client.WatchResources("Pod", "", podWatcher)
	virtMgr := server.NewVirtualManager(client, podMgr)
	go virtMgr.Run()

	devInfo := deviceInfo.NewGpuInfo(client)
	devInfo.NvmlTest()
	//自己打上环境变量，从环境变量获取节点名称
	//devInfo.CreateCRD("133.133.135.73")

	rscFactory := register.NewresourceFactory()
	vcoreServer, _ := rscFactory.CreateResource("nvidiaCore", mes, devInfo)
	go vcoreServer.Run()
	vmemServer, _ := rscFactory.CreateResource("nvinvidiaMem", mes, devInfo)
	go vmemServer.Run()
	resourceSrv[register.NvidiaCoreSocketName] = vcoreServer
	resourceSrv[register.NvidiaMemSocketName] = vmemServer

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
