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
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
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
