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
	"crypto/sha256"
	"errors"
	"fmt"
	"net"
	"os"
	"os/signal"
	"syscall"

	"github.com/golang/glog"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	thrift_any "thrift/conformance/any"
	"thrift/conformance/conformance"
	"thrift/conformance/patch_data"
	"thrift/conformance/protocol"
	"thrift/conformance/serialization"
	"thrift/lib/thrift/protocol_detail"
	"thrift/test/testset"
	enum "thrift/test/testset/Enum"
)

// Registry initializer func type
type registryInitializerFuncType = func() any

// typeRegistry is a registry from thrift_uri or its hash Thrift initializer function.
// typeRegistry is used to serialize and deserialize thrift.Any.
type typeRegistry struct {
	names  map[string]registryInitializerFuncType
	hash32 map[[32]byte]registryInitializerFuncType
	hash16 map[[16]byte]registryInitializerFuncType
	hash8  map[[8]byte]registryInitializerFuncType
}

func newTypeRegistry() *typeRegistry {
	return &typeRegistry{
		names:  make(map[string]registryInitializerFuncType),
		hash32: make(map[[32]byte]registryInitializerFuncType),
		hash16: make(map[[16]byte]registryInitializerFuncType),
		hash8:  make(map[[8]byte]registryInitializerFuncType),
	}
}

const thriftURIPrefix = "fbthrift://"

// RegisterType is called by the generated RegisterTypes function in thrift packages.
// Only types with a thrift_uri is registered.
func (r *typeRegistry) RegisterType(name string, initializer registryInitializerFuncType) {
	r.names[name] = initializer
	h := sha256.New()
	h.Write([]byte(thriftURIPrefix + name))
	hash := h.Sum(nil)
	hash32 := *(*[32]byte)(hash[0:32])
	hash16 := *(*[16]byte)(hash[0:16])
	hash8 := *(*[8]byte)(hash[0:8])
	r.hash32[hash32] = initializer
	r.hash16[hash16] = initializer
	r.hash8[hash8] = initializer
}

// LoadInitializerWithName loads initializer from the type registry for deserialization given the thrift_uri name.
func (r *typeRegistry) LoadInitializerWithName(name string) (registryInitializerFuncType, error) {
	initializer, ok := r.names[name]
	if !ok {
		return nil, fmt.Errorf("load from registry error: %s is not registered", name)
	}
	return initializer, nil
}

// LoadInitializerWithHash loads initializer from the type registry for deserialization given the hashed thrift_uri name.
// The hashed thrift_uri can either be of length 8, 16 or 32.
func (r *typeRegistry) LoadInitializerWithHash(hash []byte) (registryInitializerFuncType, error) {
	var initializer registryInitializerFuncType
	var ok bool
	if len(hash) == 8 {
		hash8 := *(*[8]byte)(hash[0:8])
		initializer, ok = r.hash8[hash8]
		if !ok {
			return nil, fmt.Errorf("load from hash8 registry error: %s is not registered", string(hash))
		}
	}
	if len(hash) == 16 {
		hash16 := *(*[16]byte)(hash[0:16])
		initializer, ok = r.hash16[hash16]
		if !ok {
			return nil, fmt.Errorf("load from hash16 registry error: %s is not registered", string(hash))
		}
	}
	if len(hash) == 32 {
		hash32 := *(*[32]byte)(hash[0:32])
		initializer, ok = r.hash32[hash32]
		if !ok {
			return nil, fmt.Errorf("load from hash32 registry error: %s is not registered", string(hash))
		}
	}
	return initializer, nil
}

func main() {
	// Catch SIGTERM/SIGKILL
	sigc := make(chan os.Signal, 1)
	signal.Notify(sigc,
		syscall.SIGTERM,
		syscall.SIGINT,
	)

	// Register all types from the testset
	registry := newTypeRegistry()
	testset.RegisterTypes(registry)
	enum.RegisterTypes(registry)
	protocol_detail.RegisterTypes(registry)

	// Startup thrift server
	handler := &dataConformanceServiceHandler{registry}
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

type dataConformanceServiceHandler struct {
	registry *typeRegistry
}

func (h *dataConformanceServiceHandler) RoundTrip(ctx context.Context, roundTripRequest *serialization.RoundTripRequest) (*serialization.RoundTripResponse, error) {
	requestValue := roundTripRequest.GetValue()
	if requestValue == nil {
		return nil, errors.New("unsupported RoundTrip roundTripRequest.Value = nil")
	}
	obj, err := deserialize(h.registry, requestValue)
	if err != nil {
		return nil, err
	}
	target := getTargetProtocol(roundTripRequest)
	data, err := serialize(obj, target)
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
		// default value in case the protocol is unknown, as seen in the java implementation of conformance tests.
		return thrift.EncodeCompact(obj)
	}
}

// loadStruct loads a thrift.Struct from the typeRegistry for a given thrift.Any.
// Any specifies the thrift.Struct to load either with a thrift_uri stored in the Type field
// Or with a hashed version of thrift_uri stored in TypeHashPrefixSha2_256.
func loadStruct(registry *typeRegistry, value *thrift_any.Any) (thrift.Struct, error) {
	var initializer registryInitializerFuncType
	var err error
	if value.IsSetType() {
		typ := value.GetType()
		initializer, err = registry.LoadInitializerWithName(typ)
		if err != nil {
			return nil, err
		}
	} else if value.IsSetTypeHashPrefixSha2_256() {
		hash := value.GetTypeHashPrefixSha2_256()
		initializer, err = registry.LoadInitializerWithHash(hash)
		if err != nil {
			return nil, err
		}
	}
	anyObj := initializer()
	structObj, ok := anyObj.(thrift.Struct)
	if !ok {
		return nil, fmt.Errorf("deserialize currently only supports thrift.Struct and not %T", anyObj)
	}
	return structObj, nil
}

// deserialize deserializes the data stored inside a thrift.Any value.
func deserialize(registry *typeRegistry, value *thrift_any.Any) (thrift.Struct, error) {
	obj, err := loadStruct(registry, value)
	if err != nil {
		return nil, err
	}
	protoc := getProtocol(value)
	switch protoc.GetStandard() {
	case protocol.StandardProtocol_Custom:
		err = thrift.DecodeCompact(value.GetData(), obj)
	case protocol.StandardProtocol_Binary:
		err = thrift.DecodeBinary(value.GetData(), obj)
	case protocol.StandardProtocol_Compact:
		err = thrift.DecodeCompact(value.GetData(), obj)
	case protocol.StandardProtocol_Json:
		err = thrift.DecodeCompactJSON(value.GetData(), obj)
	case protocol.StandardProtocol_SimpleJson:
		err = thrift.DecodeSimpleJSON(value.GetData(), obj)
	default:
		// default value in case the protocol is unknown, as seen in the java implementation of conformance tests.
		err = thrift.DecodeCompact(value.GetData(), obj)
	}
	if err != nil {
		return nil, err
	}
	return obj, nil
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
	return getProtocol(value)
}

// getProtocol creates a ProtocolStruct from the values stored inside thrift.Any.
func getProtocol(value *thrift_any.Any) *protocol.ProtocolStruct {
	return &protocol.ProtocolStruct{
		Standard: value.GetProtocol(),
		Custom:   value.CustomProtocol,
	}
}

func (h *dataConformanceServiceHandler) Patch(ctx context.Context, request *patch_data.PatchOpRequest) (_r *patch_data.PatchOpResponse, err error) {
	return nil, errors.New("patch is not supported")
}
