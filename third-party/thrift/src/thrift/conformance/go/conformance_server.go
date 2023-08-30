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
	"time"

	"github.com/golang/glog"

	"libfb/go/thriftbase"

	thrift_any "thrift/conformance/any"
	"thrift/conformance/conformance"
	"thrift/conformance/patch_data"
	"thrift/conformance/protocol"
	"thrift/conformance/serialization"
	"thrift/lib/go/thrift"
	"thrift/test/testset"
	enum "thrift/test/testset/Enum"
)

// typeRegistry is a registry from thrift_uri or a hash of thrift_uri to thrift objects.
// typeRegistry is used to serialize and deserialize thrift.Any.
type typeRegistry struct {
	names  map[string]any
	hash32 map[[32]byte]any
	hash16 map[[16]byte]any
	hash8  map[[8]byte]any
}

func newTypeRegistry() *typeRegistry {
	return &typeRegistry{
		names:  make(map[string]any),
		hash32: make(map[[32]byte]any),
		hash16: make(map[[16]byte]any),
		hash8:  make(map[[8]byte]any),
	}
}

const thriftURIPrefix = "fbthrift://"

// RegisterType is called by the generated RegisterTypes function in thrift packages.
// Only types with a thrift_uri is registered.
func (r *typeRegistry) RegisterType(name string, obj any) {
	r.names[name] = obj
	h := sha256.New()
	h.Write([]byte(thriftURIPrefix + name))
	hash := h.Sum(nil)
	hash32 := *(*[32]byte)(hash[0:32])
	hash16 := *(*[16]byte)(hash[0:16])
	hash8 := *(*[8]byte)(hash[0:8])
	r.hash32[hash32] = obj
	r.hash16[hash16] = obj
	r.hash8[hash8] = obj
}

// LoadWithName loads objects from the type registry for deserialization given the thrift_uri name.
func (r *typeRegistry) LoadWithName(name string) (any, error) {
	obj, ok := r.names[name]
	if !ok {
		return nil, fmt.Errorf("load from registry error: %s is not registered", name)
	}
	return obj, nil
}

// LoadWithHash loads objects from the type registry for deserialization given the hashed thrift_uri name.
// The hashed thrift_uri can either be of length 8, 16 or 32.
func (r *typeRegistry) LoadWithHash(hash []byte) (any, error) {
	var obj any
	var ok bool
	if len(hash) == 8 {
		hash8 := *(*[8]byte)(hash[0:8])
		obj, ok = r.hash8[hash8]
		if !ok {
			return nil, fmt.Errorf("load from hash8 registry error: %s is not registered", string(hash))
		}
	}
	if len(hash) == 16 {
		hash16 := *(*[16]byte)(hash[0:16])
		obj, ok = r.hash16[hash16]
		if !ok {
			return nil, fmt.Errorf("load from hash16 registry error: %s is not registered", string(hash))
		}
	}
	if len(hash) == 32 {
		hash32 := *(*[32]byte)(hash[0:32])
		obj, ok = r.hash32[hash32]
		if !ok {
			return nil, fmt.Errorf("load from hash32 registry error: %s is not registered", string(hash))
		}
	}
	return obj, nil
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

	// Startup thrift server
	handler := &dataConformanceServiceHandler{registry}
	proc := conformance.NewConformanceServiceProcessor(handler)
	ts, err := thriftbase.ServerContext(
		proc,
		thriftbase.ServerSSLPolicy(thriftbase.SSLPolicyDisabled),
		// Ports must be dynamically allocated to prevent any conflicts.
		// Allocating a free port is usually done by setting the port number as zero.
		// Operating system should assign a free port to the application.
		thriftbase.BindAddr("[::]:0"),
	)
	if err != nil {
		glog.Fatalf("failed to start server: %v", err)
	}
	go func() {
		err := ts.Serve()
		if err != nil {
			glog.Fatalf("failed to start server")
		}
	}()

	// When server is started, it must print the listening port to the standard output console.
	for i := 1; i < 10; i++ {
		// Unfortunately there is currently no way to tell
		// if the server has started listening :(
		time.Sleep(1 * time.Second)
		addr := ts.ServerTransport().Addr()
		if addr != nil {
			fmt.Println(addr.(*net.TCPAddr).Port)
			break
		}
	}
	<-sigc
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
	return newRoundTripResponse(newResponse(requestValue, data)), nil
}

