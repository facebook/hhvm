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
	"errors"
	"flag"
	"fmt"
	"net"

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
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return nil, err
	}
	transport, err := thrift.NewSocket(thrift.SocketConn(conn))
	if err != nil {
		return nil, fmt.Errorf("unable to connect to %s", addr)
	}
	proto := thrift.NewHeaderProtocol(thrift.NewHeaderTransport(transport))
	return rpc.NewRPCConformanceServiceClient(proto), nil
}

func (t *rpcClientConformanceTester) execute() {
	var err error

	t.client, err = t.getClient()
	if err != nil {
		glog.Fatalf("failed to get client: %v", err)
	}

	testCase, err := t.client.GetTestCase()
	if err != nil {
		glog.Fatalf("failed to get test case: %v", err)
	}
	t.instruction = testCase.ClientInstruction

	switch {
	case t.instruction.RequestResponseBasic != nil:
		err = t.RequestResponseBasic()
	case t.instruction.RequestResponseNoArgVoidResponse != nil:
		err = t.RequestResponseNoArgVoidResponse()
	case t.instruction.RequestResponseDeclaredException != nil:
		err = t.RequestResponseDeclaredException()
	case t.instruction.RequestResponseUndeclaredException != nil:
		err = t.RequestResponseUndeclaredException()
	case t.instruction.RequestResponseTimeout != nil:
		err = t.RequestResponseTimeout()
	default:
		glog.Fatal("unsupported test case")
	}

	if err != nil {
		glog.Fatalf("test failed: %v", err)
	}
}

func (t *rpcClientConformanceTester) RequestResponseBasic() error {
	response, err := t.client.RequestResponseBasic(
		t.instruction.RequestResponseBasic.Request,
	)
	if err != nil {
		return err
	}

	responseValue := rpc.NewRequestResponseBasicClientTestResult().
		SetResponse(response)
	clientTestResult := rpc.NewClientTestResult().
		SetRequestResponseBasic(responseValue)
	return t.client.SendTestResult(clientTestResult)
}

func (t *rpcClientConformanceTester) RequestResponseNoArgVoidResponse() error {
	err := t.client.RequestResponseNoArgVoidResponse()
	if err != nil {
		return err
	}

	responseValue := rpc.NewRequestResponseNoArgVoidResponseClientTestResult()
	clientTestResult := rpc.NewClientTestResult().
		SetRequestResponseNoArgVoidResponse(responseValue)
	return t.client.SendTestResult(clientTestResult)
}

func (t *rpcClientConformanceTester) RequestResponseTimeout() error {
	// TODO: per call timeouts are not yet supported in Go.
	// This test case is currently marked as nonconforming.
	return errors.New("not supported")
}

func (t *rpcClientConformanceTester) RequestResponseDeclaredException() error {
	err := t.client.RequestResponseDeclaredException(
		t.instruction.RequestResponseDeclaredException.Request,
	)

	responseValue := rpc.NewRequestResponseDeclaredExceptionClientTestResult().
		SetUserException(err.(*rpc.UserException))
	clientTestResult := rpc.NewClientTestResult().
		SetRequestResponseDeclaredException(responseValue)
	return t.client.SendTestResult(clientTestResult)
}

func (t *rpcClientConformanceTester) RequestResponseUndeclaredException() error {
	err := t.client.RequestResponseUndeclaredException(
		t.instruction.RequestResponseUndeclaredException.Request,
	)

	responseValue := rpc.NewRequestResponseUndeclaredExceptionClientTestResult().
		SetExceptionMessage(err.Error())
	clientTestResult := rpc.NewClientTestResult().
		SetRequestResponseUndeclaredException(responseValue)
	return t.client.SendTestResult(clientTestResult)
}
