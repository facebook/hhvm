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
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
	"github.com/stretchr/testify/require"
)

func dummyInterceptor(
	ctx context.Context,
	methodName string,
	pfunc types.ProcessorFunction,
	args types.ReadableStruct,
) (types.WritableStruct, types.ApplicationExceptionIf) {
	if methodName == "Echo" {
		if echoArg, ok := args.(*dummyif.DummyEchoArgsDeprecated); ok {
			echoArg.Value = echoArg.Value + "-intercepted"
		}
	}
	return pfunc.RunContext(ctx, args)
}

func TestInterceptorCreation(t *testing.T) {
	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})

	// Nil case
	derivedProc := WrapInterceptor(nil, processor)
	require.NotNil(t, derivedProc)
	require.Equal(t, processor, derivedProc)

	// Regular case
	derivedProc = WrapInterceptor(dummyInterceptor, processor)
	require.NotNil(t, derivedProc)
	require.NotEqual(t, processor, derivedProc)
}

func TestInterceptorProcessorFunctionMap(t *testing.T) {
	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	derivedProc := WrapInterceptor(dummyInterceptor, processor)
	pFunc := derivedProc.ProcessorFunctionMap()["blah"]
	// Assert that all original functions are present
	require.Equal(t, len(processor.ProcessorFunctionMap()), len(derivedProc.ProcessorFunctionMap()))
	for funcName := range processor.ProcessorFunctionMap() {
		require.Contains(t, derivedProc.ProcessorFunctionMap(), funcName)
	}
	require.Nil(t, pFunc)
}

func TestInterceptorRunContext(t *testing.T) {
	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	derivedProc := WrapInterceptor(dummyInterceptor, processor)
	pFunc := derivedProc.ProcessorFunctionMap()["Echo"]
	arg := dummyif.DummyEchoArgsDeprecated{
		Value: "hello",
	}
	resp, err := pFunc.RunContext(context.Background(), &arg)
	echoResp := resp.(*dummyif.DummyEchoResultDeprecated)
	require.NoError(t, err)
	require.Equal(t, "hello-intercepted", *echoResp.Success)
}

func TestChainInterceptors(t *testing.T) {
	dummyInterceptor1 := func(
		ctx context.Context,
		methodName string,
		pfunc types.ProcessorFunction,
		args types.ReadableStruct,
	) (types.WritableStruct, types.ApplicationExceptionIf) {
		if methodName == "Echo" {
			ws, ae := pfunc.RunContext(ctx, args)
			echoResp := ws.(*dummyif.DummyEchoResultDeprecated)
			echoResp.Success = Pointerize(*echoResp.Success + "-intercepted1")
			return ws, ae
		}
		return pfunc.RunContext(ctx, args)
	}
	dummyInterceptor2 := func(
		ctx context.Context,
		methodName string,
		pfunc types.ProcessorFunction,
		args types.ReadableStruct,
	) (types.WritableStruct, types.ApplicationExceptionIf) {
		if methodName == "Echo" {
			ws, ae := pfunc.RunContext(ctx, args)
			echoResp := ws.(*dummyif.DummyEchoResultDeprecated)
			echoResp.Success = Pointerize(*echoResp.Success + "-intercepted2")
			return ws, ae
		}
		return pfunc.RunContext(ctx, args)
	}
	dummyInterceptor3 := func(
		ctx context.Context,
		methodName string,
		pfunc types.ProcessorFunction,
		args types.ReadableStruct,
	) (types.WritableStruct, types.ApplicationExceptionIf) {
		if methodName == "Echo" {
			ws, ae := pfunc.RunContext(ctx, args)
			echoResp := ws.(*dummyif.DummyEchoResultDeprecated)
			echoResp.Success = Pointerize(*echoResp.Success + "-intercepted3")
			return ws, ae
		}
		return pfunc.RunContext(ctx, args)
	}

	chainedInterceptor := ChainInterceptors(dummyInterceptor1, dummyInterceptor2, dummyInterceptor3)
	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	derivedProc := WrapInterceptor(chainedInterceptor, processor)
	pFunc := derivedProc.ProcessorFunctionMap()["Echo"]
	arg := dummyif.DummyEchoArgsDeprecated{
		Value: "hello",
	}
	resp, err := pFunc.RunContext(context.Background(), &arg)
	echoResp := resp.(*dummyif.DummyEchoResultDeprecated)
	require.NoError(t, err)
	require.Equal(t, "hello-intercepted3-intercepted2-intercepted1", *echoResp.Success)
}
