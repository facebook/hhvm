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

package types

import (
	"crypto/sha256"
	"testing"

	"github.com/stretchr/testify/require"
)

func TestRegisterTypeAndGetFromURI(t *testing.T) {
	uri := "test/module/DummyStruct"
	spec := &TypeSpec{
		FullName: "test.module.DummyStruct",
		CodecStructSpec: &CodecStructSpec{
			ScopedName: "test.module.DummyStruct",
			IsUnion:    false,
		},
	}

	InternalRegisterType(uri, spec)

	result, ok := InternalTypeRegistryGetFromURI(uri)
	require.True(t, ok)
	require.NotNil(t, result)
	require.Equal(t, spec, result)
	require.Equal(t, "test.module.DummyStruct", result.FullName)
}

func TestRegisterTypeWithThriftPrefix(t *testing.T) {
	uri := "fbthrift://test/module/PrefixedStruct"
	spec := &TypeSpec{
		FullName: "test.module.PrefixedStruct",
		CodecPrimitiveSpec: &CodecPrimitiveSpec{
			PrimitiveType: CODEC_PRIMITIVE_TYPE_STRING,
		},
	}

	InternalRegisterType(uri, spec)

	result, ok := InternalTypeRegistryGetFromURI("test/module/PrefixedStruct")
	require.True(t, ok)
	require.NotNil(t, result)
	require.Equal(t, spec, result)
}

func TestGetFromURINotFound(t *testing.T) {
	result, ok := InternalTypeRegistryGetFromURI("nonexistent/type")
	require.False(t, ok)
	require.Nil(t, result)
}

func TestGetFromHash(t *testing.T) {
	uri := "test/module/HashStruct"
	spec := &TypeSpec{
		FullName: "test.module.HashStruct",
	}

	InternalRegisterType(uri, spec)

	h := sha256.New()
	h.Write([]byte("fbthrift://" + uri))
	hash := h.Sum(nil)

	result32, ok := InternalTypeRegistryGetFromHash(hash[0:32])
	require.True(t, ok)
	require.Equal(t, spec, result32)

	result16, ok := InternalTypeRegistryGetFromHash(hash[0:16])
	require.True(t, ok)
	require.Equal(t, spec, result16)

	result8, ok := InternalTypeRegistryGetFromHash(hash[0:8])
	require.True(t, ok)
	require.Equal(t, spec, result8)
}

func TestGetFromHashInvalidLength(t *testing.T) {
	result, ok := InternalTypeRegistryGetFromHash([]byte{1, 2, 3, 4})
	require.False(t, ok)
	require.Nil(t, result)
}

func TestGetFromHashNotFound(t *testing.T) {
	nonExistentHash := make([]byte, 32)
	for i := range nonExistentHash {
		nonExistentHash[i] = byte(i)
	}

	result, ok := InternalTypeRegistryGetFromHash(nonExistentHash)
	require.False(t, ok)
	require.Nil(t, result)
}
