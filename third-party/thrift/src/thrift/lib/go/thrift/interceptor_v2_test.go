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
	"slices"
	"sync"
	"testing"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
	"github.com/stretchr/testify/require"
)

// recordingInterceptor is a test ServiceInterceptor that records the order in
// which its OnRequest/OnResponse callbacks fire, optionally returns a request
// state from OnRequest, and optionally returns errors from either callback.
type recordingInterceptor struct {
	BaseServiceInterceptor
	name string

	// shared ordering log across all interceptors in a test
	log *[]string

	reqState     any
	onRequestErr error

	// userReqState observed by OnResponse, captured for assertions
	gotReqState   any
	onResponseErr error
}

func (i *recordingInterceptor) OnRequest(_ context.Context, _ types.ReadableStruct, _ any) (any, error) {
	*i.log = append(*i.log, "req:"+i.name)
	return i.reqState, i.onRequestErr
}

func (i *recordingInterceptor) OnResponse(_ context.Context, _ types.WritableStruct, userReqState any) error {
	*i.log = append(*i.log, "resp:"+i.name)
	i.gotReqState = userReqState
	return i.onResponseErr
}

func newTestServer(interceptors ...ServiceInterceptor) *server {
	return &server{interceptors: interceptors}
}

func TestRunOnRequestInterceptorsForwardOrder(t *testing.T) {
	var log []string
	a := &recordingInterceptor{name: "a", log: &log}
	b := &recordingInterceptor{name: "b", log: &log}
	c := &recordingInterceptor{name: "c", log: &log}
	s := newTestServer(a, b, c)

	ctx, err := s.runOnRequestInterceptors(context.Background(), nil)
	require.NoError(t, err)
	require.NotNil(t, ctx)
	require.Equal(t, []string{"req:a", "req:b", "req:c"}, log)
}

func TestRunOnResponseInterceptorsReverseOrder(t *testing.T) {
	var log []string
	a := &recordingInterceptor{name: "a", log: &log}
	b := &recordingInterceptor{name: "b", log: &log}
	c := &recordingInterceptor{name: "c", log: &log}
	s := newTestServer(a, b, c)

	err := s.runOnResponseInterceptors(context.Background(), nil)
	require.NoError(t, err)
	require.Equal(t, []string{"resp:c", "resp:b", "resp:a"}, log)
}

func TestRunInterceptorsThreadsRequestState(t *testing.T) {
	var log []string
	// a and c produce state; b produces none (nil), so no context value is set.
	a := &recordingInterceptor{name: "a", log: &log, reqState: "stateA"}
	b := &recordingInterceptor{name: "b", log: &log}
	c := &recordingInterceptor{name: "c", log: &log, reqState: 42}
	s := newTestServer(a, b, c)

	ctx, err := s.runOnRequestInterceptors(context.Background(), nil)
	require.NoError(t, err)

	err = s.runOnResponseInterceptors(ctx, nil)
	require.NoError(t, err)

	// Each interceptor's OnResponse receives exactly the state its OnRequest returned.
	require.Equal(t, "stateA", a.gotReqState)
	require.Nil(t, b.gotReqState)
	require.Equal(t, 42, c.gotReqState)
}

func TestRunOnRequestInterceptorsKeepsFirstError(t *testing.T) {
	var log []string
	errFirst := errors.New("first")
	errSecond := errors.New("second")
	// Both b and c error; the first error (b) must be returned, but all
	// interceptors must still run.
	a := &recordingInterceptor{name: "a", log: &log}
	b := &recordingInterceptor{name: "b", log: &log, onRequestErr: errFirst}
	c := &recordingInterceptor{name: "c", log: &log, onRequestErr: errSecond}
	s := newTestServer(a, b, c)

	_, err := s.runOnRequestInterceptors(context.Background(), nil)
	require.ErrorIs(t, err, errFirst)
	require.Equal(t, []string{"req:a", "req:b", "req:c"}, log)
}

func TestRunOnRequestInterceptorsThreadsStateDespiteError(t *testing.T) {
	var log []string
	// a errors but still returns state; that state must reach a's OnResponse.
	a := &recordingInterceptor{name: "a", log: &log, reqState: "stateA", onRequestErr: errors.New("boom")}
	b := &recordingInterceptor{name: "b", log: &log, reqState: "stateB"}
	s := newTestServer(a, b)

	ctx, err := s.runOnRequestInterceptors(context.Background(), nil)
	require.Error(t, err)

	err = s.runOnResponseInterceptors(ctx, nil)
	require.NoError(t, err)
	require.Equal(t, "stateA", a.gotReqState)
	require.Equal(t, "stateB", b.gotReqState)
}

func TestRunOnResponseInterceptorsKeepsLastError(t *testing.T) {
	var log []string
	errA := errors.New("errA")
	errC := errors.New("errC")
	// a and c error. Iteration is reverse (c, b, a), so the last error written is
	// a's, matching the C++ "OnResponse overwrites the active exception" rule.
	a := &recordingInterceptor{name: "a", log: &log, onResponseErr: errA}
	b := &recordingInterceptor{name: "b", log: &log}
	c := &recordingInterceptor{name: "c", log: &log, onResponseErr: errC}
	s := newTestServer(a, b, c)

	err := s.runOnResponseInterceptors(context.Background(), nil)
	require.ErrorIs(t, err, errA)
	require.Equal(t, []string{"resp:c", "resp:b", "resp:a"}, log)
}

