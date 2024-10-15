/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

// GetServiceContexts returns Thrift service context references.
func GetServiceContexts(md *metadata.ThriftMetadata) []*metadata.ThriftServiceContextRef {
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

// GetServiceContexts returns Thrift service context references for all added Processors.
func (tm *ThriftMetadataCombiner) GetServiceContexts() []*metadata.ThriftServiceContextRef {
	md := tm.GetCombinedThriftMetadata()
	return GetServiceContexts(md)
}

// GetThriftServiceMetadataResponse returns a Thrift service metadata response.
func GetThriftServiceMetadataResponse(thriftMetadata *metadata.ThriftMetadata) *metadata.ThriftServiceMetadataResponse {
	serviceContexts := GetServiceContexts(thriftMetadata)
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

// GetThriftServiceMetadataResponse returns a Thrift service metadata response for all added Processors.
func (tm *ThriftMetadataCombiner) GetThriftServiceMetadataResponse() *metadata.ThriftServiceMetadataResponse {
	thriftMetadata := tm.GetCombinedThriftMetadata()
	return GetThriftServiceMetadataResponse(thriftMetadata)
}
