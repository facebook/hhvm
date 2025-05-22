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
	"flag"
	"fmt"
	"net"
	"time"

	"thrift/conformance/rpc"
	"thrift/lib/go/thrift"

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
	client      *rpc.RPCConformanceServiceClient
	instruction *rpc.ClientInstruction
}

func newRPCClientConformanceTester(port int) *rpcClientConformanceTester {
	return &rpcClientConformanceTester{
		port: port,
	}
}

func (t *rpcClientConformanceTester) getClient() (*rpc.RPCConformanceServiceClient, error) {
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
