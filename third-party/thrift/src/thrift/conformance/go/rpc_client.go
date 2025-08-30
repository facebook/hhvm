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
	"flag"
	"fmt"
	"net"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	"thrift/conformance/rpc"

	"github.com/golang/glog"
)

func main() {
	var port int
	flag.IntVar(&port, "port", 7777, "RPC conformance test server port")
	flag.Parse()

	tester := newRPCClientConformanceTester(port)
	tester.execute()
}

type rpcClientConformanceTester struct {
	port        int
	client      rpc.RPCConformanceServiceClientInterface
	instruction *rpc.ClientInstruction
}

func newRPCClientConformanceTester(port int) *rpcClientConformanceTester {
	return &rpcClientConformanceTester{
		port: port,
	}
}

func (t *rpcClientConformanceTester) getClient() (rpc.RPCConformanceServiceClientInterface, error) {
	addr := fmt.Sprintf("localhost:%d", t.port)
	channel, err := thrift.NewClient(
		thrift.WithRocket(),
		thrift.WithDialer(func() (net.Conn, error) {
			return net.Dial("tcp", addr)
		}),
	)
	if err != nil {
		return nil, fmt.Errorf("unable to upgrade to rocket protocol: %w", err)
	}
	return rpc.NewRPCConformanceServiceChannelClient(channel), nil
}

func (t *rpcClientConformanceTester) execute() {
	var err error

	t.client, err = t.getClient()
	if err != nil {
		glog.Fatalf("failed to get client: %v", err)
	}

	// 60 second timeout per test-case should be plenty
	testCaseCtx, testCaseCancel := context.WithTimeout(context.Background(), 60*time.Second)
	defer testCaseCancel()

	testCase, err := t.client.GetTestCase(testCaseCtx)
	if err != nil {
		glog.Fatalf("failed to get test case: %v", err)
	}
	t.instruction = testCase.ClientInstruction

	switch {
	case t.instruction.RequestResponseBasic != nil:
		err = t.RequestResponseBasic(testCaseCtx)
	case t.instruction.RequestResponseNoArgVoidResponse != nil:
		err = t.RequestResponseNoArgVoidResponse(testCaseCtx)
	case t.instruction.RequestResponseDeclaredException != nil:
		err = t.RequestResponseDeclaredException(testCaseCtx)
	case t.instruction.RequestResponseUndeclaredException != nil:
		err = t.RequestResponseUndeclaredException(testCaseCtx)
	case t.instruction.RequestResponseTimeout != nil:
		err = t.RequestResponseTimeout(testCaseCtx)
	case t.instruction.StreamBasic != nil:
		err = t.StreamBasic(testCaseCtx)
	case t.instruction.StreamInitialResponse != nil:
		err = t.StreamInitialResponse(testCaseCtx)
	case t.instruction.StreamDeclaredException != nil:
		err = t.StreamDeclaredException(testCaseCtx)
	case t.instruction.StreamUndeclaredException != nil:
		err = t.StreamUndeclaredException(testCaseCtx)
	case t.instruction.StreamInitialDeclaredException != nil:
		err = t.StreamInitialDeclaredException(testCaseCtx)
	case t.instruction.StreamInitialUndeclaredException != nil:
		err = t.StreamInitialUndeclaredException(testCaseCtx)
	case t.instruction.StreamChunkTimeout != nil:
		err = t.StreamChunkTimeout(testCaseCtx)
	case t.instruction.StreamCreditTimeout != nil:
		err = t.StreamCreditTimeout(testCaseCtx)
	case t.instruction.StreamInitialTimeout != nil:
		err = t.StreamInitialTimeout(testCaseCtx)
	case t.instruction.SinkBasic != nil:
		err = t.SinkBasic(testCaseCtx)
	case t.instruction.SinkChunkTimeout != nil:
		err = t.SinkChunkTimeout(testCaseCtx)
	case t.instruction.SinkInitialResponse != nil:
		err = t.SinkInitialResponse(testCaseCtx)
	case t.instruction.SinkDeclaredException != nil:
		err = t.SinkDeclaredException(testCaseCtx)
	case t.instruction.SinkUndeclaredException != nil:
		err = t.SinkUndeclaredException(testCaseCtx)
	case t.instruction.InteractionConstructor != nil:
		err = t.InteractionConstructor(testCaseCtx)
	case t.instruction.InteractionFactoryFunction != nil:
		err = t.InteractionFactoryFunction(testCaseCtx)
	case t.instruction.InteractionPersistsState != nil:
		err = t.InteractionPersistsState(testCaseCtx)
	case t.instruction.InteractionTermination != nil:
		err = t.InteractionTermination(testCaseCtx)
	default:
		glog.Fatal("unsupported test case")
	}

	if err != nil {
		glog.Fatalf("test failed: %v", err)
	}
}

