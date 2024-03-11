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
	"time"

	"thrift/conformance/rpc"
	"thrift/lib/go/thrift"

	"github.com/golang/glog"
)

func main() {
	// catch SIGTERM/SIGKILL
	sigc := make(chan os.Signal, 1)
	signal.Notify(sigc,
		syscall.SIGTERM,
		syscall.SIGINT,
	)

	handler := &rpcConformanceServiceHandler{}
	proc := rpc.NewRPCConformanceServiceProcessor(handler)
	ts, err := newServer(
		proc,
		"[::]:0",
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

	for i := 1; i < 10; i++ {
		// Unfortunately there is currently no way to tell
		// if the server has started listening :(
		time.Sleep(1 * time.Second)
		addr := ts.ServerTransport().Addr()
		if addr != nil {
			// Print port for the test runner
			fmt.Println(addr.(*net.TCPAddr).Port)
			break
		}
	}

	<-sigc
	os.Exit(0)
}

func newServer(processor thrift.ProcessorContext, addr string) (thrift.Server, error) {
	socket, err := thrift.NewServerSocket(addr)
	if err != nil {
		return nil, err
	}
	protocol := thrift.NewHeaderProtocolFactory()
	transport := thrift.NewHeaderTransportFactory(thrift.NewTransportFactory())
	return thrift.NewSimpleServerContext(processor, socket, transport, protocol), nil
}

type rpcConformanceServiceHandler struct {
	result      *rpc.ServerTestResult
	instruction *rpc.ServerInstruction
}

func (h *rpcConformanceServiceHandler) RequestResponseBasic(ctx context.Context, request *rpc.Request) (*rpc.Response, error) {
	requestValue := rpc.NewRequestResponseBasicServerTestResult().
		SetRequest(request)
	h.result = rpc.NewServerTestResult().
		SetRequestResponseBasic(requestValue)
	return h.instruction.RequestResponseBasic.Response, nil
}

func (h *rpcConformanceServiceHandler) RequestResponseNoArgVoidResponse(ctx context.Context) error {
	requestValue := rpc.NewRequestResponseNoArgVoidResponseServerTestResult()
	h.result = rpc.NewServerTestResult().
		SetRequestResponseNoArgVoidResponse(requestValue)
	return nil
}

func (h *rpcConformanceServiceHandler) RequestResponseTimeout(ctx context.Context, request *rpc.Request) (*rpc.Response, error) {
	return nil, errors.New("not supported")
}

func (h *rpcConformanceServiceHandler) RequestResponseDeclaredException(ctx context.Context, request *rpc.Request) error {
	requestValue := rpc.NewRequestResponseDeclaredExceptionServerTestResult().
		SetRequest(request)
	h.result = rpc.NewServerTestResult().
		SetRequestResponseDeclaredException(requestValue)
	return rpc.NewUserException().SetMsg(h.instruction.RequestResponseDeclaredException.UserException.Msg)
}

func (h *rpcConformanceServiceHandler) RequestResponseUndeclaredException(ctx context.Context, request *rpc.Request) error {
	requestValue := rpc.NewRequestResponseUndeclaredExceptionServerTestResult().
		SetRequest(request)
	h.result = rpc.NewServerTestResult().
		SetRequestResponseUndeclaredException(requestValue)
	return errors.New(
		h.instruction.RequestResponseUndeclaredException.ExceptionMessage,
	)
}

func (h *rpcConformanceServiceHandler) SendTestCase(ctx context.Context, testCase *rpc.RpcTestCase) error {
	h.instruction = testCase.ServerInstruction
	return nil
}

func (h *rpcConformanceServiceHandler) GetTestResult(ctx context.Context) (*rpc.ServerTestResult, error) {
	return h.result, nil
}

func (h *rpcConformanceServiceHandler) GetTestCase(ctx context.Context) (*rpc.RpcTestCase, error) {
	return nil, errors.New("not implemented")
}

func (h *rpcConformanceServiceHandler) SendTestResult(ctx context.Context, result *rpc.ClientTestResult) error {
	return errors.New("not implemented")
}
