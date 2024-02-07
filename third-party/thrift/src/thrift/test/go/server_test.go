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

package gotest

import (
	"context"
	"fmt"
	"net"
	"reflect"
	"testing"
	"thrift/lib/go/thrift"
	"thrift/test/go/if/thrifttest"
	"time"
)

const localConnTimeout = time.Second * 1
const testCallString = "this is a fairly lengthy test string \\ that ' has \x20 some 东西奇怪的"

// createTestHeaderServer Create and bind a test server to localhost
func createTestHeaderServer(handler thrifttest.ThriftTest) (*thrift.SimpleServer, net.Addr, error) {
	processor := thrifttest.NewThriftTestProcessor(handler)
	transportFactory := thrift.NewHeaderTransportFactory(thrift.NewTransportFactory())
	protocolFactory := thrift.NewHeaderProtocolFactory()

	transport, err := thrift.NewServerSocket("[::]:0")
	if err != nil {
		return nil, nil, fmt.Errorf("failed to open test socket: %s", err)
	}

	err = transport.Listen()
	if err != nil {
		return nil, nil, fmt.Errorf("failed to listen on socket: %s", err)
	}
	taddr := transport.Addr()

	server := thrift.NewSimpleServerContext(processor, transport,
		thrift.TransportFactories(transportFactory),
		thrift.ProtocolFactories(protocolFactory))
	go func(server *thrift.SimpleServer) {
		err = server.Serve()
		if err != nil && err != thrift.ErrServerClosed {
			panic(fmt.Errorf("failed to begin serving test socket: %s", err))
		}
	}(server)

	conn, err := net.DialTimeout(taddr.Network(), taddr.String(), localConnTimeout)
	if err != nil {
		return nil, nil, fmt.Errorf(
			"failed to connect to test socket: %s:%s", taddr.Network(), taddr.String(),
		)
	}
	conn.Close()

	return server, taddr, nil
}

type transportFactory = func(socket *thrift.Socket) thrift.Transport

// connectTestHeaderServer Create a client and connect to a test server
func connectTestHeaderServer(
	addr net.Addr,
	transportFactory func(socket *thrift.Socket) thrift.Transport,
	protocolFactory thrift.ProtocolFactory,
) (*thrifttest.ThriftTestChannelClient, error) {
	socket, err := thrift.NewSocket(thrift.SocketAddr(addr.String()), thrift.SocketTimeout(localConnTimeout))
	if err != nil {
		return nil, err
	}

	err = socket.Open()
	if err != nil {
		return nil, err
	}

	trans := transportFactory(socket)
	prot := protocolFactory.GetProtocol(trans)
	return thrifttest.NewThriftTestChannelClient(thrift.NewSerialChannel(prot)), nil
}

func doClientTest(ctx context.Context, t *testing.T, transportFactory transportFactory, protocolFactory thrift.ProtocolFactory) {
	handler := &testHandler{}
	serv, addr, err := createTestHeaderServer(handler)
	if err != nil {
		t.Fatalf("failed to create test server: %s", err.Error())
	}
	defer serv.Stop()

	client, err := connectTestHeaderServer(addr, transportFactory, protocolFactory)
	if err != nil {
		t.Fatalf("failed to connect to test server: %s", err.Error())
	}
	defer client.Close()

	res, err := client.DoTestString(ctx, testCallString)
	if err != nil {
		t.Fatalf("failed to query test server: %s", err.Error())
	}

	if res != testCallString {
		t.Fatalf("server query compare failed")
	}

	// Try sending a lot of requests
	for i := 0; i < 1000; i++ {
		res, err = client.DoTestString(ctx, testCallString)
		if err != nil {
			t.Fatalf("failed to query test server: %s", err.Error())
		}
		if res != testCallString {
			t.Fatalf("server query compare failed")
		}
	}

	// Try getting an application Exception
	exp1 := thrifttest.NewXception()
	exp1.ErrorCode = 5
	exp1.Message = testCallString
	handler.ReturnError = exp1

	err = client.DoTestException(ctx, testCallString)
	if texp, ok := err.(*thrifttest.Xception); ok && texp != nil {
		if texp.ErrorCode != 5 || texp.Message != testCallString {
			t.Fatalf("application exception values incorrect: got=%s", texp.String())
		}
	} else {
		t.Fatalf("application exception type incorrect: got=%v", err)
	}
	handler.ReturnError = nil

	// Make a large-ish struct
	insanity := thrifttest.NewInsanity()
	insanity.UserMap = map[thrifttest.Numberz]thrifttest.UserId{}
	insanity.Str2str = map[string]string{}
	for i := 0; i < 50000; i++ {
		insanity.UserMap[thrifttest.Numberz_SIX] = thrifttest.UserId(i)
		insanity.Xtructs = append(insanity.Xtructs, &thrifttest.Xtruct{
			StringThing: testCallString, ByteThing: 5, I32Thing: 50, I64Thing: 100,
		})
		insanity.Str2str[fmt.Sprintf("%d", i)] = testCallString
	}

	// Try sending a lot of large things
	for i := 0; i < 10; i++ {
		resp, terr := client.DoTestInsanity(ctx, insanity)
		if terr != nil {
			t.Fatalf("failed to query test server: %s", err.Error())
		}

		num, ok := resp[thrifttest.UserId(3)]
		if !ok {
			t.Fatalf("incorrect response from server on insanity")
		}

		data, ok := num[thrifttest.Numberz_EIGHT]
		if !ok {
			t.Fatalf("incorrect response from server on insanity")
		}

		if !reflect.DeepEqual(data, insanity) {
			t.Fatalf("incorrect response from server on insanity")
		}
	}

	// Ensure poorly named method exists
	_ = client.XDoTestPoorName(ctx)
}

func TestHeaderHeader(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	doClientTest(
		ctx,
		t,
		func(socket *thrift.Socket) thrift.Transport {
			return thrift.NewHeaderTransport(socket)
		},
		thrift.NewHeaderProtocolFactory(),
	)
}

func TestHeaderFramedBinary(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	doClientTest(
		ctx,
		t,
		func(socket *thrift.Socket) thrift.Transport {
			return thrift.NewFramedTransport(socket)
		},
		thrift.NewBinaryProtocolFactory(false, true),
	)
}

func TestHeaderFramedCompact(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	doClientTest(
		ctx,
		t,
		func(socket *thrift.Socket) thrift.Transport {
			return thrift.NewFramedTransport(socket)
		},
		thrift.NewCompactProtocolFactory(),
	)
}

func TestFunctionServiceMap(t *testing.T) {
	handler := &testHandler{}
	proc := thrifttest.NewThriftTestProcessor(handler)
	mapping := proc.FunctionServiceMap()

	srv, ok := mapping["doTestVoid"]
	if !ok {
		t.Errorf("expected key 'doTestVoid' in FunctionServiceMap")
	}
	if srv != "ThriftTest" {
		t.Errorf("expected key 'doTestVoid' with value 'ThriftTest'")
	}
}

// unframed not supported?
// func TestHeaderUnframedBinary(t *testing.T) {
// 	doClientTest(
// 		t,
// 		thrift.NewBufferedTransportFactory(8192),
// 		thrift.NewBinaryProtocolFactory(false, true),
// 	)
// }
//
// func TestHeaderUnframedCompact(t *testing.T) {
// 	doClientTest(
// 		t,
// 		thrift.NewBufferedTransportFactory(8192),
// 		thrift.NewCompactProtocolFactory(),
// 	)
// }
