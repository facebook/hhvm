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

package main

import (
	"context"
	"errors"
	"fmt"
	"net"
	"os"
	"os/signal"
	"syscall"

	thrift_any "thrift/conformance/any"
	"thrift/conformance/conformance"
	"thrift/conformance/patch_data"
	"thrift/conformance/protocol"
	"thrift/conformance/serialization"
	thriftany "thrift/lib/thrift/any"
	thriftanyrep "thrift/lib/thrift/any_rep"
	thriftstandard "thrift/lib/thrift/standard"
	thrifttyperep "thrift/lib/thrift/type_rep"

	"thrift/lib/thrift/protocol_detail"
	"thrift/test/testset"
	enum "thrift/test/testset/Enum"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/format"
	"github.com/golang/glog"
)

// Ensure that the types below are registered (so that Any serdes works!)
var _ = protocol_detail.GoUnusedProtection__
var _ = testset.GoUnusedProtection__
var _ = enum.GoUnusedProtection__

func main() {
	// Catch SIGTERM/SIGKILL
	sigc := make(chan os.Signal, 1)
	signal.Notify(sigc,
		syscall.SIGTERM,
		syscall.SIGINT,
	)

	// Startup thrift server
	handler := &dataConformanceServiceHandler{}
	proc := conformance.NewConformanceServiceProcessor(handler)

	listener, err := net.Listen("tcp", "[::]:0")
	if err != nil {
		glog.Fatalf("failed to listen: %v", err)
	}
	addr := listener.Addr()
	server := thrift.NewServer(proc, listener, thrift.TransportIDUpgradeToRocket)
	ctx, cancel := context.WithCancel(context.Background())
	go func() {
		err := server.ServeContext(ctx)
		if err != nil {
			glog.Fatalf("failed to start server")
		}
	}()
	fmt.Println(addr.(*net.TCPAddr).Port)

	<-sigc
	cancel()
	os.Exit(0)
}

type dataConformanceServiceHandler struct{}

func (h *dataConformanceServiceHandler) RoundTrip(ctx context.Context, roundTripRequest *serialization.RoundTripRequest) (*serialization.RoundTripResponse, error) {
	requestValue := roundTripRequest.GetValue()
	if requestValue == nil {
		return nil, errors.New("unsupported RoundTrip roundTripRequest.Value = nil")
	}

	libAny, err := conformanceAnyToLibAny(requestValue)
	if err != nil {
		return nil, err
	}

	obj, err := format.DecodeAny(libAny)
	if err != nil {
		return nil, err
	}
	objAsStruct, ok := obj.(thrift.Struct)
	if !ok {
		return nil, errors.New("unsupported RoundTrip request value type")
	}

	target := getTargetProtocol(roundTripRequest)
	data, err := serialize(objAsStruct, target)
	if err != nil {
		return nil, err
	}

	respAny := thrift_any.NewAny().
		SetData(data).
		SetCustomProtocol(requestValue.CustomProtocol).
		SetProtocol(requestValue.Protocol).
		SetType(requestValue.Type).
		SetTypeHashPrefixSha2_256(requestValue.TypeHashPrefixSha2_256)
	return serialization.NewRoundTripResponse().
		SetValue(respAny), nil
}

func (h *dataConformanceServiceHandler) Patch(ctx context.Context, request *patch_data.PatchOpRequest) (_r *patch_data.PatchOpResponse, err error) {
	return nil, errors.New("patch is not supported")
}

// serialize serializes a thrift.Struct with a target protocol to be stored inside a thrift.Any.
func serialize(obj thrift.Struct, protoc *protocol.ProtocolStruct) ([]byte, error) {
	switch protoc.GetStandard() {
	case protocol.StandardProtocol_Custom:
		return thrift.EncodeCompact(obj)
	case protocol.StandardProtocol_Binary:
		return thrift.EncodeBinary(obj)
	case protocol.StandardProtocol_Compact:
		return thrift.EncodeCompact(obj)
	case protocol.StandardProtocol_Json:
		return thrift.EncodeCompactJSON(obj)
	case protocol.StandardProtocol_SimpleJson:
		return thrift.EncodeSimpleJSON(obj)
	default:
		// Default to Compact as per thrift spec
		return thrift.EncodeCompact(obj)
	}
}

