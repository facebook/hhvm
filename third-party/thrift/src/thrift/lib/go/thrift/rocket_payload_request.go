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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
	"github.com/rsocket/rsocket-go/payload"
)

type requestPayload struct {
	metadata *rpcmetadata.RequestRpcMetadata
	data     []byte
	typeID   types.MessageType
	protoID  types.ProtocolID
}

func encodeRequestPayload(name string, protoID types.ProtocolID, typeID types.MessageType, headers map[string]string, zstd bool, dataBytes []byte) (payload.Payload, error) {
	metadata := rpcmetadata.NewRequestRpcMetadata()
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
		compression := rpcmetadata.CompressionAlgorithm_ZSTD
		metadata.SetCompression(&compression)
	}
	metadata.OtherMetadata = make(map[string]string)
	maps.Copy(metadata.OtherMetadata, headers)
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
		metadata := &rpcmetadata.RequestRpcMetadata{}
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

func (r *requestPayload) TypeID() types.MessageType {
	return r.typeID
}

func (r *requestPayload) ProtoID() types.ProtocolID {
	return r.protoID
}

func (r *requestPayload) Zstd() bool {
	return r.metadata != nil && r.metadata.GetCompression() == rpcmetadata.CompressionAlgorithm_ZSTD
}

func (r *requestPayload) Headers() map[string]string {
	if r.metadata == nil {
		return nil
	}
	return r.metadata.GetOtherMetadata()
}

func protocolIDToRPCProtocolID(protocolID types.ProtocolID) (rpcmetadata.ProtocolId, error) {
	switch protocolID {
	case types.ProtocolIDBinary:
		return rpcmetadata.ProtocolId_BINARY, nil
	case types.ProtocolIDCompact:
		return rpcmetadata.ProtocolId_COMPACT, nil
	}
	return 0, fmt.Errorf("unsupported ProtocolID %v", protocolID)
}

func rpcProtocolIDToProtocolID(protocolID rpcmetadata.ProtocolId) (types.ProtocolID, error) {
	switch protocolID {
	case rpcmetadata.ProtocolId_BINARY:
		return types.ProtocolIDBinary, nil
	case rpcmetadata.ProtocolId_COMPACT:
		return types.ProtocolIDCompact, nil
	}
	return 0, fmt.Errorf("unsupported ProtocolId %v", protocolID)
}

func messageTypeToRPCKind(typeID types.MessageType) (rpcmetadata.RpcKind, error) {
	switch typeID {
	case types.CALL:
		return rpcmetadata.RpcKind_SINGLE_REQUEST_SINGLE_RESPONSE, nil
	case types.ONEWAY:
		return rpcmetadata.RpcKind_SINGLE_REQUEST_NO_RESPONSE, nil
	}
	return 0, fmt.Errorf("unsupported MessageType %v", typeID)
}

func rpcKindToMessageType(kind rpcmetadata.RpcKind) (types.MessageType, error) {
	switch kind {
	case rpcmetadata.RpcKind_SINGLE_REQUEST_SINGLE_RESPONSE:
		return types.CALL, nil
	case rpcmetadata.RpcKind_SINGLE_REQUEST_NO_RESPONSE:
		return types.ONEWAY, nil
	}
	return 0, fmt.Errorf("unsupported RpcKind %v", kind)
}
