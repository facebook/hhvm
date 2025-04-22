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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
	"github.com/rsocket/rsocket-go/payload"
)

type responsePayload struct {
	metadata  *rpcmetadata.ResponseRpcMetadata
	exception *rocketException
	data      []byte
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

func (r *responsePayload) Error() error {
	if r.exception != nil && !r.exception.IsDeclared() {
		return r.exception
	}
	return nil
}

func decodeResponsePayload(msg payload.Payload) (*responsePayload, error) {
	msg = payload.Clone(msg)
	if msg == nil {
		return &responsePayload{metadata: &rpcmetadata.ResponseRpcMetadata{}, data: []byte{}}, nil
	}
	res := &responsePayload{metadata: &rpcmetadata.ResponseRpcMetadata{}, data: msg.Data()}
	var err error
	metadataBytes, ok := msg.Metadata()
	if ok {
		if err := DecodeCompact(metadataBytes, res.metadata); err != nil {
			return nil, err
		}
		res.data, err = maybeDecompress(res.data, res.metadata.GetCompression())
		if err != nil {
			return nil, fmt.Errorf("response payload decompression failed: %w", err)
		}
		if res.metadata.PayloadMetadata != nil && res.metadata.PayloadMetadata.ExceptionMetadata != nil {
			res.exception = newRocketException(res.metadata.PayloadMetadata.ExceptionMetadata)
		}
	}
	return res, res.Error()
}

func encodeResponsePayload(name string, messageType types.MessageType, headers map[string]string, zstd bool, dataBytes []byte) (payload.Payload, error) {
	metadata := rpcmetadata.NewResponseRpcMetadata()
	metadata.SetOtherMetadata(headers)
	if zstd {
		compression := rpcmetadata.CompressionAlgorithm_ZSTD
		metadata.SetCompression(&compression)
	}
	if messageType == types.EXCEPTION {
		excpetionMetadata := newUnknownPayloadExceptionMetadataBase(name, string(dataBytes))
		metadata.SetPayloadMetadata(rpcmetadata.NewPayloadMetadata().SetExceptionMetadata(excpetionMetadata))
	}
	metadataBytes, err := EncodeCompact(metadata)
	if err != nil {
		return nil, err
	}
	if messageType == types.EXCEPTION {
		return payload.New(nil, metadataBytes), nil
	}

	if zstd {
		dataBytes, err = compressZstd(dataBytes)
		if err != nil {
			return nil, err
		}
	}
	pay := payload.New(dataBytes, metadataBytes)
	return pay, nil
}
