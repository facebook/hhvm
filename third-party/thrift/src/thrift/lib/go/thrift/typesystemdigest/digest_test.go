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

package typesystemdigest

import (
	"encoding/hex"
	"testing"

	"github.com/stretchr/testify/require"

	expectedvalues "thrift/lib/cpp2/dynamic/test/digest_expected_values"
	record "thrift/lib/thrift/record"
	typeid "thrift/lib/thrift/type_id"
	typesystem "thrift/lib/thrift/type_system"
)

func TestDigestMatchesCrossLanguageGoldens(t *testing.T) {
	tests := []struct {
		name       string
		typeSystem *typesystem.SerializableTypeSystem
		expected   string
	}{
		{
			name:       "empty",
			typeSystem: &typesystem.SerializableTypeSystem{},
			expected:   expectedvalues.DIGEST_EMPTY,
		},
		{
			name:       "single empty struct",
			typeSystem: typeSystem(map[typeid.Uri]*typesystem.SerializableTypeDefinitionEntry{
				"meta.com/test/Empty": structEntry(nil),
			}),
			expected: expectedvalues.DIGEST_SINGLE_EMPTY_STRUCT,
		},
		{
			name:       "struct with fields",
			typeSystem: typeSystem(map[typeid.Uri]*typesystem.SerializableTypeDefinitionEntry{
				"meta.com/test/Person": personStructEntry(),
			}),
			expected: expectedvalues.DIGEST_STRUCT_WITH_FIELDS,
		},
		{
			name: "enum",
			typeSystem: typeSystem(map[typeid.Uri]*typesystem.SerializableTypeDefinitionEntry{
				"meta.com/test/Status": enumEntry([]*typesystem.SerializableEnumValueDefinition{
					enumValue("ACTIVE", 1),
					enumValue("INACTIVE", 2),
					enumValue("PENDING", 3),
				}),
			}),
			expected: expectedvalues.DIGEST_ENUM,
		},
		{
			name: "multiple types",
			typeSystem: typeSystem(map[typeid.Uri]*typesystem.SerializableTypeDefinitionEntry{
				"meta.com/test/multi/Person": personStructEntry(),
				"meta.com/test/multi/Status": enumEntry([]*typesystem.SerializableEnumValueDefinition{
					enumValue("ACTIVE", 1),
					enumValue("INACTIVE", 2),
				}),
				"meta.com/test/multi/UserId": opaqueAliasEntry(i64TypeID()),
			}),
			expected: expectedvalues.DIGEST_MULTIPLE_TYPES,
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			digest, err := Digest(test.typeSystem)
			require.NoError(t, err)
			require.Equal(t, test.expected, hex.EncodeToString(digest))
		})
	}
}

func TestTypeIDDigestMatchesCrossLanguageGoldens(t *testing.T) {
	uri := typeid.Uri("meta.com/test/MyStruct")
	tests := []struct {
		name     string
		typeID   *typeid.TypeId
		expected string
	}{
		{
			name:     "bool",
			typeID:   boolTypeID(),
			expected: expectedvalues.DIGEST_TYPE_ID_BOOL,
		},
		{
			name:     "i32",
			typeID:   i32TypeID(),
			expected: expectedvalues.DIGEST_TYPE_ID_I32,
		},
		{
			name:     "string",
			typeID:   stringTypeID(),
			expected: expectedvalues.DIGEST_TYPE_ID_STRING,
		},
		{
			name:     "uri",
			typeID:   &typeid.TypeId{UserDefinedType: &uri},
			expected: expectedvalues.DIGEST_TYPE_ID_URI,
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			digest, err := TypeIDDigest(test.typeID)
			require.NoError(t, err)
			require.Equal(t, test.expected, hex.EncodeToString(digest))
		})
	}
}

func TestDigestIgnoresInputOrder(t *testing.T) {
	first := typeSystem(map[typeid.Uri]*typesystem.SerializableTypeDefinitionEntry{
		"meta.com/order/Struct": structEntry([]*typesystem.SerializableFieldDefinition{
			field(2, "second", typesystem.PresenceQualifier_UNQUALIFIED, i32TypeID()),
			field(1, "first", typesystem.PresenceQualifier_UNQUALIFIED, stringTypeID()),
		}),
		"meta.com/order/Enum": enumEntry([]*typesystem.SerializableEnumValueDefinition{
			enumValue("B", 2),
			enumValue("A", 1),
		}),
	})
	second := typeSystem(map[typeid.Uri]*typesystem.SerializableTypeDefinitionEntry{
		"meta.com/order/Enum": enumEntry([]*typesystem.SerializableEnumValueDefinition{
			enumValue("A", 1),
			enumValue("B", 2),
		}),
		"meta.com/order/Struct": structEntry([]*typesystem.SerializableFieldDefinition{
			field(1, "first", typesystem.PresenceQualifier_UNQUALIFIED, stringTypeID()),
			field(2, "second", typesystem.PresenceQualifier_UNQUALIFIED, i32TypeID()),
		}),
	})

	firstDigest, err := Digest(first)
	require.NoError(t, err)
	secondDigest, err := Digest(second)
	require.NoError(t, err)
	require.Equal(t, firstDigest, secondDigest)
}

