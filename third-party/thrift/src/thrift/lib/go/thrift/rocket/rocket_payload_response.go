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

type responsePayload struct {
	metadata *rpcmetadata.ResponseRpcMetadata
	data     []byte
}

func (r *responsePayload) Headers() map[string]string {
	if r.metadata == nil {
		return nil
	}
	return r.metadata.OtherMetadata
}

func (r *responsePayload) Data() []byte {
	return r.data
}

// DecodeResponsePayload decodes a response payload.
func DecodeResponsePayload(msg payload.Payload) (*responsePayload, error) {
	msg = payload.Clone(msg)
	if msg == nil {
		return &responsePayload{metadata: &rpcmetadata.ResponseRpcMetadata{}, data: []byte{}}, nil
	}
	res := &responsePayload{metadata: &rpcmetadata.ResponseRpcMetadata{}, data: msg.Data()}

	metadataBytes, ok := msg.Metadata()
	if !ok {
		return res, nil
	}

	var err error
	if err := format.DecodeCompact(metadataBytes, res.metadata); err != nil {
		return nil, err
	}
	res.data, err = maybeDecompress(res.data, res.metadata.GetCompression())
	if err != nil {
		return nil, fmt.Errorf("response payload decompression failed: %w", err)
	}
	if res.metadata.PayloadMetadata != nil && res.metadata.PayloadMetadata.ExceptionMetadata != nil {
		exception := newRocketException(res.metadata.PayloadMetadata.ExceptionMetadata)
		if !exception.IsDeclared() {
			exception.SerializedException = res.data
			return res, exception
		}
	}
	return res, nil
}

// EncodeResponsePayload encodes a response payload.
func EncodeResponsePayload(
	name string,
	messageType types.MessageType,
	headers map[string]string,
	compression rpcmetadata.CompressionAlgorithm,
	dataBytes []byte,
) (payload.Payload, error) {
	metadata := rpcmetadata.NewResponseRpcMetadata().
		SetOtherMetadata(headers).
		SetCompression(&compression)

	if messageType == types.EXCEPTION {
		excpetionMetadata := newUnknownPayloadExceptionMetadataBase(name, string(dataBytes))
		metadata.SetPayloadMetadata(rpcmetadata.NewPayloadMetadata().SetExceptionMetadata(excpetionMetadata))
	}
	metadataBytes, err := format.EncodeCompact(metadata)
	if err != nil {
		return nil, err
	}
	if messageType == types.EXCEPTION {
		return payload.New(nil, metadataBytes), nil
	}

	dataBytes, err = maybeCompress(dataBytes, compression)
	if err != nil {
		return nil, err
	}

	pay := payload.New(dataBytes, metadataBytes)
	return pay, nil
}
