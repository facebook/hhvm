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
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestHeaderProtocolSomePersistentHeaders(t *testing.T) {
	protocol := NewHeaderProtocol(NewHeaderTransport(NewMemoryBuffer()))
	protocol.(PersistentHeaders).SetPersistentHeader("key", "value")
	v, ok := protocol.(PersistentHeaders).GetPersistentHeader("key")
	assert.True(t, ok)
	assert.Equal(t, "value", v)
}

func TestRocketProtocolSomePersistentHeaders(t *testing.T) {
	protocol := NewRocketProtocol(NewRocketTransport(newPipe()))
	protocol.(PersistentHeaders).SetPersistentHeader("key", "value")
	v, ok := protocol.(PersistentHeaders).GetPersistentHeader("key")
	assert.True(t, ok)
	assert.Equal(t, "value", v)
}

func TestUpgradeToRocketProtocolSomePersistentHeaders(t *testing.T) {
	protocol := NewUpgradeToRocketProtocol(
		NewRocketProtocol(NewRocketTransport(newPipe())),
		NewHeaderProtocol(NewHeaderTransport(NewMemoryBuffer())),
	)
	protocol.(PersistentHeaders).SetPersistentHeader("key", "value")
	v, ok := protocol.(PersistentHeaders).GetPersistentHeader("key")
	assert.True(t, ok)
	assert.Equal(t, "value", v)
}
