package main

import (
	"fmt"

	"github.com/spf13/pflag"
)

var (
	busId         = pflag.String("bus-id", "", "xx:xx.x")
	podUid        = pflag.String("pod-uid", "", "xx:xx.x")
	containerName = pflag.String("cont-name", "", "xx:xx.x")
	containerId   = pflag.String("cont-id", "", "xx:xx.x")
	//addr          = pflag.String("addr", "", "/etc/unishare/vm/podUID/vcuda.sock")
)

func main() {
	pflag.Parse()
	addr := "/etc/unishare/vm/" + *podUid + "/vcuda.sock"

	fmt.Println(addr, *busId, *podUid, *containerName, *containerId)

	// conn, err := grpc.Dial(addr, grpc.WithInsecure())
	// if err != nil {
	// 	log.Fatalf("Failed to connect: %v", err)
	// }
	// defer conn.Close()

	// client := pb.NewVCUDAServiceClient(conn)
	// in := &pb.VDeviceRequest{
	// 	BusId:         *busId,
	// 	PodUid:        *podUid,
	// 	ContainerName: *containerName,
	// 	ContainerId:   *containerId,
	// }
	// _, err = client.RegisterVDevice(context.TODO(), in)
	// if err != nil {
	// 	log.Fatalf("RegisterVDevice RPC failed: %v", err)
	// }

}
