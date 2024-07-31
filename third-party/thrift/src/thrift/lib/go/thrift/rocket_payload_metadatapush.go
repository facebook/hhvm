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

package thrift

import (
	"fmt"

	"github.com/rsocket/rsocket-go/payload"
)

type serverMetadataPayload struct {
	zstd  bool
	drain bool
}

func decodeServerMetadataPushVersion8(msg payload.Payload) (*serverMetadataPayload, error) {
	msg = payload.Clone(msg)
	minVersion := int32(8)
	maxVersion := int32(8)
	res := &serverMetadataPayload{}
	// For documentation/reference see the CPP implementation
	// https://www.internalfb.com/code/fbsource/[ec968d3ea0ab]/fbcode/thrift/lib/cpp2/transport/rocket/client/RocketClient.cpp?lines=181
	metadataBytes, ok := msg.Metadata()
	if !ok {
		return nil, fmt.Errorf("no metadata in server metadata push")
	}
	// Use ServerPushMetadata{} and do not use &ServerPushMetadata{} to ensure stack and avoid heap allocation.
	metadata := ServerPushMetadata{}
	if err := deserializeCompact(metadataBytes, &metadata); err != nil {
		panic(fmt.Errorf("unable to deserialize metadata push into ServerPushMetadata %w", err))
	}
	if metadata.SetupResponse != nil {
		// If zstdSupported is not set (or if false) client SHOULD not use ZSTD compression.
		res.zstd = metadata.SetupResponse.ZstdSupported != nil && *metadata.SetupResponse.ZstdSupported
		if metadata.SetupResponse.Version != nil {
			version := *metadata.SetupResponse.Version
			if version < minVersion || version > maxVersion {
				return nil, fmt.Errorf("unsupported protocol version received in metadata push: %d, we only support versions in range: [%d, %d]", version, minVersion, maxVersion)
			}
		}
	} else if metadata.StreamHeadersPush != nil {
		panic("server metadata push: StreamHeadersPush not implemented")
	} else if metadata.DrainCompletePush != nil {
		res.drain = true
	}
	return res, nil
}
