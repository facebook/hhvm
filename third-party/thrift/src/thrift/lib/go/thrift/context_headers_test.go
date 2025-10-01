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

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestWithRequestHeadersDoNotOverride(t *testing.T) {
	ctx := context.Background()
	input1 := map[string]string{"key1": "value1"}
	input2 := map[string]string{"key2": "value2"}
	want := map[string]string{"key1": "value1", "key2": "value2"}
	var err error
	ctx, err = WithRequestHeader(ctx, "key1", "value1")
	assert.NoError(t, err)
	output1 := GetRequestHeadersFromContext(ctx)
	assert.Equal(t, input1, output1)
	ctx = WithRequestHeaders(ctx, input2)
	output2 := GetRequestHeadersFromContext(ctx)
	assert.Equal(t, want, output2)
}

func TestSetHeadersDoesOverride(t *testing.T) {
	ctx := context.Background()
	input1 := map[string]string{"key1": "value1"}
	input2 := map[string]string{"key2": "value2"}
	var err error
	ctx, err = WithRequestHeader(ctx, "key1", "value1")
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
	ctxWithHeaders := WithRequestHeaders(context.TODO(), headersMap)
	headers = GetRequestHeadersFromContext(ctxWithHeaders)
	require.Equal(t, headersMap, headers)

	// Case: context with invalid headers type
	ctxWithInvalidHeadersType := context.WithValue(context.TODO(), requestHeadersKey, 1234)
	headers = GetRequestHeadersFromContext(ctxWithInvalidHeadersType)
	require.Nil(t, headers)
}

func TestGetResponseHeadersFromContext(t *testing.T) {
	// Case: nil context
	headers := GetResponseHeadersFromContext(nil)
	require.Nil(t, headers)

	// Case: empty context
	headers = GetResponseHeadersFromContext(context.TODO())
	require.Nil(t, headers)

	// Case: context with headers
	headersMap := map[string]string{"foo": "bar"}
	ctxWithHeaders := NewResponseHeadersContext(context.TODO())
	setResponseHeaders(ctxWithHeaders, headersMap)
	headers = GetResponseHeadersFromContext(ctxWithHeaders)
	require.Equal(t, headersMap, headers)

	// Case: context with invalid headers type
	ctxWithInvalidHeadersType := context.WithValue(context.TODO(), responseHeadersKey, 1234)
	headers = GetResponseHeadersFromContext(ctxWithInvalidHeadersType)
	require.Nil(t, headers)
}
