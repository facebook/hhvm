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

package typesystem

import (
	"testing"

	"github.com/stretchr/testify/require"
)

func TestTrueTypeResolvesTypedefChain(t *testing.T) {
	leaf := Primitive(TypeRefKindI32)
	inner := &testTypedefDefinition{
		name:   "InnerAlias",
		uri:    "meta.com/test/InnerAlias",
		target: leaf,
	}
	outer := &testTypedefDefinition{
		name:   "OuterAlias",
		uri:    "meta.com/test/OuterAlias",
		target: UserDefined(TypeRefKindTypedef, inner),
	}

	trueType, err := UserDefined(TypeRefKindTypedef, outer).TrueType()
	require.NoError(t, err)
	require.Equal(t, TypeRefKindI32, trueType.Kind())
}

func TestTrueTypeRejectsTypedefCycle(t *testing.T) {
	first := &testTypedefDefinition{
		name: "FirstAlias",
		uri:  "meta.com/test/FirstAlias",
	}
	second := &testTypedefDefinition{
		name: "SecondAlias",
		uri:  "meta.com/test/SecondAlias",
	}
	first.target = UserDefined(TypeRefKindTypedef, second)
	second.target = UserDefined(TypeRefKindTypedef, first)

	_, err := UserDefined(TypeRefKindTypedef, first).TrueType()
	require.ErrorContains(t, err, "cyclic typedef chain detected")
}

type testTypedefDefinition struct {
	name   string
	uri    string
	target TypeRef
}

func (d *testTypedefDefinition) Name() string {
	return d.name
}

func (d *testTypedefDefinition) URI() string {
	return d.uri
}

func (*testTypedefDefinition) TypeKind() TypeRefKind {
	return TypeRefKindTypedef
}

func (d *testTypedefDefinition) TargetType() (TypeRef, error) {
	return d.target, nil
}
