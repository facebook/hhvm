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
	"bytes"
	"context"
	"net"
	"testing"
)

// Make sure that ConnInfo is added to the context of a rocket server.
func TestRocketServerConnInfo(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	errChan := make(chan error)
	defer close(errChan)
	listener, err := net.Listen("tcp", ":0")
	if err != nil {
		t.Fatalf("failed to listen: %v", err)
	}
	server := NewSimpleServer(&rocketServerTestProcessor{}, listener, TransportIDRocket)
	go func() {
		errChan <- server.ServeContext(ctx)
	}()
	addr := listener.Addr()
	conn, err := net.Dial(addr.Network(), addr.String())
	if err != nil {
		t.Fatalf("failed to dial: %v", err)
	}
	proto, err := newRocketClient(conn, ProtocolIDCompact, 0, nil)
	if err != nil {
		t.Fatalf("could not create client protocol: %s", err)
	}
	client := NewSerialChannel(proto)
	req := &MyTestStruct{
		St: "hello",
	}
	resp := &MyTestStruct{}
	if err := client.Call(context.Background(), "test", req, resp); err != nil {
		t.Fatalf("could not complete call: %v", err)
	}
	if resp.St != "hello" {
		t.Fatalf("expected response to be a hello, got %s", resp.St)
	}
	if !bytes.Equal(resp.GetBin(), []byte(conn.LocalAddr().String())) {
		t.Fatalf("expected response to be an address %s, got %s", conn.LocalAddr().String(), resp.GetBin())
	}
	cancel()
	<-errChan
}

type rocketServerTestProcessor struct{}

func (t *rocketServerTestProcessor) GetProcessorFunctionContext(name string) ProcessorFunctionContext {
	if name == "test" {
		return &rocketServerTestProcessorFunction{&testProcessorFunction{}}
	}
	return nil
}

type rocketServerTestProcessorFunction struct {
	ProcessorFunctionContext
}

func (p *rocketServerTestProcessorFunction) RunContext(ctx context.Context, reqStruct Struct) (WritableStruct, ApplicationException) {
	v, ok := ConnInfoFromContext(ctx)
	if ok {
		reqStruct.(*MyTestStruct).Bin = []byte(v.RemoteAddr.String())
	}
	return reqStruct, nil
}
