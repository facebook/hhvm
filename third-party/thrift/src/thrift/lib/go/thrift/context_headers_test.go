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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestHeaderProtocolSomeHeaders(t *testing.T) {
	ctx := context.Background()
	want := map[string]string{"key1": "value1", "key2": "value2"}
	var err error
	for key, value := range want {
		ctx, err = AddHeader(ctx, key, value)
		require.NoError(t, err)
	}
	protocol, err := newHeaderProtocol(newMockSocket(), types.ProtocolIDCompact, 0, nil)
	require.NoError(t, err)
	err = SetRequestHeaders(ctx, protocol)
	require.NoError(t, err)
	got := protocol.(*headerProtocol).trans.writeInfoHeaders
	assert.Equal(t, want, got)
}

// somewhere we are still passing context as nil, so we need to support this for now
func TestHeaderProtocolSetNilHeaders(t *testing.T) {
	protocol, err := newHeaderProtocol(newMockSocket(), types.ProtocolIDCompact, 0, nil)
	require.NoError(t, err)
	err = SetRequestHeaders(nil, protocol)
	require.NoError(t, err)
}

func TestRocketProtocolSomeHeaders(t *testing.T) {
	ctx := context.Background()
	want := map[string]string{"key1": "value1", "key2": "value2"}
	var err error
	for key, value := range want {
		ctx, err = AddHeader(ctx, key, value)
		require.NoError(t, err)
	}
	protocol, err := newRocketClient(newMockSocket(), types.ProtocolIDCompact, 0, nil)
	require.NoError(t, err)
	err = SetRequestHeaders(ctx, protocol)
	require.NoError(t, err)
	got := protocol.(*rocketClient).reqHeaders
	assert.Equal(t, want, got)
}

// somewhere we are still passing context as nil, so we need to support this for now
func TestRocketProtocolSetNilHeaders(t *testing.T) {
	protocol, err := newRocketClient(newMockSocket(), types.ProtocolIDCompact, 0, nil)
	require.NoError(t, err)
	err = SetRequestHeaders(nil, protocol)
	require.NoError(t, err)
}

// somewhere we are still passing context as nil, so we need to support this for now
func TestUpgradeToRocketProtocolSetNilHeaders(t *testing.T) {
	protocol, err := newUpgradeToRocketClient(newMockSocket(), types.ProtocolIDCompact, 0, nil)
	require.NoError(t, err)
	err = SetRequestHeaders(nil, protocol)
	require.NoError(t, err)
}

func TestWithHeadersDoNotOverride(t *testing.T) {
	ctx := context.Background()
	input1 := map[string]string{"key1": "value1"}
	input2 := map[string]string{"key2": "value2"}
	want := map[string]string{"key1": "value1", "key2": "value2"}
	var err error
	ctx, err = AddHeader(ctx, "key1", "value1")
	assert.NoError(t, err)
	output1 := GetRequestHeadersFromContext(ctx)
	assert.Equal(t, input1, output1)
	ctx = WithHeaders(ctx, input2)
	output2 := GetRequestHeadersFromContext(ctx)
	assert.Equal(t, want, output2)
}

func TestSetHeadersDoesOverride(t *testing.T) {
	ctx := context.Background()
	input1 := map[string]string{"key1": "value1"}
	input2 := map[string]string{"key2": "value2"}
	var err error
	ctx, err = AddHeader(ctx, "key1", "value1")
	assert.NoError(t, err)
	output1 := GetRequestHeadersFromContext(ctx)
	assert.Equal(t, input1, output1)
	ctx = SetHeaders(ctx, input2)
	output2 := GetRequestHeadersFromContext(ctx)
	assert.Equal(t, input2, output2)
}

func TestGetRequestHeadersFromContext(t *testing.T) {
	// Case: nil context
	headers := GetRequestHeadersFromContext(nil)
	require.Nil(t, headers)

	// Case: empty context
	headers = GetRequestHeadersFromContext(context.TODO())
	require.Nil(t, headers)

	// Case: context with headers
	headersMap := map[string]string{"foo": "bar"}
	ctxWithHeaders := WithHeaders(context.TODO(), headersMap)
	headers = GetRequestHeadersFromContext(ctxWithHeaders)
	require.Equal(t, headersMap, headers)

	// Case: context with invalid headers type
	ctxWithInvalidHeadersType := context.WithValue(context.TODO(), requestHeadersKey, 1234)
	headers = GetRequestHeadersFromContext(ctxWithInvalidHeadersType)
	require.Nil(t, headers)
}
