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

package rocket

import (
	"fmt"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/format"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
	"github.com/rsocket/rsocket-go/payload"
)

// DecodeServerMetadataPush decodes the server metadata push.
func DecodeServerMetadataPush(msg payload.Payload) (*rpcmetadata.ServerPushMetadata, error) {
	msg = payload.Clone(msg)

	// For documentation/reference see the CPP implementation
	// https://www.internalfb.com/code/fbsource/[ec968d3ea0ab]/fbcode/thrift/lib/cpp2/transport/rocket/client/RocketClient.cpp?lines=181
	metadata := &rpcmetadata.ServerPushMetadata{}
	err := DecodePayloadMetadata(msg, metadata)
	if err != nil {
		return nil, fmt.Errorf("unable to decode ServerPushMetadata: %w", err)
	}
	if err := validateServerPushMetadata(metadata); err != nil {
		return nil, err
	}
	return metadata, nil
}

// EncodeServerMetadataPush encodes the server metadata push.
func EncodeServerMetadataPush(zstdSupported bool) (payload.Payload, error) {
	version := int32(8)
	res := rpcmetadata.NewServerPushMetadata().
		SetSetupResponse(
			rpcmetadata.NewSetupResponse().
				SetVersion(&version).
				SetZstdSupported(&zstdSupported),
		)
	metadataBytes, err := format.EncodeCompact(res)
	if err != nil {
		return nil, fmt.Errorf("unable to serialize metadata push %w", err)
	}
	return payload.New(nil, metadataBytes), nil
}

func validateServerPushMetadata(metadata *rpcmetadata.ServerPushMetadata) error {
	minVersion := int32(8)
	maxVersion := int32(8)

	if metadata.SetupResponse != nil {
		if metadata.SetupResponse.Version != nil {
			version := *metadata.SetupResponse.Version
			if version < minVersion || version > maxVersion {
				return fmt.Errorf("unsupported rocket protocol version: %d", version)
			}
		}
	} else if metadata.StreamHeadersPush != nil {
		return fmt.Errorf("unsupported StreamHeadersPush metadata type")
	}
	return nil
}