func (t *rpcClientConformanceTester) RequestResponseBasic(ctx context.Context) error {
	response, err := t.client.RequestResponseBasic(
		ctx, t.instruction.RequestResponseBasic.Request,
	)
	if err != nil {
		return err
	}

	responseValue := rpc.NewRequestResponseBasicClientTestResult().
		SetResponse(response)
	clientTestResult := rpc.NewClientTestResult().
		SetRequestResponseBasic(responseValue)
	return t.client.SendTestResult(ctx, clientTestResult)
}

func (t *rpcClientConformanceTester) RequestResponseNoArgVoidResponse(ctx context.Context) error {
	err := t.client.RequestResponseNoArgVoidResponse(ctx)
	if err != nil {
		return err
	}

	responseValue := rpc.NewRequestResponseNoArgVoidResponseClientTestResult()
	clientTestResult := rpc.NewClientTestResult().
		SetRequestResponseNoArgVoidResponse(responseValue)
	return t.client.SendTestResult(ctx, clientTestResult)
}

func (t *rpcClientConformanceTester) RequestResponseTimeout(ctx context.Context) error {
	timeoutMs := t.instruction.RequestResponseTimeout.GetTimeoutMs()
	timeoutCtx, timeoutCancel := context.WithTimeout(
		ctx, time.Duration(timeoutMs)*time.Millisecond,
	)
	defer timeoutCancel()

	_, err := t.client.RequestResponseTimeout(
		timeoutCtx, t.instruction.RequestResponseTimeout.Request,
	)

	// We expect to hit a timeout for this test case.
	isTimeout := (err != nil)

	responseValue := rpc.NewRequestResponseTimeoutClientTestResult().
		SetTimeoutException(isTimeout)
	clientTestResult := rpc.NewClientTestResult().
		SetRequestResponseTimeout(responseValue)
	return t.client.SendTestResult(ctx, clientTestResult)
}

func (t *rpcClientConformanceTester) RequestResponseDeclaredException(ctx context.Context) error {
	err := t.client.RequestResponseDeclaredException(
		ctx, t.instruction.RequestResponseDeclaredException.Request,
	)

	responseValue := rpc.NewRequestResponseDeclaredExceptionClientTestResult().
		SetUserException(err.(*rpc.UserException))
	clientTestResult := rpc.NewClientTestResult().
		SetRequestResponseDeclaredException(responseValue)
	return t.client.SendTestResult(ctx, clientTestResult)
}

func (t *rpcClientConformanceTester) RequestResponseUndeclaredException(ctx context.Context) error {
	err := t.client.RequestResponseUndeclaredException(
		ctx, t.instruction.RequestResponseUndeclaredException.Request,
	)

	responseValue := rpc.NewRequestResponseUndeclaredExceptionClientTestResult().
		SetExceptionMessage(err.Error())
	clientTestResult := rpc.NewClientTestResult().
		SetRequestResponseUndeclaredException(responseValue)
	return t.client.SendTestResult(ctx, clientTestResult)
}

func (t *rpcClientConformanceTester) StreamBasic(ctx context.Context) error {
	streamCtx, streamCancel := context.WithCancel(ctx)
	defer streamCancel()

	elemChan, elemErrChan, err := t.client.StreamBasic(streamCtx, t.instruction.StreamBasic.Request)
	if err != nil {
		return err
	}

	responses := make([]*rpc.Response, 0)
	for elem := range elemChan {
		responses = append(responses, elem)
	}

	// Check if streaming encountered and error
	streamErr := <-elemErrChan
	if streamErr != nil {
		return err
	}

	responseValue := rpc.NewStreamBasicClientTestResult().
		SetStreamPayloads(responses)
	clientTestResult := rpc.NewClientTestResult().
		SetStreamBasic(responseValue)
	return t.client.SendTestResult(ctx, clientTestResult)
}

func (t *rpcClientConformanceTester) StreamInitialResponse(ctx context.Context) error {
	streamCtx, streamCancel := context.WithCancel(ctx)
	defer streamCancel()

	initElem, elemChan, elemErrChan, err := t.client.StreamInitialResponse(streamCtx, t.instruction.StreamInitialResponse.Request)
	if err != nil {
		return err
	}

	responses := make([]*rpc.Response, 0)
	for elem := range elemChan {
		responses = append(responses, elem)
	}

	// Check if streaming encountered and error
	streamErr := <-elemErrChan
	if streamErr != nil {
		return err
	}

	responseValue := rpc.NewStreamInitialResponseClientTestResult().
		SetInitialResponse(initElem).
		SetStreamPayloads(responses)
	clientTestResult := rpc.NewClientTestResult().
		SetStreamInitialResponse(responseValue)
	return t.client.SendTestResult(ctx, clientTestResult)
}

