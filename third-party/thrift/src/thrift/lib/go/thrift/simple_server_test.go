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
	"context"
	"errors"
	"net"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/metadata"
)

// TestSimpleServer is a simple tests that simple sends an empty message to a server and receives an empty result.
func TestSimpleServer(t *testing.T) {
	listener, err := net.Listen("tcp", "[::]:0")
	if err != nil {
		t.Fatalf("could not create listener: %s", err)
	}
	addr := listener.Addr()
	handler := &testProcessor{}
	server := NewSimpleServer(handler, listener, TransportIDHeader)
	errChan := make(chan error)
	ctx, cancel := context.WithCancel(context.Background())
	go func() {
		err := server.ServeContext(ctx)
		errChan <- err
		close(errChan)
	}()
	conn, err := DialHostPort(addr.String())
	if err != nil {
		t.Fatalf("could not create client socket: %s", err)
	}
	proto, err := newHeaderProtocol(conn, types.ProtocolIDCompact, 0, nil)
	if err != nil {
		t.Fatalf("could not create client protocol: %s", err)
	}
	client := NewSerialChannel(proto)
	if err := client.Call(context.Background(), "test", &MyTestStruct{}, &MyTestStruct{}); err != nil {
		t.Fatalf("could not send message: %s", err)
	}
	cancel()
	err = <-errChan
	if !errors.Is(err, context.Canceled) {
		t.Fatalf("expected %v, got %v", context.Canceled, err)
	}
}

type testProcessor struct {
}

func (t *testProcessor) ProcessorFunctionMap() map[string]types.ProcessorFunction {
	return map[string]types.ProcessorFunction{"test": &testProcessorFunction{}}
}

func (t *testProcessor) GetThriftMetadata() *metadata.ThriftMetadata {
	return nil
}

type testProcessorFunction struct{}

func (p *testProcessorFunction) Read(prot types.Decoder) (types.Struct, types.Exception) {
	args := NewMyTestStruct()
	if err := args.Read(prot); err != nil {
		return nil, err
	}
	prot.ReadMessageEnd()
	return args, nil
}

func (p *testProcessorFunction) Write(seqID int32, result types.WritableStruct, oprot types.Encoder) (err types.Exception) {
	var err2 error
	messageType := types.REPLY
	switch result.(type) {
	case types.ApplicationException:
		messageType = types.EXCEPTION
	}

	if err2 = oprot.WriteMessageBegin("test", messageType, seqID); err2 != nil {
		err = err2
	}
	if err2 = result.Write(oprot); err == nil && err2 != nil {
		err = err2
	}
	if err2 = oprot.WriteMessageEnd(); err == nil && err2 != nil {
		err = err2
	}
	if err2 = oprot.Flush(); err == nil && err2 != nil {
		err = err2
	}
	return err
}

func (p *testProcessorFunction) RunContext(ctx context.Context, reqStruct types.Struct) (types.WritableStruct, types.ApplicationException) {
	return reqStruct, nil
}

// This tests that S425600 does not happen again.
// The client is allowed to set a serializaton format to the non default and the server should adjust accordingly.
func TestSimpleServerClientSetsDifferentProtocol(t *testing.T) {
	listener, err := net.Listen("tcp", "[::]:0")
	if err != nil {
		t.Fatalf("could not create listener: %s", err)
	}
	addr := listener.Addr()
	handler := &testProcessor{}
	server := NewSimpleServer(handler, listener, TransportIDHeader)
	errChan := make(chan error)
	ctx, cancel := context.WithCancel(context.Background())
	go func() {
		err := server.ServeContext(ctx)
		errChan <- err
		close(errChan)
	}()
	conn, err := DialHostPort(addr.String())
	if err != nil {
		t.Fatalf("could not create client socket: %s", err)
	}
	// Sets the client serialization format to a non default.
	proto, err := NewClient(WithHeader(), WithConn(conn), WithProtocolID(types.ProtocolIDBinary))
	if err != nil {
		t.Fatalf("could not create client protocol: %s", err)
	}
	client := NewSerialChannel(proto)
	if err := client.Call(context.Background(), "test", &MyTestStruct{}, &MyTestStruct{}); err != nil {
		t.Fatalf("could not send message: %s", err)
	}
	cancel()
	<-errChan
}