// conformanceAnyToLibAny converts from thrift/conformance/any.Any to thrift/lib/thrift/any.Any.
// The conformance Any has a simpler structure with type (string URI) or typeHashPrefixSha2_256,
// while the lib Any uses TypeStruct and ProtocolUnion.
func conformanceAnyToLibAny(conformanceAny *thrift_any.Any) (*thriftany.Any, error) {
	typeURI := thriftstandard.NewTypeUri()
	typeName := thriftstandard.NewTypeName().
		SetStructType(typeURI)
	typeStruct := thrifttyperep.NewTypeStruct().
		SetName(typeName)
	protocolUnion := thrifttyperep.NewProtocolUnion()
	anyStruct := thriftanyrep.NewAnyStruct().
		SetType(typeStruct).
		SetProtocol(protocolUnion).
		SetData(conformanceAny.GetData())

	if conformanceAny.IsSetType() && conformanceAny.GetType() != "" {
		typeURI.SetUri(thrift.Pointerize(conformanceAny.GetType()))
	} else if conformanceAny.TypeHashPrefixSha2_256 != nil {
		typeURI.SetTypeHashPrefixSha2_256(conformanceAny.TypeHashPrefixSha2_256)
	} else {
		return nil, errors.New("conformanceAnyToLibAny: type or typeHashPrefixSha2_256 must be set")
	}

	// Default to Compact as per thrift spec
	protocolUnion.SetStandard(thrift.Pointerize(thriftstandard.StandardProtocol_Compact))
	if conformanceAny.Protocol != nil {
		// Map conformance StandardProtocol to lib StandardProtocol
		switch *conformanceAny.Protocol {
		case protocol.StandardProtocol_Binary:
			protocolUnion.SetStandard(thrift.Pointerize(thriftstandard.StandardProtocol_Binary))
		case protocol.StandardProtocol_Compact:
			protocolUnion.SetStandard(thrift.Pointerize(thriftstandard.StandardProtocol_Compact))
		case protocol.StandardProtocol_Json:
			protocolUnion.SetStandard(thrift.Pointerize(thriftstandard.StandardProtocol_Json))
		case protocol.StandardProtocol_SimpleJson:
			protocolUnion.SetStandard(thrift.Pointerize(thriftstandard.StandardProtocol_SimpleJson))
		case protocol.StandardProtocol_Custom:
			if conformanceAny.IsSetCustomProtocol() {
				protocolUnion.SetCustom(thrift.Pointerize(conformanceAny.GetCustomProtocol()))
			}
		}
	}

	return anyStruct, nil
}

// getTargetProtocol returns a consistent target protocol in the ProtocolStruct, whether the target protocol was set or not.
// In the case that the target protocol is taken from the value set inside the Any value.
// Worst case the Compact protocol is returned as a default.
// "Any request and encodes it back to the RoundTripResponse using target protocol - if not empty - or using the protocol in Any."
func getTargetProtocol(request *serialization.RoundTripRequest) *protocol.ProtocolStruct {
	if request.IsSetTargetProtocol() {
		return request.GetTargetProtocol()
	}
	// default value in case the protocol is not specified, as seen in the java implementation of conformance tests.
	if !request.IsSetValue() {
		return &protocol.ProtocolStruct{
			Standard: protocol.StandardProtocol_Compact,
			Custom:   nil,
		}
	}
	value := request.GetValue()
	return &protocol.ProtocolStruct{
		Standard: value.GetProtocol(),
		Custom:   value.CustomProtocol,
	}
}
