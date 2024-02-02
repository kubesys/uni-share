package nvidia

type ResourceServer interface {
	SocketName() string
	ResourceName() string
	Stop()
	Run() error
}
