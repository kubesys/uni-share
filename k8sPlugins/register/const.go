package register

const (
	VcudaSocketName    = "vcuda.sock"
	VirtualManagerPath = "/etc/unishare/vm"

	ResourceMemory    = "iscas.cn/gpu-memory"
	ResourceCore      = "doslab.io/gpu-core"
	MluResourceMemory = "iscas.cn/mlu-memory"
	MluResourceCore   = "iscas.cn/mlu-core"

	AnnAssumeTime   = "doslab.io/gpu-assume-time"
	AnnAssignedFlag = "doslab.io/gpu-assigned"
	//AnnResourceUUID       = "doslab.io/gpu-uuid"
	AnnVCUDAReady = "doslab.io/vcuda"

	/** 256MB */
	MemoryBlockSize = 268435456

	CambriconCoreSocketName = "mlucore.scok"
	CambriconMemSocketName  = "mlumem.sock"
	NvidiaCoreSocketName    = "nvidiacore.sock"
	NvidiaMemSocketName     = "nvidiamem.sock"
)
