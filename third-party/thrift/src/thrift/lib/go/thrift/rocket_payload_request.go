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
	"maps"

	"github.com/rsocket/rsocket-go/payload"
)

type requestPayload struct {
	metadata *RequestRpcMetadata
	data     []byte
	typeID   MessageType
	protoID  ProtocolID
}

func encodeRequestPayload(name string, protoID ProtocolID, typeID MessageType, headers map[string]string, persistentHeaders map[string]string, zstd bool, dataBytes []byte) (payload.Payload, error) {
	metadata := NewRequestRpcMetadata()
	metadata.SetName(&name)
	rpcProtocolID, err := protocolIDToRPCProtocolID(protoID)
	if err != nil {
		return nil, err
	}
	metadata.SetProtocol(&rpcProtocolID)
	kind, err := messageTypeToRPCKind(typeID)
	if err != nil {
		return nil, err
	}
	metadata.SetKind(&kind)
	if zstd {
		compression := CompressionAlgorithm_ZSTD
		metadata.SetCompression(&compression)
	}
	metadata.OtherMetadata = make(map[string]string)
	maps.Copy(metadata.OtherMetadata, headers)
	maps.Copy(metadata.OtherMetadata, persistentHeaders)
	metadataBytes, err := serializeCompact(metadata)
	if err != nil {
		return nil, err
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

func decodeRequestPayload(msg payload.Payload) (*requestPayload, error) {
	msg = payload.Clone(msg)
	res := &requestPayload{data: msg.Data()}
	var err error
	metadataBytes, ok := msg.Metadata()
	if ok {
		metadata := &RequestRpcMetadata{}
		if err := deserializeCompact(metadataBytes, metadata); err != nil {
			return nil, err
		}
		res.metadata = metadata
		if res.Zstd() {
			res.data, err = decompressZstd(res.data)
			if err != nil {
				return nil, err
			}
		}
		res.typeID, err = rpcKindToMessageType(metadata.GetKind())
		if err != nil {
			return nil, err
		}
		res.protoID, err = rpcProtocolIDToProtocolID(metadata.GetProtocol())
		if err != nil {
			return nil, err
		}
	}
	return res, nil
}

func (r *requestPayload) Data() []byte {
	return r.data
}

func (r *requestPayload) HasMetadata() bool {
	return r.metadata != nil
}

func (r *requestPayload) Name() string {
	if r.metadata == nil {
		return ""
	}
	return r.metadata.GetName()
}

func (r *requestPayload) TypeID() MessageType {
	return r.typeID
}

func (r *requestPayload) ProtoID() ProtocolID {
	return r.protoID
}

func (r *requestPayload) Zstd() bool {
	return r.metadata != nil && r.metadata.GetCompression() == CompressionAlgorithm_ZSTD
}

func (r *requestPayload) Headers() map[string]string {
	if r.metadata == nil {
		return nil
	}
	return r.metadata.GetOtherMetadata()
}

func protocolIDToRPCProtocolID(protocolID ProtocolID) (ProtocolId, error) {
	switch protocolID {
	case ProtocolIDBinary:
		return ProtocolId_BINARY, nil
	case ProtocolIDCompact:
		return ProtocolId_COMPACT, nil
	}
	return 0, fmt.Errorf("unsupported ProtocolID %v", protocolID)
}

func rpcProtocolIDToProtocolID(protocolID ProtocolId) (ProtocolID, error) {
	switch protocolID {
	case ProtocolId_BINARY:
		return ProtocolIDBinary, nil
	case ProtocolId_COMPACT:
		return ProtocolIDCompact, nil
	}
	return 0, fmt.Errorf("unsupported ProtocolId %v", protocolID)
}

func messageTypeToRPCKind(typeID MessageType) (RpcKind, error) {
	switch typeID {
	case CALL:
		return RpcKind_SINGLE_REQUEST_SINGLE_RESPONSE, nil
	case ONEWAY:
		return RpcKind_SINGLE_REQUEST_NO_RESPONSE, nil
	}
	return 0, fmt.Errorf("unsupported MessageType %v", typeID)
}

func rpcKindToMessageType(kind RpcKind) (MessageType, error) {
	switch kind {
	case RpcKind_SINGLE_REQUEST_SINGLE_RESPONSE:
		return CALL, nil
	case RpcKind_SINGLE_REQUEST_NO_RESPONSE:
		return ONEWAY, nil
	}
	return 0, fmt.Errorf("unsupported RpcKind %v", kind)
}
