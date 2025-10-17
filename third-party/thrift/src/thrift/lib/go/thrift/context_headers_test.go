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

	"github.com/stretchr/testify/require"
)

func TestGetRequestHeadersFromContext(t *testing.T) {
	t.Run("nil context", func(t *testing.T) {
		headers := GetRequestHeadersFromContext(nil)
		require.Nil(t, headers)
	})
	t.Run("empty context", func(t *testing.T) {
		headers := GetRequestHeadersFromContext(context.TODO())
		require.Empty(t, headers)
	})
	t.Run("context with existing headers", func(t *testing.T) {
		headers := map[string]string{"foo": "bar"}
		reqCtx := &RequestContext{}
		reqCtx.SetReadHeaders(headers)
		ctxWithHeaders := WithRequestContext(context.TODO(), reqCtx)
		headersFromContext := GetRequestHeadersFromContext(ctxWithHeaders)
		require.Equal(t, headers, headersFromContext)
	})
	t.Run("ensure map copy", func(t *testing.T) {
		// Ensure that the user can't modify the map returned from the context
		originalHeaders := map[string]string{"foo": "bar"}
		reqCtx := &RequestContext{}
		reqCtx.SetReadHeaders(originalHeaders)
		ctxWithHeaders := WithRequestContext(context.TODO(), reqCtx)
		// Get headers map and modify it
		headersFromContext1 := GetRequestHeadersFromContext(ctxWithHeaders)
		headersFromContext1["foo"] = "123"
		headersFromContext1["bar"] = "123"
		headersFromContext2 := GetRequestHeadersFromContext(ctxWithHeaders)
		require.Equal(t, originalHeaders, headersFromContext2)
	})
}
