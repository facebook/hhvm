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
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
	"github.com/rsocket/rsocket-go/payload"
)

// DecodePayloadMetadata decodes payload metadata.
func DecodePayloadMetadata(msg payload.Payload, result types.ReadableStruct) error {
	metadataBytes, ok := msg.Metadata()
	if !ok {
		return fmt.Errorf("payload is missing metadata")
	}

	err := format.DecodeCompact(metadataBytes, result)
	if err != nil {
		return err
	}

	return nil
}

// EncodePayloadMetadataAndData encodes payload metadata and data.
func EncodePayloadMetadataAndData(
	metadata types.WritableStruct,
	dataBytes []byte,
	dataCompression rpcmetadata.CompressionAlgorithm,
) (payload.Payload, error) {
	metadataBytes, err := format.EncodeCompact(metadata)
	if err != nil {
		return nil, err
	}

	if dataBytes == nil {
		return payload.New(nil, metadataBytes), nil
	}

	dataBytes, err = MaybeCompress(dataBytes, dataCompression)
	if err != nil {
		return nil, err
	}
	return payload.New(dataBytes, metadataBytes), nil
}