func TestRunInterceptorsNoInterceptors(t *testing.T) {
	s := newTestServer()

	ctx, err := s.runOnRequestInterceptors(context.Background(), nil)
	require.NoError(t, err)
	require.Equal(t, context.Background(), ctx)

	require.NoError(t, s.runOnResponseInterceptors(context.Background(), nil))
}

// --- End-to-end server tests ---
//
// These stand up a single shared Rocket DummyService server with registered
// ServiceInterceptors and exercise the wired OnRequest behavior from the
// official ServiceInterceptor contract:
// https://www.internalfb.com/intern/staticdocs/thrift/docs/fb/server/service-interceptors/
//
// Interceptors are wired into the Rocket server path, so these tests use
// TransportIDRocket.

// serveDummyWithInterceptors stands up a Rocket DummyService server with the
// given interceptors and returns a connected client. Shutdown is registered via
// t.Cleanup so it runs after the test and all of its (possibly parallel)
// subtests complete.
func serveDummyWithInterceptors(t *testing.T, interceptors ...ServiceInterceptor) dummyif.DummyClient {
	t.Helper()

	listener, err := net.Listen("tcp", "[::]:0")
	require.NoError(t, err)
	addr := listener.Addr()

	opts := make([]ServerOption, 0, len(interceptors))
	for _, interceptor := range interceptors {
		opts = append(opts, WithServiceInterceptor(interceptor))
	}
	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDRocket, opts...)

	serverCtx, serverCancel := context.WithCancel(context.Background())
	var serverEG errgroup.Group
	serverEG.Go(func() error {
		return server.ServeContext(serverCtx)
	})

	channel, err := NewClient(
		getClientTransportOption(TransportIDRocket),
		WithIoTimeout(5*time.Second),
		WithDialer(func() (net.Conn, error) {
			return net.DialTimeout(addr.Network(), addr.String(), 5*time.Second)
		}),
	)
	require.NoError(t, err)
	client := dummyif.NewDummyChannelClient(channel)

	t.Cleanup(func() {
		require.NoError(t, client.Close())
		serverCancel()
		require.ErrorIs(t, serverEG.Wait(), context.Canceled)
	})
	return client
}

// interceptorRecorder records, per RPC method, the ordered names of the
// interceptors whose OnRequest fired. It is safe for concurrent use so subtests
// sharing one server can run in parallel.
type interceptorRecorder struct {
	mu  sync.Mutex
	log map[string][]string
}

func newInterceptorRecorder() *interceptorRecorder {
	return &interceptorRecorder{log: map[string][]string{}}
}

func (r *interceptorRecorder) record(method, name string) {
	r.mu.Lock()
	defer r.mu.Unlock()
	r.log[method] = append(r.log[method], name)
}

func (r *interceptorRecorder) get(method string) []string {
	r.mu.Lock()
	defer r.mu.Unlock()
	return slices.Clone(r.log[method])
}

// serverTestInterceptor is a concurrency-safe ServiceInterceptor for the shared
// server tests. It records every OnRequest call (keyed by method name) into a
// shared recorder and optionally fails OnRequest for specific methods.
type serverTestInterceptor struct {
	BaseServiceInterceptor
	name     string
	recorder *interceptorRecorder
	failOn   map[string]error // method name -> error returned from OnRequest
}

func (i *serverTestInterceptor) OnRequest(ctx context.Context, _ types.ReadableStruct, _ any) (any, error) {
	method := GetRequestContext(ctx).MethodName
	i.recorder.record(method, i.name)
	return nil, i.failOn[method]
}

// TestServiceInterceptorServer exercises the wired OnRequest behavior end-to-end
// against a single shared server, with the subtests running in parallel. Each
// subtest uses a distinct RPC method so their recorded calls don't overlap.
func TestServiceInterceptorServer(t *testing.T) {
	t.Parallel()

	recorder := newInterceptorRecorder()
	errDenied := errors.New("denied by interceptor")
	// Two interceptors share the recorder. "a" rejects OnRequest for the Ping
	// method (used by the error subtest); both record every OnRequest call.
	a := &serverTestInterceptor{name: "a", recorder: recorder, failOn: map[string]error{"Ping": errDenied}}
	b := &serverTestInterceptor{name: "b", recorder: recorder}
	client := serveDummyWithInterceptors(t, a, b)

	t.Run("OnRequest runs for all interceptors in forward order", func(t *testing.T) {
		t.Parallel()
		got, err := client.Echo(context.Background(), "hello")
		require.NoError(t, err)
		require.Equal(t, "hello", got)
		// OnRequest runs after deserialization and before the handler, for every
		// interceptor, in registration (forward) order.
		require.Equal(t, []string{"a", "b"}, recorder.get("Echo"))
	})

	t.Run("OnRequest error is returned to client and skips the handler", func(t *testing.T) {
		t.Parallel()
		// Ping normally returns nil; the interceptor error surfaces to the client
		// instead, which proves the handler was skipped.
		err := client.Ping(context.Background())
		require.ErrorContains(t, err, "denied by interceptor")
		// Every interceptor's OnRequest still ran, even though "a" errored.
		require.Equal(t, []string{"a", "b"}, recorder.get("Ping"))
	})
}
