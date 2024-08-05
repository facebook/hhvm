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
	"github.com/rsocket/rsocket-go/payload"
)

type responsePayload struct {
	metadata  *ResponseRpcMetadata
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

func (r *responsePayload) Zstd() bool {
	return r.metadata != nil && r.metadata.GetCompression() == CompressionAlgorithm_ZSTD
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
		return &responsePayload{metadata: &ResponseRpcMetadata{}, data: []byte{}}, nil
	}
	res := &responsePayload{metadata: &ResponseRpcMetadata{}, data: msg.Data()}
	var err error
	metadataBytes, ok := msg.Metadata()
	if ok {
		if err := deserializeCompact(metadataBytes, res.metadata); err != nil {
			return nil, err
		}
		if res.Zstd() {
			res.data, err = decompressZstd(res.data)
			if err != nil {
				return nil, err
			}
		}
		if res.metadata.PayloadMetadata != nil && res.metadata.PayloadMetadata.ExceptionMetadata != nil {
			res.exception = newRocketException(res.metadata.PayloadMetadata.ExceptionMetadata)
		}
	}
	return res, res.Error()
}

func encodeResponsePayload(name string, messageType MessageType, headers map[string]string, zstd bool, dataBytes []byte) (payload.Payload, error) {
	metadata := NewResponseRpcMetadata()
	metadata.SetOtherMetadata(headers)
	if zstd {
		compression := CompressionAlgorithm_ZSTD
		metadata.SetCompression(&compression)
	}
	if messageType == EXCEPTION {
		excpetionMetadata := newUnknownPayloadExceptionMetadataBase(name, string(dataBytes))
		metadata.SetPayloadMetadata(NewPayloadMetadata().SetExceptionMetadata(excpetionMetadata))
	}
	metadataBytes, err := serializeCompact(metadata)
	if err != nil {
		return nil, err
	}
	if messageType == EXCEPTION {
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
