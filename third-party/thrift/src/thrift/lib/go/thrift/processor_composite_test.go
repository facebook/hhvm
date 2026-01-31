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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	dummygen "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
	"github.com/stretchr/testify/assert"
)

// TestCompositeProcessor verifies that CompositeProcessor correctly includes
// processor functions, function-to-service mappings, metadata, and interaction processors.
func TestCompositeProcessor(t *testing.T) {
	composite := NewCompositeProcessor()
	processor := dummygen.NewDummyProcessor(&dummy.DummyHandler{})

	composite.Include(processor)

	// Verify service function is registered
	assert.NotNil(t, composite.ProcessorFunctionMap()["Ping"])

	// Verify function-to-service mappings
	assert.Equal(t, "Dummy", composite.FunctionServiceMap()["Ping"])

	// Verify metadata is populated
	assert.NotNil(t, composite.GetThriftMetadata().GetServices())

	// Verify interaction processors are collected and have FunctionServiceMap
	interactionProcessors := composite.GetInteractionProcessors()
	assert.GreaterOrEqual(t, len(interactionProcessors), 1)
	for _, ip := range interactionProcessors {
		fsm := ip.FunctionServiceMap()
		if serviceName, ok := fsm["add"]; ok {
			assert.Equal(t, "Summer", serviceName)
		}
	}
}
