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

package gotest

import (
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/stretchr/testify/require"
	"github.com/facebook/fbthrift/thrift/test/go/if/dummy"
)

// TestErrorRegistryDummyException verifies that the Go code generator
// emits an InternalRegisterError call for every Thrift exception with
// the correct blame/kind/safety values derived from IDL annotations.
//
// DummyException is declared as: `safe permanent server exception` in
// xplat/thrift/test/go/if/dummy.thrift.
func TestErrorRegistryDummyException(t *testing.T) {
	spec := types.InternalErrorRegistryGet(&dummy.DummyException{})
	require.NotNil(t, spec, "DummyException must be registered in the error registry")
	require.Equal(t, "DummyException", spec.TypeName)
	require.Equal(t, types.ErrorBlameServer, spec.Blame)
	require.Equal(t, types.ErrorKindPermanent, spec.Kind)
	require.Equal(t, types.ErrorSafetySafe, spec.Safety)
}
