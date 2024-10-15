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

// GetThriftServiceMetadataResponse returns a Thrift service metadata response.
func GetThriftServiceMetadataResponse(metadataModuleName string, thriftMetadata *metadata.ThriftMetadata) *metadata.ThriftServiceMetadataResponse {
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
						SetName(metadataModuleName),
				),
		).
		SetMetadata(thriftMetadata).
		SetServices(serviceContexts)
}
