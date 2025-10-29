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

	metadata := &rpcmetadata.ResponseRpcMetadata{}
	err := DecodePayloadMetadata(msg, metadata)
	if err != nil {
		return nil, fmt.Errorf("unable to decode ResponseRpcMetadata: %w", err)
	}

	dataBytes, err := MaybeDecompress(msg.Data(), metadata.GetCompression())
	if err != nil {
		return nil, fmt.Errorf("response payload decompression failed: %w", err)
	}
	result := &responsePayload{metadata: metadata, data: dataBytes}
	if metadata.PayloadMetadata != nil && metadata.PayloadMetadata.ExceptionMetadata != nil {
		exception := newRocketException(metadata.PayloadMetadata.ExceptionMetadata)
		if exception.ExceptionType == RocketExceptionAppUnknown {
			// a.k.a. undeclared exception (ApplicaitonException)
			return result, types.NewApplicationException(types.UNKNOWN_APPLICATION_EXCEPTION, exception.What)
		} else if !exception.IsDeclared() {
			exception.SerializedException = dataBytes
			return result, exception
		}
	}
	return result, nil
}

// EncodeResponsePayload encodes a response payload.
func EncodeResponsePayload(
	headers map[string]string,
	compression rpcmetadata.CompressionAlgorithm,
	dataBytes []byte,
) (payload.Payload, error) {
	responseMetadata := rpcmetadata.NewPayloadResponseMetadata()
	payloadMetadata := rpcmetadata.NewPayloadMetadata().
		SetResponseMetadata(responseMetadata)

	metadata := rpcmetadata.NewResponseRpcMetadata().
		SetOtherMetadata(headers).
		SetCompression(&compression).
		SetPayloadMetadata(payloadMetadata)

	return EncodePayloadMetadataAndData(metadata, dataBytes, compression)
}

// EncodeResponseApplicationErrorPayload encodes a response error payload.
func EncodeResponseApplicationErrorPayload(
	appException *types.ApplicationException,
	headers map[string]string,
	compression rpcmetadata.CompressionAlgorithm,
) (payload.Payload, error) {
	exceptionMetadata := NewPayloadExceptionMetadataBase(
		"ApplicationException",
		appException.Error(),
		RocketExceptionAppUnknown,
		rpcmetadata.ErrorKind_UNSPECIFIED,
		rpcmetadata.ErrorBlame_UNSPECIFIED,
		rpcmetadata.ErrorSafety_UNSPECIFIED,
	)
	payloadMetadata := rpcmetadata.NewPayloadMetadata().
		SetExceptionMetadata(exceptionMetadata)

	metadata := rpcmetadata.NewResponseRpcMetadata().
		SetOtherMetadata(headers).
		SetCompression(&compression).
		SetPayloadMetadata(payloadMetadata)

	return EncodePayloadMetadataAndData(metadata, nil /* dataBytes */, compression)
}
