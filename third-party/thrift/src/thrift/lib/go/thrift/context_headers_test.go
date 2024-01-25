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
)

func TestSomeHeaders(t *testing.T) {
	ctx := context.Background()
	want := map[string]string{"key1": "value1", "key2": "value2"}
	var err error
	for key, value := range want {
		ctx, err = AddHeader(ctx, key, value)
		if err != nil {
			t.Fatal(err)
		}
	}
	transport := NewHeaderTransport(NewMemoryBuffer())
	if err := setHeaders(ctx, transport); err != nil {
		t.Fatal(err)
	}
	got := transport.Headers()
	assert.Equal(t, want, got)
}

// somewhere we are still passing context as nil, so we need to support this for now
func TestSetNilHeaders(t *testing.T) {
	transport := NewHeaderTransport(NewMemoryBuffer())
	if err := setHeaders(nil, transport); err != nil {
		t.Fatal(err)
	}
}
