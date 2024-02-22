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
	"thrift/lib/thrift/rpcmetadata"

	"thrift/lib/go/thrift"
)

// RequestRPCMetadata is a thrift library version of rpcmetadata.RequestRpcMetadata
type RequestRPCMetadata struct {
	Name    string
	TypeID  thrift.MessageType
	ProtoID thrift.ProtocolID
	Zstd    bool
}

// SerializeRequestRPCMetadata sets the given arguments into a RequestRpcMetadata struct and serializes it into bytes
func SerializeRequestRPCMetadata(request *RequestRPCMetadata) ([]byte, error) {
	metadata, err := newRPCMetadataRequest(request)
	if err != nil {
		return nil, err
	}
	compactSerializer := thrift.NewCompactSerializer()
	return compactSerializer.Write(metadata)
}

// DeserializeRequestRPCMetadata deserializes the given bytes into a ResponseRpcMetadata struct
func DeserializeRequestRPCMetadata(metadataBytes []byte) (*RequestRPCMetadata, error) {
	metadata := &rpcmetadata.RequestRpcMetadata{}
	compactDeserializer := thrift.NewCompactDeserializer()
	err := compactDeserializer.Read(metadata, metadataBytes)
	if err != nil {
		return nil, err
	}
	return newRequestRPCMetadata(metadata)
}

func newRequestRPCMetadata(request *rpcmetadata.RequestRpcMetadata) (*RequestRPCMetadata, error) {
	name := request.GetName()
	typeID, err := rpcKindToMessageType(request.GetKind())
	if err != nil {
		return nil, err
	}
	protoID, err := rpcProtocolIDToProtocolID(request.GetProtocol())
	if err != nil {
		return nil, err
	}
	zstd := request.GetCompression() == rpcmetadata.CompressionAlgorithm_ZSTD
	return &RequestRPCMetadata{
		Name:    name,
		TypeID:  typeID,
		ProtoID: protoID,
		Zstd:    zstd,
	}, nil
}

func newRPCMetadataRequest(request *RequestRPCMetadata) (*rpcmetadata.RequestRpcMetadata, error) {
	metadata := rpcmetadata.NewRequestRpcMetadata()
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
		compression := rpcmetadata.CompressionAlgorithm_ZSTD
		metadata.SetCompression(&compression)
	}
	return metadata, nil
}

func protocolIDToRPCProtocolID(protocolID thrift.ProtocolID) (rpcmetadata.ProtocolId, error) {
	switch protocolID {
	case thrift.ProtocolIDBinary:
		return rpcmetadata.ProtocolId_BINARY, nil
	case thrift.ProtocolIDCompact:
		return rpcmetadata.ProtocolId_COMPACT, nil
	}
	return 0, fmt.Errorf("unsupported thrift.ProtocolID %v", protocolID)
}

func rpcProtocolIDToProtocolID(protocolID rpcmetadata.ProtocolId) (thrift.ProtocolID, error) {
	switch protocolID {
	case rpcmetadata.ProtocolId_BINARY:
		return thrift.ProtocolIDBinary, nil
	case rpcmetadata.ProtocolId_COMPACT:
		return thrift.ProtocolIDCompact, nil
	}
	return 0, fmt.Errorf("unsupported rpcmetadata.ProtocolId %v", protocolID)
}

func messageTypeToRPCKind(typeID thrift.MessageType) (rpcmetadata.RpcKind, error) {
	switch typeID {
	case thrift.CALL:
		return rpcmetadata.RpcKind_SINGLE_REQUEST_SINGLE_RESPONSE, nil
	case thrift.ONEWAY:
		return rpcmetadata.RpcKind_SINGLE_REQUEST_NO_RESPONSE, nil
	}
	return 0, fmt.Errorf("unsupported thrift.MessageType %v", typeID)
}

func rpcKindToMessageType(kind rpcmetadata.RpcKind) (thrift.MessageType, error) {
	switch kind {
	case rpcmetadata.RpcKind_SINGLE_REQUEST_SINGLE_RESPONSE:
		return thrift.CALL, nil
	case rpcmetadata.RpcKind_SINGLE_REQUEST_NO_RESPONSE:
		return thrift.ONEWAY, nil
	}
	return 0, fmt.Errorf("unsupported rpcmetadata.RpcKind %v", kind)
}
