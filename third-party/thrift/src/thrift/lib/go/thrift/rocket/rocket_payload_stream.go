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

	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
	"github.com/rsocket/rsocket-go/payload"
)

type streamPayload struct {
	metadata *rpcmetadata.StreamPayloadMetadata
	data     []byte
}

func (r *streamPayload) Data() []byte {
	return r.data
}

// DecodeStreamPayload decodes a stream payload.
func DecodeStreamPayload(msg payload.Payload) (*streamPayload, error) {
	msg = payload.Clone(msg)

	metadata := &rpcmetadata.StreamPayloadMetadata{}
	err := DecodePayloadMetadata(msg, metadata)
	if err != nil {
		return nil, fmt.Errorf("unable to decode StreamPayloadMetadata: %w", err)
	}

	dataBytes, err := MaybeDecompress(msg.Data(), metadata.GetCompression())
	if err != nil {
		return nil, fmt.Errorf("stream payload decompression failed: %w", err)
	}
	result := &streamPayload{metadata: metadata, data: dataBytes}
	if metadata.PayloadMetadata != nil && metadata.PayloadMetadata.ExceptionMetadata != nil {
		exception := newRocketException(metadata.PayloadMetadata.ExceptionMetadata)
		if !exception.IsDeclared() {
			exception.SerializedException = dataBytes
			return result, exception
		}
	}
	return result, nil
}
