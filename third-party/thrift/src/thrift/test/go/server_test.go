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
	"errors"
	"fmt"
	"net"
	"reflect"
	"testing"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	"github.com/stretchr/testify/require"
	"thrift/test/go/if/thrifttest"
)

const localConnTimeout = time.Second * 1
const testCallString = "this is a fairly lengthy test string \\ that ' has \x20 some 东西奇怪的"

// createTestHeaderServer Create and bind a test server to localhost
func createTestHeaderServer(handler thrifttest.ThriftTest) (context.CancelFunc, net.Addr, error) {
	ctx, cancel := context.WithCancel(context.Background())
	processor := thrifttest.NewThriftTestProcessor(handler)

	listener, err := net.Listen("tcp", "[::]:0")
	if err != nil {
		return cancel, nil, fmt.Errorf("failed to open test socket: %w", err)
	}
	taddr := listener.Addr()

	server := thrift.NewServer(processor, listener, thrift.TransportIDHeader)
	go func(server thrift.Server) {
		err = server.ServeContext(ctx)
		if err != nil && !errors.Is(err, context.Canceled) {
			panic(fmt.Errorf("failed to begin serving test socket: %s", err))
		}
	}(server)

	conn, err := net.DialTimeout(taddr.Network(), taddr.String(), localConnTimeout)
	if err != nil {
		return cancel, nil, fmt.Errorf(
			"failed to connect to test socket: %s:%s", taddr.Network(), taddr.String(),
		)
	}
	conn.Close()

	return cancel, taddr, nil
}

// connectTestHeaderServer Create a client and connect to a test server
func connectTestHeaderServer(
	addr net.Addr,
) (thrifttest.ThriftTestClient, error) {
	channel, err := thrift.NewClient(
		thrift.WithUpgradeToRocket(),
		thrift.WithDialer(func() (net.Conn, error) {
			return net.Dial("tcp", addr.String())
		}),
		thrift.WithIoTimeout(localConnTimeout),
	)
	if err != nil {
		return nil, err
	}
	return thrifttest.NewThriftTestChannelClient(channel), nil
}

func doClientTest(ctx context.Context, t *testing.T) {
	handler := &testHandler{}
	cancel, addr, err := createTestHeaderServer(handler)
	require.NoError(t, err)
	defer cancel()

	client, err := connectTestHeaderServer(addr)
	require.NoError(t, err)
	defer client.Close()

	res, err := client.DoTestString(ctx, testCallString)
	require.NoError(t, err)
	require.Equal(t, testCallString, res)

	// Try sending a lot of requests
	for i := 0; i < 1000; i++ {
		res, err = client.DoTestString(ctx, testCallString)
		require.NoError(t, err)
		require.Equal(t, testCallString, res)
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
		require.NoError(t, terr)

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
	doClientTest(ctx, t)
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
