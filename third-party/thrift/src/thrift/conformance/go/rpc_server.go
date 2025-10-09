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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	"thrift/conformance/rpc"

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
	ts, addr, err := newServer(
		proc,
		"[::]:0",
	)
	if err != nil {
		glog.Fatalf("failed to start server: %v", err)
	}

	fmt.Println(addr.(*net.TCPAddr).Port)
	ctx, cancel := context.WithCancel(context.Background())
	go func() {
		err := ts.ServeContext(ctx)
		if err != nil {
			glog.Fatalf("failed to start server")
		}
	}()

	<-sigc
	cancel()
	os.Exit(0)
}

func newServer(processor thrift.Processor, addr string) (thrift.Server, net.Addr, error) {
	listener, err := net.Listen("tcp", addr)
	if err != nil {
		return nil, nil, err
	}
	return thrift.NewServer(processor, listener, thrift.TransportIDRocket), listener.Addr(), nil
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
	return h.instruction.RequestResponseDeclaredException.UserException
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

func (h *rpcConformanceServiceHandler) StreamBasic(ctx context.Context, request *rpc.Request) (func(context.Context, chan<- *rpc.Response) error, error) {
	requestValue := rpc.NewStreamBasicServerTestResult().SetRequest(request)
	h.result = rpc.NewServerTestResult().
		SetStreamBasic(requestValue)

	elemProducerFunc := func(ctx context.Context, elemChan chan<- *rpc.Response) error {
        for _, resp := range h.instruction.StreamBasic.StreamPayloads {
            select {
                case <-ctx.Done():
                    return ctx.Err()
                case elemChan <- resp:
            }
        }
        return nil
	}
	return elemProducerFunc, nil
}

func (h *rpcConformanceServiceHandler) StreamInitialResponse(ctx context.Context, request *rpc.Request) (*rpc.Response, func(context.Context, chan<- *rpc.Response) error, error) {
	requestValue := rpc.NewStreamInitialResponseServerTestResult().SetRequest(request)
	h.result = rpc.NewServerTestResult().
		SetStreamInitialResponse(requestValue)

	elemProducerFunc := func(ctx context.Context, elemChan chan<- *rpc.Response) error {
        for _, resp := range h.instruction.StreamInitialResponse.StreamPayloads {
            select {
                case <-ctx.Done():
                    return ctx.Err()
                case elemChan <- resp:
            }
        }
        return nil
	}
	return h.instruction.StreamInitialResponse.InitialResponse, elemProducerFunc, nil
}

func (h *rpcConformanceServiceHandler) StreamDeclaredException(ctx context.Context, request *rpc.Request) (func(context.Context, chan<- *rpc.Response) error, error) {
	requestValue := rpc.NewStreamDeclaredExceptionServerTestResult().SetRequest(request)
	h.result = rpc.NewServerTestResult().
		SetStreamDeclaredException(requestValue)

	elemProducerFunc := func(ctx context.Context, elemChan chan<- *rpc.Response) error {
        return h.instruction.StreamDeclaredException.UserException
	}
	return elemProducerFunc, nil
}

func (h *rpcConformanceServiceHandler) StreamUndeclaredException(ctx context.Context, request *rpc.Request) (func(context.Context, chan<- *rpc.Response) error, error) {
	requestValue := rpc.NewStreamUndeclaredExceptionServerTestResult().SetRequest(request)
	h.result = rpc.NewServerTestResult().
		SetStreamUndeclaredException(requestValue)

	elemProducerFunc := func(ctx context.Context, elemChan chan<- *rpc.Response) error {
        return errors.New(h.instruction.StreamUndeclaredException.ExceptionMessage)
	}
	return elemProducerFunc, nil
}

func (h *rpcConformanceServiceHandler) StreamInitialDeclaredException(ctx context.Context, request *rpc.Request) (func(context.Context, chan<- *rpc.Response) error, error) {
	requestValue := rpc.NewStreamInitialDeclaredExceptionServerTestResult().SetRequest(request)
	h.result = rpc.NewServerTestResult().
		SetStreamInitialDeclaredException(requestValue)

	elemProducerFunc := func(ctx context.Context, elemChan chan<- *rpc.Response) error {
        return nil
	}
	return elemProducerFunc, h.instruction.StreamInitialDeclaredException.UserException
}

func (h *rpcConformanceServiceHandler) StreamInitialUndeclaredException(ctx context.Context, request *rpc.Request) (func(context.Context, chan<- *rpc.Response) error, error) {
	requestValue := rpc.NewStreamInitialUndeclaredExceptionServerTestResult().SetRequest(request)
	h.result = rpc.NewServerTestResult().
		SetStreamInitialUndeclaredException(requestValue)

	elemProducerFunc := func(ctx context.Context, elemChan chan<- *rpc.Response) error {
        return nil
	}
	return elemProducerFunc, errors.New(h.instruction.StreamInitialUndeclaredException.ExceptionMessage)
}

func (h *rpcConformanceServiceHandler) StreamChunkTimeout(ctx context.Context, request *rpc.Request) (func(context.Context, chan<- *rpc.Response) error, error) {
	return nil, errors.New("not supported")
}

func (h *rpcConformanceServiceHandler) StreamCreditTimeout(ctx context.Context, request *rpc.Request) (func(context.Context, chan<- *rpc.Response) error, error) {
	return nil, errors.New("not supported")
}

func (h *rpcConformanceServiceHandler) StreamInitialTimeout(ctx context.Context, request *rpc.Request) (func(context.Context, chan<- *rpc.Response) error, error) {
	return nil, errors.New("not supported")
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