func TestStructuralModeExcludesAnnotationsAndDefaults(t *testing.T) {
	withExtras := field(1, "field", typesystem.PresenceQualifier_OPTIONAL, i32TypeID())
	withExtras.CustomDefaultPartialRecord = int32Record(1)
	withExtras.Annotations = map[typeid.Uri]*record.SerializableRecord{
		"meta.com/test/Annotation": boolRecord(true),
	}
	withoutExtras := field(1, "field", typesystem.PresenceQualifier_OPTIONAL, i32TypeID())

	withExtrasTypeSystem := typeSystem(map[typeid.Uri]*typesystem.SerializableTypeDefinitionEntry{
		"meta.com/test/Struct": structEntry([]*typesystem.SerializableFieldDefinition{withExtras}),
	})
	withoutExtrasTypeSystem := typeSystem(map[typeid.Uri]*typesystem.SerializableTypeDefinitionEntry{
		"meta.com/test/Struct": structEntry([]*typesystem.SerializableFieldDefinition{withoutExtras}),
	})

	fullWithExtras, err := Digest(withExtrasTypeSystem)
	require.NoError(t, err)
	fullWithoutExtras, err := Digest(withoutExtrasTypeSystem)
	require.NoError(t, err)
	require.NotEqual(t, fullWithExtras, fullWithoutExtras)

	structuralWithExtras, err := DigestWithMode(withExtrasTypeSystem, ModeStructural)
	require.NoError(t, err)
	structuralWithoutExtras, err := DigestWithMode(withoutExtrasTypeSystem, ModeStructural)
	require.NoError(t, err)
	require.Equal(t, structuralWithExtras, structuralWithoutExtras)
}

func typeSystem(
	types map[typeid.Uri]*typesystem.SerializableTypeDefinitionEntry,
) *typesystem.SerializableTypeSystem {
	return &typesystem.SerializableTypeSystem{Types: types}
}

func structEntry(
	fields []*typesystem.SerializableFieldDefinition,
) *typesystem.SerializableTypeDefinitionEntry {
	return &typesystem.SerializableTypeDefinitionEntry{
		Definition: &typesystem.SerializableTypeDefinition{
			StructDef: &typesystem.SerializableStructDefinition{
				Fields: fields,
			},
		},
	}
}

func personStructEntry() *typesystem.SerializableTypeDefinitionEntry {
	return structEntry([]*typesystem.SerializableFieldDefinition{
		field(1, "name", typesystem.PresenceQualifier_UNQUALIFIED, stringTypeID()),
		field(2, "age", typesystem.PresenceQualifier_OPTIONAL, i32TypeID()),
	})
}

func enumEntry(
	values []*typesystem.SerializableEnumValueDefinition,
) *typesystem.SerializableTypeDefinitionEntry {
	return &typesystem.SerializableTypeDefinitionEntry{
		Definition: &typesystem.SerializableTypeDefinition{
			EnumDef: &typesystem.SerializableEnumDefinition{
				Values: values,
			},
		},
	}
}

func enumValue(name string, datum int32) *typesystem.SerializableEnumValueDefinition {
	return &typesystem.SerializableEnumValueDefinition{
		Name:  name,
		Datum: datum,
	}
}

func opaqueAliasEntry(
	targetType *typeid.TypeId,
) *typesystem.SerializableTypeDefinitionEntry {
	return &typesystem.SerializableTypeDefinitionEntry{
		Definition: &typesystem.SerializableTypeDefinition{
			OpaqueAliasDef: &typesystem.SerializableOpaqueAliasDefinition{
				TargetType: targetType,
			},
		},
	}
}

func field(
	id typesystem.FieldId,
	name typesystem.FieldName,
	presence typesystem.PresenceQualifier,
	fieldType *typeid.TypeId,
) *typesystem.SerializableFieldDefinition {
	return &typesystem.SerializableFieldDefinition{
		Identity: &typesystem.FieldIdentity{
			Id:   id,
			Name: name,
		},
		Presence: presence,
		Type:     fieldType,
	}
}

func boolTypeID() *typeid.TypeId {
	return &typeid.TypeId{BoolType: &typeid.BoolTypeId{}}
}

func i32TypeID() *typeid.TypeId {
	return &typeid.TypeId{I32Type: &typeid.I32TypeId{}}
}

func i64TypeID() *typeid.TypeId {
	return &typeid.TypeId{I64Type: &typeid.I64TypeId{}}
}

func stringTypeID() *typeid.TypeId {
	return &typeid.TypeId{StringType: &typeid.StringTypeId{}}
}

func boolRecord(value bool) *record.SerializableRecord {
	return &record.SerializableRecord{BoolDatum: &value}
}

func int32Record(value int32) *record.SerializableRecord {
	return &record.SerializableRecord{Int32Datum: &value}
}
