package metadata

import (
	"github.com/facebook/fbthrift/thrift/lib/thrift/metadata"
)

// ProcessorWithMetadata is an interface for Processors
// that are able to provide their Thrift metadata.
type ProcessorWithMetadata interface {
	GetThriftMetadata() *metadata.ThriftMetadata
}

// ThriftMetadataCombiner combines metadata from multiple Processors.
type ThriftMetadataCombiner struct {
	processors []ProcessorWithMetadata
}

// NewThriftMetadataCombiner creates a new Thrift metadata combiner.
func NewThriftMetadataCombiner() *ThriftMetadataCombiner {
	return &ThriftMetadataCombiner{}
}

// AddProcessor adds a Processor to the Thrift metadata.
func (tm *ThriftMetadataCombiner) AddProcessor(p ProcessorWithMetadata) {
	tm.processors = append(tm.processors, p)
}

// GetCombinedThriftMetadata returns Thrift metadata combined from all added Processors.
func (tm *ThriftMetadataCombiner) GetCombinedThriftMetadata() *metadata.ThriftMetadata {
	allServices := make(map[string]*metadata.ThriftService)
	allEnums := make(map[string]*metadata.ThriftEnum)
	allStructs := make(map[string]*metadata.ThriftStruct)
	allExceptions := make(map[string]*metadata.ThriftException)

	for _, processor := range tm.processors {
		md := processor.GetThriftMetadata()

		for serviceName, thriftService := range md.GetServices() {
			allServices[serviceName] = thriftService
		}
		for enumName, thriftEnum := range md.GetEnums() {
			allEnums[enumName] = thriftEnum
		}
		for structName, thriftStruct := range md.GetStructs() {
			allStructs[structName] = thriftStruct
		}
		for exceptionName, thriftException := range md.GetExceptions() {
			allExceptions[exceptionName] = thriftException
		}
	}

	return metadata.NewThriftMetadata().
		SetEnums(allEnums).
		SetStructs(allStructs).
		SetExceptions(allExceptions).
		SetServices(allServices)
}

// GetServiceContexts returns Thrift service context references for all added Processors.
func (tm *ThriftMetadataCombiner) GetServiceContexts() []*metadata.ThriftServiceContextRef {
	md := tm.GetCombinedThriftMetadata()

	serviceContexts := make([]*metadata.ThriftServiceContextRef, 0, len(md.GetServices()))
	for serviceName := range md.GetServices() {
		sc := metadata.NewThriftServiceContextRef().
			SetServiceName(serviceName).
			SetModule(
				metadata.NewThriftModuleContext().
					SetName(serviceName),
			)
		serviceContexts = append(serviceContexts, sc)
	}

	return serviceContexts
}

// GetThriftServiceMetadataResponse returns a Thrift service metadata response for all added Processors.
func (tm *ThriftMetadataCombiner) GetThriftServiceMetadataResponse() *metadata.ThriftServiceMetadataResponse {
	thriftMetadata := tm.GetCombinedThriftMetadata()
	serviceContexts := tm.GetServiceContexts()

	return metadata.NewThriftServiceMetadataResponse().
		SetContext(
			metadata.NewThriftServiceContext().
				SetServiceInfo(
					metadata.NewThriftService().
						SetName("").
						SetFunctions(nil),
				).
				SetModule(
					metadata.NewThriftModuleContext().
						SetName("thwork"),
				),
		).
		SetMetadata(thriftMetadata).
		SetServices(serviceContexts)
}
