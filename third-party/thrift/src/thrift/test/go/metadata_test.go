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

	"github.com/facebook/fbthrift/thrift/lib/thrift/metadata"
	"github.com/stretchr/testify/require"
	"thrift/test/go/if/thrifttest"
)

func TestMetadata(t *testing.T) {
	// This test asserts that Thrift metadata for a given package has
	// all the necessary types, by performing a traversal of the entire
	// metadata tree to ensure there are no missing type references.

	completeMetadata := thrifttest.GetThriftMetadata()

	for _, strct := range completeMetadata.GetStructs() {
		for _, field := range strct.GetFields() {
			verifyThriftType(t, field.GetType(), completeMetadata)
		}
	}

	for _, exception := range completeMetadata.GetExceptions() {
		for _, field := range exception.GetFields() {
			verifyThriftType(t, field.GetType(), completeMetadata)
		}
	}

	for _, service := range completeMetadata.GetServices() {
		for _, function := range service.GetFunctions() {
			verifyThriftType(t, function.GetReturnType(), completeMetadata)
			for _, argument := range function.GetArguments() {
				verifyThriftType(t, argument.GetType(), completeMetadata)
			}
			for _, exception := range function.GetExceptions() {
				verifyThriftType(t, exception.GetType(), completeMetadata)
			}
		}
	}
}

func verifyThriftType(t *testing.T, thriftType *metadata.ThriftType, allMetadata *metadata.ThriftMetadata) {
	require.NotNil(t, thriftType)

	switch {
	case thriftType.TPrimitive != nil:
		// Do nothing
	case thriftType.TList != nil:
		verifyThriftType(t, thriftType.TList.GetValueType(), allMetadata)
	case thriftType.TSet != nil:
		verifyThriftType(t, thriftType.TSet.GetValueType(), allMetadata)
	case thriftType.TMap != nil:
		verifyThriftType(t, thriftType.TMap.GetKeyType(), allMetadata)
		verifyThriftType(t, thriftType.TMap.GetValueType(), allMetadata)
	case thriftType.TTypedef != nil:
		verifyThriftType(t, thriftType.TTypedef.GetUnderlyingType(), allMetadata)
	case thriftType.TStream != nil:
		verifyThriftType(t, thriftType.TStream.GetInitialResponseType(), allMetadata)
		verifyThriftType(t, thriftType.TStream.GetElemType(), allMetadata)
	case thriftType.TSink != nil:
		verifyThriftType(t, thriftType.TSink.GetInitialResponseType(), allMetadata)
		verifyThriftType(t, thriftType.TSink.GetElemType(), allMetadata)
		verifyThriftType(t, thriftType.TSink.GetFinalResponseType(), allMetadata)
	case thriftType.TEnum != nil:
		enumName := thriftType.TEnum.GetName()
		require.Contains(t, allMetadata.GetEnums(), enumName)
	case thriftType.TStruct != nil:
		structName := thriftType.TStruct.GetName()
		// TStruct can be either a struct or an exception
		_, ok1 := allMetadata.GetStructs()[structName]
		_, ok2 := allMetadata.GetExceptions()[structName]
		if !ok1 && !ok2 {
			t.Fatalf("struct definition not found for %s", structName)
		}
	case thriftType.TUnion != nil:
		unionName := thriftType.TUnion.GetName()
		require.Contains(t, allMetadata.GetStructs(), unionName)
	}
}

func TestGetThriftStructMetadata(t *testing.T) {
	value := thrifttest.NewBonk()
	structMetadata := value.GetThriftStructMetadata()
	require.Len(t, structMetadata.StructuredAnnotations, 1)
	annotation := structMetadata.StructuredAnnotations[0]
	require.Equal(t, "thrift.ReserveIds", annotation.Type.Name)
	require.Contains(t, annotation.Fields, "ids")
	require.NotNil(t, annotation.Fields["ids"].CvList)
	require.Len(t, annotation.Fields["ids"].CvList, 1)
	require.NotNil(t, annotation.Fields["ids"].CvList[0].CvInteger)
	require.EqualValues(t, 12345, *annotation.Fields["ids"].CvList[0].CvInteger)
}
