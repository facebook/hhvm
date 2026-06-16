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

//! [`TypeSystemDigest`] trait implementations for thrift-generated type system types.
//!
//! Every implementation here must produce byte-identical hash output to the
//! corresponding C++ code in `TypeSystemDigest.cpp`.

use std::collections::BTreeMap;

use record::SerializableRecord;
use record::SerializableRecordMapEntry;
use type_id::TypeId;
use type_system::SerializableEnumDefinition;
use type_system::SerializableFieldDefinition;
use type_system::SerializableOpaqueAliasDefinition;
use type_system::SerializableStructDefinition;
use type_system::SerializableTypeDefinition;
use type_system::SerializableTypeDefinitionEntry;
use type_system::SerializableTypeSystem;
use type_system::SerializableUnionDefinition;

use crate::TYPE_SYSTEM_DIGEST_VERSION;
use crate::TypeSystemDigest;
use crate::hasher::Hasher;

/// Maps a `SerializableRecord` variant to its thrift field ID.
///
/// Must match `SerializableRecordUnion::getType()` in C++ (which returns the
/// thrift field discriminant). Note the gap: field ID 4 is unused in
/// `record.thrift`.
fn record_thrift_field_id(record: &SerializableRecord) -> i32 {
    match record {
        SerializableRecord::boolDatum(_) => 1,
        SerializableRecord::int8Datum(_) => 2,
        SerializableRecord::int16Datum(_) => 3,
        // field 4 is missing in record.thrift
        SerializableRecord::int32Datum(_) => 5,
        SerializableRecord::int64Datum(_) => 6,
        SerializableRecord::float32Datum(_) => 7,
        SerializableRecord::float64Datum(_) => 8,
        SerializableRecord::textDatum(_) => 9,
        SerializableRecord::byteArrayDatum(_) => 10,
        SerializableRecord::fieldSetDatum(_) => 11,
        SerializableRecord::listDatum(_) => 12,
        SerializableRecord::setDatum(_) => 13,
        SerializableRecord::mapDatum(_) => 14,
        SerializableRecord::UnknownField(id) => {
            panic!("TypeSystemDigest: unhandled SerializableRecord variant (field {id})")
        }
    }
}

/// Maps a `SerializableTypeDefinition` variant to its thrift field ID.
///
/// Must match `SerializableTypeDefinition::getType()` in C++.
fn type_def_thrift_field_id(def: &SerializableTypeDefinition) -> i32 {
    match def {
        SerializableTypeDefinition::structDef(_) => 1,
        SerializableTypeDefinition::unionDef(_) => 2,
        SerializableTypeDefinition::enumDef(_) => 3,
        SerializableTypeDefinition::opaqueAliasDef(_) => 4,
        SerializableTypeDefinition::UnknownField(id) => {
            panic!("TypeSystemDigest: unhandled SerializableTypeDefinition variant (field {id})")
        }
    }
}

impl TypeSystemDigest for TypeId {
    fn hash_into(&self, h: &mut Hasher) {
        match self {
            TypeId::boolType(_) => h.hash(&1i32),
            TypeId::byteType(_) => h.hash(&2i32),
            TypeId::i16Type(_) => h.hash(&3i32),
            TypeId::i32Type(_) => h.hash(&4i32),
            TypeId::i64Type(_) => h.hash(&5i32),
            TypeId::floatType(_) => h.hash(&6i32),
            TypeId::doubleType(_) => h.hash(&7i32),
            TypeId::stringType(_) => h.hash(&8i32),
            TypeId::binaryType(_) => h.hash(&9i32),
            TypeId::anyType(_) => h.hash(&10i32),
            TypeId::userDefinedType(uri) => {
                h.hash(&11i32);
                h.hash(uri.as_str());
            }
            TypeId::listType(list) => {
                h.hash(&12i32);
                hash_type_id_field(&list.elementType, h);
            }
            TypeId::setType(set) => {
                h.hash(&13i32);
                hash_type_id_field(&set.elementType, h);
            }
            TypeId::mapType(map) => {
                h.hash(&14i32);
                hash_type_id_field(&map.keyType, h);
                hash_type_id_field(&map.valueType, h);
            }
            TypeId::UnknownField(id) => {
                panic!("TypeSystemDigest: unhandled TypeId variant (field {id})")
            }
        }
    }
}

fn hash_type_id_field(field: &Option<Box<TypeId>>, h: &mut Hasher) {
    match field {
        Some(inner) => h.hash(inner.as_ref()),
        None => h.hash(&0u8),
    }
}