func (t *rpcClientConformanceTester) StreamDeclaredException(ctx context.Context) error {
	streamCtx, streamCancel := context.WithCancel(ctx)
	defer streamCancel()

	elemChan, elemErrChan, err := t.client.StreamDeclaredException(streamCtx, t.instruction.StreamDeclaredException.Request)
	if err != nil {
		return err
	}

	for range elemChan {
		// Do nothing.
	}

	// Check if streaming encountered and error
	streamErr := <-elemErrChan

	responseValue := rpc.NewStreamDeclaredExceptionClientTestResult().
		SetUserException(streamErr.(*rpc.UserException))
	clientTestResult := rpc.NewClientTestResult().
		SetStreamDeclaredException(responseValue)
	return t.client.SendTestResult(ctx, clientTestResult)
}

func (t *rpcClientConformanceTester) StreamUndeclaredException(ctx context.Context) error {
	streamCtx, streamCancel := context.WithCancel(ctx)
	defer streamCancel()

	elemChan, elemErrChan, err := t.client.StreamUndeclaredException(streamCtx, t.instruction.StreamUndeclaredException.Request)
	if err != nil {
		return err
	}

	for range elemChan {
		// Do nothing.
	}

	// Check if streaming encountered and error
	streamErr := <-elemErrChan

	responseValue := rpc.NewStreamUndeclaredExceptionClientTestResult().
		SetExceptionMessage(streamErr.Error())
	clientTestResult := rpc.NewClientTestResult().
		SetStreamUndeclaredException(responseValue)
	return t.client.SendTestResult(ctx, clientTestResult)
}

func (t *rpcClientConformanceTester) StreamInitialDeclaredException(ctx context.Context) error {
	streamCtx, streamCancel := context.WithCancel(ctx)
	defer streamCancel()

	_, _, err := t.client.StreamInitialDeclaredException(streamCtx, t.instruction.StreamInitialDeclaredException.Request)

	responseValue := rpc.NewStreamInitialDeclaredExceptionClientTestResult().
		SetUserException(err.(*rpc.UserException))
	clientTestResult := rpc.NewClientTestResult().
		SetStreamInitialDeclaredException(responseValue)
	return t.client.SendTestResult(ctx, clientTestResult)
}

func (t *rpcClientConformanceTester) StreamInitialUndeclaredException(ctx context.Context) error {
	streamCtx, streamCancel := context.WithCancel(ctx)
	defer streamCancel()

	_, _, err := t.client.StreamInitialUndeclaredException(streamCtx, t.instruction.StreamInitialUndeclaredException.Request)

	responseValue := rpc.NewStreamInitialUndeclaredExceptionClientTestResult().
		SetExceptionMessage(err.Error())
	clientTestResult := rpc.NewClientTestResult().
		SetStreamInitialUndeclaredException(responseValue)
	return t.client.SendTestResult(ctx, clientTestResult)
}

func (t *rpcClientConformanceTester) StreamChunkTimeout(ctx context.Context) error {
	return errors.New("not supported")
}

func (t *rpcClientConformanceTester) StreamCreditTimeout(ctx context.Context) error {
	return errors.New("not supported")
}

func (t *rpcClientConformanceTester) StreamInitialTimeout(ctx context.Context) error {
	timeoutMs := t.instruction.StreamInitialTimeout.GetTimeoutMs()
	timeoutCtx, timeoutCancel := context.WithTimeout(
		ctx, time.Duration(timeoutMs)*time.Millisecond,
	)
	defer timeoutCancel()

	_, _, err := t.client.StreamInitialTimeout(timeoutCtx, t.instruction.StreamInitialTimeout.Request)

	// We expect to hit a timeout for this test case.
	isTimeout := (err != nil)

	responseValue := rpc.NewStreamInitialTimeoutClientTestResult().
		SetTimeoutException(isTimeout)
	clientTestResult := rpc.NewClientTestResult().
		SetStreamInitialTimeout(responseValue)
	return t.client.SendTestResult(ctx, clientTestResult)
}

func (t *rpcClientConformanceTester) SinkBasic(ctx context.Context) error {
	return errors.New("not supported")
}

func (t *rpcClientConformanceTester) SinkChunkTimeout(ctx context.Context) error {
	return errors.New("not supported")
}

func (t *rpcClientConformanceTester) SinkInitialResponse(ctx context.Context) error {
	return errors.New("not supported")
}

func (t *rpcClientConformanceTester) SinkDeclaredException(ctx context.Context) error {
	return errors.New("not supported")
}

func (t *rpcClientConformanceTester) SinkUndeclaredException(ctx context.Context) error {
	return errors.New("not supported")
}

func (t *rpcClientConformanceTester) InteractionConstructor(ctx context.Context) error {
	return errors.New("not supported")
}

func (t *rpcClientConformanceTester) InteractionFactoryFunction(ctx context.Context) error {
	return errors.New("not supported")
}

func (t *rpcClientConformanceTester) InteractionPersistsState(ctx context.Context) error {
	return errors.New("not supported")
}

func (t *rpcClientConformanceTester) InteractionTermination(ctx context.Context) error {
	return errors.New("not supported")
}