// newRoundTripResponse wraps the response thrift.Any inside a RoundTripResponse.
func newRoundTripResponse(response *thrift_any.Any) *serialization.RoundTripResponse {
	resp := serialization.NewRoundTripResponseBuilder()
	resp.Value(response)
	return resp.Emit()
}

// newResponse creates a new response Any from the request Any using new serialized data.
func newResponse(request *thrift_any.Any, data []byte) *thrift_any.Any {
	respAny := thrift_any.NewAnyBuilder()
	respAny.Data(data)
	respAny.CustomProtocol(request.CustomProtocol)
	respAny.Protocol(request.Protocol)
	respAny.Type(request.Type)
	return respAny.Emit()
}

// serialize serializes a thrift.Struct with a target protocol to be stored inside a thrift.Any.
func serialize(obj thrift.Struct, protoc *protocol.ProtocolStruct) ([]byte, error) {
	s, err := newSerializer(protoc)
	if err != nil {
		return nil, err
	}
	return s.Write(obj)
}

// loadStruct loads a thrift.Struct from the typeRegistry for a given thrift.Any.
// Any specifies the thrift.Struct to load either with a thrift_uri stored in the Type field
// Or with a hashed version of thrift_uri stored in TypeHashPrefixSha2_256.
func loadStruct(registry *typeRegistry, value *thrift_any.Any) (thrift.Struct, error) {
	var obj any
	if value.IsSetType() {
		typ := value.GetType()
		var err error
		obj, err = registry.LoadWithName(typ)
		if err != nil {
			return nil, err
		}
	} else if value.IsSetTypeHashPrefixSha2_256() {
		hash := value.GetTypeHashPrefixSha2_256()
		var err error
		obj, err = registry.LoadWithHash(hash)
		if err != nil {
			return nil, err
		}
	}
	structObj, ok := obj.(thrift.Struct)
	if !ok {
		return nil, fmt.Errorf("deserialize currently only supports thrift.Struct and not %T", obj)
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
	d, err := newDeserializer(protoc)
	if err != nil {
		return nil, err
	}
	if err := d.Read(obj, value.GetData()); err != nil {
		return nil, err
	}
	return obj, nil
}

// newSerializer initializes the appropriate serializer for the specific protocol.
func newSerializer(protoc *protocol.ProtocolStruct) (*thrift.Serializer, error) {
	factory, err := getProtocolFactory(protoc)
	if err != nil {
		return nil, err
	}
	s := thrift.NewSerializer()
	s.Protocol = factory.GetProtocol(s.Transport)
	return s, nil
}

// newDeserializer initializes the appropriate deserializer for the specific protocol.
func newDeserializer(protoc *protocol.ProtocolStruct) (*thrift.Deserializer, error) {
	factory, err := getProtocolFactory(protoc)
	if err != nil {
		return nil, err
	}
	d := thrift.NewDeserializer()
	d.Protocol = factory.GetProtocol(d.Transport)
	return d, nil
}

// getProtocolFactory is given a target protocol, which it uses to return a protocol factory that is used to initialise a Serializer or Deserializer.
func getProtocolFactory(protoc *protocol.ProtocolStruct) (thrift.ProtocolFactory, error) {
	switch protoc.GetStandard() {
	case protocol.StandardProtocol_Custom:
	case protocol.StandardProtocol_Binary:
		return thrift.NewBinaryProtocolFactoryDefault(), nil
	case protocol.StandardProtocol_Compact:
		return thrift.NewCompactProtocolFactory(), nil
	case protocol.StandardProtocol_Json:
		return thrift.NewSimpleJSONProtocolFactory(), nil
	case protocol.StandardProtocol_SimpleJson:
		return thrift.NewJSONProtocolFactory(), nil
	}
	// default value in case the protocol is unknown, as seen in the java implementation of conformance tests.
	return thrift.NewCompactProtocolFactory(), nil
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
