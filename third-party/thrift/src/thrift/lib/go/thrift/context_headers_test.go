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

func TestWithRequestHeaders(t *testing.T) {
	ctx := context.Background()

	input1 := map[string]string{"key1": "value1"}
	ctx = WithRequestHeaders(ctx, input1)
	output1 := GetRequestHeadersFromContext(ctx)
	assert.Equal(t, input1, output1)

	input2 := map[string]string{"key2": "value2"}
	ctx = WithRequestHeaders(ctx, input2)
	output2 := GetRequestHeadersFromContext(ctx)
	expected := map[string]string{"key1": "value1", "key2": "value2"}
	assert.Equal(t, expected, output2)
}

func TestSetHeadersDoesOverride(t *testing.T) {
	ctx := context.Background()
	input1 := map[string]string{"key1": "value1"}
	input2 := map[string]string{"key2": "value2"}
	ctx = WithRequestHeader(ctx, "key1", "value1")
	output1 := GetRequestHeadersFromContext(ctx)
	assert.Equal(t, input1, output1)
	ctx = SetHeaders(ctx, input2)
	output2 := GetRequestHeadersFromContext(ctx)
	assert.Equal(t, input2, output2)
}

func TestGetRequestHeadersFromContext(t *testing.T) {
	t.Run("nil context", func(t *testing.T) {
		headers := GetRequestHeadersFromContext(nil)
		require.Nil(t, headers)
	})
	t.Run("empty context", func(t *testing.T) {
		headers := GetRequestHeadersFromContext(context.TODO())
		require.Nil(t, headers)
	})
	t.Run("context with existing headers", func(t *testing.T) {
		headers := map[string]string{"foo": "bar"}
		ctxWithHeaders := WithRequestHeaders(context.TODO(), headers)
		headersFromContext := GetRequestHeadersFromContext(ctxWithHeaders)
		require.Equal(t, headers, headersFromContext)
	})
}
