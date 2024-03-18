package deviceInfo

type device interface {
	CreateCRD(string)
	GetMemInfo() []int64
	GetInfo()
	GetUUID() []string
	GetCount() int
}