impl TypeSystemDigest for SerializableRecord {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&record_thrift_field_id(self));

        match self {
            SerializableRecord::boolDatum(v) => h.hash(v),
            SerializableRecord::int8Datum(v) => h.hash(v),
            SerializableRecord::int16Datum(v) => h.hash(v),
            SerializableRecord::int32Datum(v) => h.hash(v),
            SerializableRecord::int64Datum(v) => h.hash(v),
            SerializableRecord::float32Datum(v) => h.hash(v),
            SerializableRecord::float64Datum(v) => h.hash(v),
            SerializableRecord::textDatum(v) => h.hash(v.as_str()),
            SerializableRecord::byteArrayDatum(v) => h.hash(v.as_slice()),
            SerializableRecord::fieldSetDatum(field_set) => hash_field_set(field_set, h),
            SerializableRecord::listDatum(list) => {
                for elem in list {
                    h.hash(elem);
                }
            }
            SerializableRecord::setDatum(set) => {
                h.hash_unordered_by_digest(set.iter(), |sub_h, elem| sub_h.hash(*elem));
            }
            SerializableRecord::mapDatum(map) => {
                h.hash_map_by_key_digest(
                    map.iter(),
                    |sub_h, entry| sub_h.hash(&entry.key),
                    |sub_h, entry| {
                        sub_h.hash(&entry.key);
                        sub_h.hash(&entry.value);
                    },
                );
            }
            SerializableRecord::UnknownField(_) => unreachable!(),
        }
    }
}

/// BTreeMap is already sorted by key, matching C++ `forEachSortedByKey`.
fn hash_field_set(field_set: &BTreeMap<id::FieldId, SerializableRecord>, h: &mut Hasher) {
    for (field_id, record) in field_set {
        h.hash(field_id);
        h.hash(record);
    }
}

fn hash_annotations(annotations: &BTreeMap<String, SerializableRecord>, h: &mut Hasher) {
    if !h.include_annotations() {
        return;
    }
    h.hash_unordered_by_digest(annotations.iter(), |sub_h, (key, value)| {
        sub_h.hash(key.as_str());
        sub_h.hash(*value);
    });
}

impl TypeSystemDigest for SerializableFieldDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.identity.id);
        h.hash(self.identity.name.as_str());
        h.hash(&self.presence.0);
        h.hash(&self.r#type);

        if h.include_custom_default_values() {
            if let Some(ref custom_default) = self.customDefaultPartialRecord {
                h.hash(custom_default);
            }
        }
        hash_annotations(&self.annotations, h);
    }
}

impl TypeSystemDigest for SerializableStructDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        let mut sorted_fields: Vec<_> = self.fields.iter().collect();
        sorted_fields.sort_by_key(|f| f.identity.id);
        for field in sorted_fields {
            h.hash(field);
        }

        h.hash(&self.isSealed);
        hash_annotations(&self.annotations, h);
    }
}

impl TypeSystemDigest for SerializableUnionDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        let mut sorted_fields: Vec<_> = self.fields.iter().collect();
        sorted_fields.sort_by_key(|f| f.identity.id);
        for field in sorted_fields {
            h.hash(field);
        }

        h.hash(&self.isSealed);
        hash_annotations(&self.annotations, h);
    }
}

impl TypeSystemDigest for SerializableEnumDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        let mut sorted_values: Vec<_> = self.values.iter().collect();
        sorted_values.sort_by_key(|v| v.datum);
        for value in sorted_values {
            h.hash(value.name.as_str());
            h.hash(&value.datum);
            hash_annotations(&value.annotations, h);
        }

        hash_annotations(&self.annotations, h);
    }
}

impl TypeSystemDigest for SerializableOpaqueAliasDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.targetType);
        hash_annotations(&self.annotations, h);
    }
}

impl TypeSystemDigest for SerializableTypeDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&type_def_thrift_field_id(self));

        match self {
            SerializableTypeDefinition::structDef(def) => h.hash(def),
            SerializableTypeDefinition::unionDef(def) => h.hash(def),
            SerializableTypeDefinition::enumDef(def) => h.hash(def),
            SerializableTypeDefinition::opaqueAliasDef(def) => h.hash(def),
            SerializableTypeDefinition::UnknownField(_) => unreachable!(),
        }
    }
}

impl TypeSystemDigest for SerializableTypeSystem {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&TYPE_SYSTEM_DIGEST_VERSION);

        // BTreeMap is already sorted by key (URI string)
        for (uri, entry) in &self.types {
            h.hash(uri.as_str());
            h.hash(&entry.definition);
        }
    }
}

impl TypeSystemDigest for SerializableTypeDefinitionEntry {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.definition);
    }
}

impl TypeSystemDigest for SerializableRecordMapEntry {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.key);
        h.hash(&self.value);
    }
}
