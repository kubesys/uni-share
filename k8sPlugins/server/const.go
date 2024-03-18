package server

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

	ConfigFileName = "vcuda.config"
	pidFileName    = "pids.config"

	CgroupV1Base = "/sys/fs/cgroup/memory"
	CgroupV2Base = "/sys/fs/cgroup"

	CgroupProcs = "cgroup.procs"

	/** 256MB */
	MemoryBlockSize = 268435456

	PodQOSGuaranteed = "Guaranteed"
	PodQOSBurstable  = "Burstable"
	PodQOSBestEffort = "BestEffort"
)

var (
	DriverVersionMajor int
	DriverVersionMinor int
)
