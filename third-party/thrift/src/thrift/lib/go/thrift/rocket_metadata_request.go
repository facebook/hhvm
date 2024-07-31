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
)

// requestRPCMetadata is a thrift library version of the generated RequestRpcMetadata
type requestRPCMetadata struct {
	Name    string
	TypeID  MessageType
	ProtoID ProtocolID
	Zstd    bool
	Other   map[string]string
}

// serializeRequestRPCMetadata sets the given arguments into a RequestRpcMetadata struct and serializes it into bytes
func serializeRequestRPCMetadata(request *requestRPCMetadata) ([]byte, error) {
	metadata, err := newRPCMetadataRequest(request)
	if err != nil {
		return nil, err
	}
	compactSerializer := NewCompactSerializer()
	return compactSerializer.Write(metadata)
}

// deserializeRequestRPCMetadata deserializes the given bytes into a ResponseRpcMetadata struct
func deserializeRequestRPCMetadata(metadataBytes []byte) (*requestRPCMetadata, error) {
	metadata := &RequestRpcMetadata{}
	compactDeserializer := NewCompactDeserializer()
	err := compactDeserializer.Read(metadata, metadataBytes)
	if err != nil {
		return nil, err
	}
	return newRequestRPCMetadata(metadata)
}

func newRequestRPCMetadata(request *RequestRpcMetadata) (*requestRPCMetadata, error) {
	name := request.GetName()
	typeID, err := rpcKindToMessageType(request.GetKind())
	if err != nil {
		return nil, err
	}
	protoID, err := rpcProtocolIDToProtocolID(request.GetProtocol())
	if err != nil {
		return nil, err
	}
	zstd := request.GetCompression() == CompressionAlgorithm_ZSTD
	return &requestRPCMetadata{
		Name:    name,
		TypeID:  typeID,
		ProtoID: protoID,
		Zstd:    zstd,
		Other:   request.OtherMetadata,
	}, nil
}

func newRPCMetadataRequest(request *requestRPCMetadata) (*RequestRpcMetadata, error) {
	metadata := NewRequestRpcMetadata()
	metadata.SetName(&request.Name)
	rpcProtocolID, err := protocolIDToRPCProtocolID(request.ProtoID)
	if err != nil {
		return nil, err
	}
	metadata.SetProtocol(&rpcProtocolID)
	kind, err := messageTypeToRPCKind(request.TypeID)
	if err != nil {
		return nil, err
	}
	metadata.SetKind(&kind)
	if request.Zstd {
		compression := CompressionAlgorithm_ZSTD
		metadata.SetCompression(&compression)
	}
	metadata.OtherMetadata = request.Other
	return metadata, nil
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
