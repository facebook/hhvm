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

//! `TypeSystemDigest` impls for runtime node types.
//!
//! These produce byte-identical digests to the serializable-type impls in
//! `type_system_digest::impls` and to the C++ `TypeSystemDigest` implementation.

use type_system_digest::TYPE_SYSTEM_DIGEST_VERSION;
use type_system_digest::TypeSystemDigest;
use type_system_digest::hasher::Hasher;

use crate::field::AnnotationsMap;
use crate::field::FieldDefinition;
use crate::nodes::EnumNode;
use crate::nodes::EnumValue;
use crate::nodes::OpaqueAliasNode;
use crate::nodes::StructNode;
use crate::nodes::UnionNode;
use crate::structured_node::StructuredNode;
use crate::type_ref::DefinitionRef;
use crate::type_system::TypeSystem;

const STRUCT_DEF_FIELD_ID: i32 = 1;
const UNION_DEF_FIELD_ID: i32 = 2;
const ENUM_DEF_FIELD_ID: i32 = 3;
const OPAQUE_ALIAS_DEF_FIELD_ID: i32 = 4;

pub(crate) fn hash_type_system_into(ts: &(impl TypeSystem + ?Sized), h: &mut Hasher) {
    h.hash(&TYPE_SYSTEM_DIGEST_VERSION);

    let mut uris: Vec<&str> = ts.known_uris().collect();
    uris.sort_unstable();

    for uri in uris {
        if let Some(def) = ts.get(uri) {
            h.hash(uri);
            def.hash_into(h);
        }
    }
}

impl TypeSystemDigest for AnnotationsMap {
    fn hash_into(&self, h: &mut Hasher) {
        if !h.include_annotations() {
            return;
        }
        h.hash_unordered_by_digest(self.iter(), |sub_h, (key, value)| {
            sub_h.hash(key.as_str());
            sub_h.hash(*value);
        });
    }
}

impl TypeSystemDigest for FieldDefinition {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.id());
        h.hash(self.name());
        h.hash(&self.presence().0);
        h.hash(&self.type_ref().id());

        if h.include_custom_default_values() {
            if let Some(custom_default) = self.custom_default() {
                h.hash(custom_default);
            }
        }
        h.hash(self.annotations());
    }
}

impl TypeSystemDigest for StructNode {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&STRUCT_DEF_FIELD_ID);

        let mut sorted_fields: Vec<&FieldDefinition> = self.fields().iter().collect();
        sorted_fields.sort_by_key(|f| f.id());
        for field in sorted_fields {
            field.hash_into(h);
        }
        h.hash(&self.is_sealed());
        h.hash(self.annotations());
    }
}

impl TypeSystemDigest for UnionNode {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&UNION_DEF_FIELD_ID);

        let mut sorted_fields: Vec<&FieldDefinition> = self.fields().iter().collect();
        sorted_fields.sort_by_key(|f| f.id());
        for field in sorted_fields {
            field.hash_into(h);
        }
        h.hash(&self.is_sealed());
        h.hash(self.annotations());
    }
}

impl TypeSystemDigest for EnumValue {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(self.name.as_str());
        h.hash(&self.value);
        h.hash(self.annotations());
    }
}

impl TypeSystemDigest for EnumNode {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&ENUM_DEF_FIELD_ID);

        let mut sorted_values: Vec<_> = self.values().iter().collect();
        sorted_values.sort_by_key(|v| v.value);
        for v in sorted_values {
            v.hash_into(h);
        }

        h.hash(self.annotations());
    }
}

impl TypeSystemDigest for OpaqueAliasNode {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&OPAQUE_ALIAS_DEF_FIELD_ID);
        h.hash(&self.target_type().id());
        h.hash(self.annotations());
    }
}

impl TypeSystemDigest for DefinitionRef {
    fn hash_into(&self, h: &mut Hasher) {
        match self {
            Self::Struct(s) => s.as_ref().hash_into(h),
            Self::Union(u) => u.as_ref().hash_into(h),
            Self::Enum(e) => e.as_ref().hash_into(h),
            Self::OpaqueAlias(o) => o.as_ref().hash_into(h),
        }
    }
}

#[cfg(test)]
mod tests {
    use std::sync::Arc;

    use type_system_digest::TypeSystemDigest;

    use crate::field::AnnotationsMap;
    use crate::field::FieldDefinition;
    use crate::field::FieldIdentity;
    use crate::field::PresenceQualifier;
    use crate::nodes::EnumNode;
    use crate::nodes::EnumValue;
    use crate::nodes::OpaqueAliasNode;
    use crate::nodes::StructNode;
    use crate::nodes::UnionNode;
    use crate::type_ref::TypeRef;
    use crate::type_system::BasicTypeSystem;
    use crate::type_system::DefinitionNode;
    use crate::type_system::TypeSystem;

    fn hex_digest(value: &impl TypeSystemDigest) -> String {
        value.digest().iter().map(|b| format!("{b:02x}")).collect()
    }

    fn make_ts(defs: Vec<(&str, DefinitionNode)>) -> BasicTypeSystem {
        BasicTypeSystem::new(
            defs.into_iter()
                .map(|(uri, node)| (uri.to_owned(), node))
                .collect(),
        )
    }

    fn make_struct(uri: &str, fields: Vec<FieldDefinition>, is_sealed: bool) -> Arc<StructNode> {
        let node = Arc::new(StructNode::new_shell(
            uri.to_owned(),
            is_sealed,
            AnnotationsMap::default(),
        ));
        node.set_fields(fields).unwrap();
        node
    }

    fn make_union(uri: &str, fields: Vec<FieldDefinition>) -> Arc<UnionNode> {
        let node = Arc::new(UnionNode::new_shell(
            uri.to_owned(),
            false,
            AnnotationsMap::default(),
        ));
        node.set_fields(fields).unwrap();
        node
    }

    fn make_enum(uri: &str, values: Vec<(&str, i32)>) -> Arc<EnumNode> {
        Arc::new(
            EnumNode::new(
                uri.to_owned(),
                values
                    .into_iter()
                    .map(|(name, value)| {
                        EnumValue::new(name.to_owned(), value, AnnotationsMap::default())
                    })
                    .collect(),
                AnnotationsMap::default(),
            )
            .unwrap(),
        )
    }

    fn make_alias(uri: &str, target: TypeRef) -> Arc<OpaqueAliasNode> {
        let node = Arc::new(OpaqueAliasNode::new_shell(
            uri.to_owned(),
            AnnotationsMap::default(),
        ));
        node.set_target(target).unwrap();
        node
    }

    fn make_field(
        id: i16,
        name: &str,
        presence: PresenceQualifier,
        type_ref: TypeRef,
    ) -> FieldDefinition {
        FieldDefinition::new(
            FieldIdentity {
                id,
                name: name.to_owned(),
            },
            presence,
            type_ref,
            None,
            AnnotationsMap::default(),
        )
    }

    fn make_field_with_default(
        id: i16,
        name: &str,
        presence: PresenceQualifier,
        type_ref: TypeRef,
        default: record::SerializableRecord,
    ) -> FieldDefinition {
        FieldDefinition::new(
            FieldIdentity {
                id,
                name: name.to_owned(),
            },
            presence,
            type_ref,
            Some(default),
            AnnotationsMap::default(),
        )
    }

    fn make_annotated_field(
        id: i16,
        name: &str,
        presence: PresenceQualifier,
        type_ref: TypeRef,
        annotations: AnnotationsMap,
    ) -> FieldDefinition {
        FieldDefinition::new(
            FieldIdentity {
                id,
                name: name.to_owned(),
            },
            presence,
            type_ref,
            None,
            annotations,
        )
    }

    // Cross-language golden values (must match C++ digest_expected_values.thrift)

    #[test]
    fn cross_language_empty_type_system() {
        let ts = make_ts(vec![]);
        assert_eq!(
            hex_digest(&ts),
            "dbc1b4c900ffe48d575b5da5c638040125f65db0fe3e24494b76ea986457d986",
        );
    }

    #[test]
    fn cross_language_single_empty_struct() {
        let s = make_struct("meta.com/test/Empty", vec![], false);
        let ts = make_ts(vec![("meta.com/test/Empty", DefinitionNode::Struct(s))]);
        assert_eq!(
            hex_digest(&ts),
            "dde072c26c1b88f6a1d9f53a6e8232e4707bb55d1b5880f4e04791d13dc8c313",
        );
    }

    #[test]
    fn cross_language_struct_with_fields() {
        let s = make_struct(
            "meta.com/test/Person",
            vec![
                make_field(1, "name", PresenceQualifier::UNQUALIFIED, TypeRef::String),
                make_field(2, "age", PresenceQualifier::OPTIONAL, TypeRef::I32),
            ],
            false,
        );
        let ts = make_ts(vec![("meta.com/test/Person", DefinitionNode::Struct(s))]);
        assert_eq!(
            hex_digest(&ts),
            "32ddd3926cbca4e2b25f751a56c9c7a4dc217c45b56637ffec0af2adb6fbccc8",
        );
    }

    #[test]
    fn cross_language_enum() {
        let e = make_enum(
            "meta.com/test/Status",
            vec![("ACTIVE", 1), ("INACTIVE", 2), ("PENDING", 3)],
        );
        let ts = make_ts(vec![("meta.com/test/Status", DefinitionNode::Enum(e))]);
        assert_eq!(
            hex_digest(&ts),
            "26828b801c942b25ef863505af365b0e6a583de1c509c855d671970b0ef5052c",
        );
    }

    #[test]
    fn cross_language_multiple_types() {
        let s = make_struct(
            "meta.com/test/multi/Person",
            vec![
                make_field(1, "name", PresenceQualifier::UNQUALIFIED, TypeRef::String),
                make_field(2, "age", PresenceQualifier::OPTIONAL, TypeRef::I32),
            ],
            false,
        );
        let e = make_enum(
            "meta.com/test/multi/Status",
            vec![("ACTIVE", 1), ("INACTIVE", 2)],
        );
        let a = make_alias("meta.com/test/multi/UserId", TypeRef::I64);
        let ts = make_ts(vec![
            ("meta.com/test/multi/Person", DefinitionNode::Struct(s)),
            ("meta.com/test/multi/Status", DefinitionNode::Enum(e)),
            ("meta.com/test/multi/UserId", DefinitionNode::OpaqueAlias(a)),
        ]);
        assert_eq!(
            hex_digest(&ts),
            "8da9b088c9e75fbdf0532e1816a52c8b46109d0381e453a07b0c28fbbafd3db2",
        );
    }

    #[test]
    fn cross_language_type_id_bool() {
        let tid = type_id::TypeId::boolType(Default::default());
        assert_eq!(
            hex_digest(&tid),
            "67abdd721024f0ff4e0b3f4c2fc13bc5bad42d0b7851d456d88d203d15aaa450",
        );
    }

    #[test]
    fn cross_language_type_id_i32() {
        let tid = type_id::TypeId::i32Type(Default::default());
        assert_eq!(
            hex_digest(&tid),
            "fb5e512425fc9449316ec95969ebe71e2d576dbab833d61e2a5b9330fd70ee02",
        );
    }

    #[test]
    fn cross_language_type_id_string() {
        let tid = type_id::TypeId::stringType(Default::default());
        assert_eq!(
            hex_digest(&tid),
            "dc765660b06ee03dd16fd7ca5b957e8c805161ac2c4af28c5a100ab2ab432ca1",
        );
    }

    #[test]
    fn cross_language_type_id_uri() {
        let tid = type_id::TypeId::userDefinedType("meta.com/test/MyStruct".to_owned());
        assert_eq!(
            hex_digest(&tid),
            "7b366d852e328a4ca0cc6f000f41f3ca1c0540e08fd0a73fd6fc7458f0042b1b",
        );
    }

    #[test]
    fn cross_language_golden_values_match_serializable() {
        let s = make_struct("meta.com/test/Empty", vec![], false);
        let ts = make_ts(vec![("meta.com/test/Empty", DefinitionNode::Struct(s))]);
        assert_eq!(ts.digest(), ts.to_serializable().digest());

        let s = make_struct(
            "meta.com/test/Person",
            vec![
                make_field(1, "name", PresenceQualifier::UNQUALIFIED, TypeRef::String),
                make_field(2, "age", PresenceQualifier::OPTIONAL, TypeRef::I32),
            ],
            false,
        );
        let ts = make_ts(vec![("meta.com/test/Person", DefinitionNode::Struct(s))]);
        assert_eq!(ts.digest(), ts.to_serializable().digest());

        let e = make_enum(
            "meta.com/test/Status",
            vec![("ACTIVE", 1), ("INACTIVE", 2), ("PENDING", 3)],
        );
        let ts = make_ts(vec![("meta.com/test/Status", DefinitionNode::Enum(e))]);
        assert_eq!(ts.digest(), ts.to_serializable().digest());
    }

    // Runtime-vs-serializable equivalence

    #[test]
    fn runtime_matches_serializable_struct_with_fields() {
        let s = make_struct(
            "meta.com/test/Struct",
            vec![
                make_field(1, "field1", PresenceQualifier::UNQUALIFIED, TypeRef::I32),
                make_field(2, "field2", PresenceQualifier::OPTIONAL, TypeRef::String),
            ],
            false,
        );
        let ts = make_ts(vec![("meta.com/test/Struct", DefinitionNode::Struct(s))]);
        assert_eq!(ts.digest(), ts.to_serializable().digest());
    }

    #[test]
    fn runtime_matches_serializable_union() {
        let u = make_union(
            "meta.com/test/Union",
            vec![
                make_field(1, "opt1", PresenceQualifier::OPTIONAL, TypeRef::I32),
                make_field(2, "opt2", PresenceQualifier::OPTIONAL, TypeRef::String),
            ],
        );
        let ts = make_ts(vec![("meta.com/test/Union", DefinitionNode::Union(u))]);
        assert_eq!(ts.digest(), ts.to_serializable().digest());
    }

    #[test]
    fn runtime_matches_serializable_enum() {
        let e = make_enum("meta.com/test/Enum", vec![("A", 1), ("B", 2), ("C", 3)]);
        let ts = make_ts(vec![("meta.com/test/Enum", DefinitionNode::Enum(e))]);
        assert_eq!(ts.digest(), ts.to_serializable().digest());
    }

    #[test]
    fn runtime_matches_serializable_opaque_alias() {
        let a = make_alias("meta.com/test/Alias", TypeRef::I64);
        let ts = make_ts(vec![(
            "meta.com/test/Alias",
            DefinitionNode::OpaqueAlias(a),
        )]);
        assert_eq!(ts.digest(), ts.to_serializable().digest());
    }

    #[test]
    fn runtime_matches_serializable_sealed_struct() {
        let s = make_struct(
            "meta.com/test/SealedStruct",
            vec![make_field(
                1,
                "f",
                PresenceQualifier::UNQUALIFIED,
                TypeRef::I32,
            )],
            true,
        );
        let ts = make_ts(vec![(
            "meta.com/test/SealedStruct",
            DefinitionNode::Struct(s),
        )]);
        assert_eq!(ts.digest(), ts.to_serializable().digest());
    }

    #[test]
    fn runtime_matches_serializable_field_with_custom_default() {
        let s = make_struct(
            "meta.com/test/S",
            vec![make_field_with_default(
                1,
                "f",
                PresenceQualifier::UNQUALIFIED,
                TypeRef::I32,
                record::SerializableRecord::int32Datum(42),
            )],
            false,
        );
        let ts = make_ts(vec![("meta.com/test/S", DefinitionNode::Struct(s))]);
        assert_eq!(ts.digest(), ts.to_serializable().digest());
    }

    #[test]
    fn runtime_matches_serializable_multiple_types() {
        let s = make_struct(
            "meta.com/test/Person",
            vec![
                make_field(1, "name", PresenceQualifier::UNQUALIFIED, TypeRef::String),
                make_field(2, "age", PresenceQualifier::OPTIONAL, TypeRef::I32),
            ],
            false,
        );
        let e = make_enum(
            "meta.com/test/Status",
            vec![("ACTIVE", 1), ("INACTIVE", 2), ("PENDING", 3)],
        );
        let a = make_alias("meta.com/test/UserId", TypeRef::I64);
        let ts = make_ts(vec![
            ("meta.com/test/Person", DefinitionNode::Struct(s)),
            ("meta.com/test/Status", DefinitionNode::Enum(e)),
            ("meta.com/test/UserId", DefinitionNode::OpaqueAlias(a)),
        ]);
        assert_eq!(ts.digest(), ts.to_serializable().digest());
    }

    #[test]
    fn runtime_matches_serializable_annotated_struct() {
        let mut struct_ann = AnnotationsMap::default();
        struct_ann.insert(
            "facebook.com/thrift/annotation/Deprecated".to_owned(),
            record::SerializableRecord::boolDatum(true),
        );
        let mut field_ann = AnnotationsMap::default();
        field_ann.insert(
            "facebook.com/thrift/annotation/Doc".to_owned(),
            record::SerializableRecord::textDatum("A field".to_owned()),
        );

        let node = Arc::new(StructNode::new_shell(
            "meta.com/test/AnnotatedStruct".to_owned(),
            false,
            struct_ann,
        ));
        node.set_fields(vec![make_annotated_field(
            1,
            "annotatedField",
            PresenceQualifier::UNQUALIFIED,
            TypeRef::I32,
            field_ann,
        )])
        .unwrap();

        let ts = make_ts(vec![(
            "meta.com/test/AnnotatedStruct",
            DefinitionNode::Struct(node),
        )]);
        assert_eq!(ts.digest(), ts.to_serializable().digest());
    }

    #[test]
    fn runtime_matches_serializable_many_field_types() {
        let s = make_struct(
            "meta.com/test/AllTypes",
            vec![
                make_field(1, "f_bool", PresenceQualifier::UNQUALIFIED, TypeRef::Bool),
                make_field(2, "f_byte", PresenceQualifier::OPTIONAL, TypeRef::Byte),
                make_field(3, "f_i16", PresenceQualifier::UNQUALIFIED, TypeRef::I16),
                make_field(4, "f_i32", PresenceQualifier::OPTIONAL, TypeRef::I32),
                make_field(5, "f_i64", PresenceQualifier::UNQUALIFIED, TypeRef::I64),
                make_field(6, "f_float", PresenceQualifier::OPTIONAL, TypeRef::Float),
                make_field(
                    7,
                    "f_double",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::Double,
                ),
                make_field(8, "f_string", PresenceQualifier::OPTIONAL, TypeRef::String),
                make_field(
                    9,
                    "f_binary",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::Binary,
                ),
            ],
            false,
        );
        let ts = make_ts(vec![("meta.com/test/AllTypes", DefinitionNode::Struct(s))]);
        assert_eq!(ts.digest(), ts.to_serializable().digest());
    }

    // Order independence

    #[test]
    fn field_order_independent() {
        let ts1 = {
            let s = make_struct(
                "meta.com/S",
                vec![
                    make_field(1, "first", PresenceQualifier::UNQUALIFIED, TypeRef::I32),
                    make_field(2, "second", PresenceQualifier::UNQUALIFIED, TypeRef::I32),
                    make_field(3, "third", PresenceQualifier::UNQUALIFIED, TypeRef::I32),
                ],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        let ts2 = {
            let s = make_struct(
                "meta.com/S",
                vec![
                    make_field(3, "third", PresenceQualifier::UNQUALIFIED, TypeRef::I32),
                    make_field(1, "first", PresenceQualifier::UNQUALIFIED, TypeRef::I32),
                    make_field(2, "second", PresenceQualifier::UNQUALIFIED, TypeRef::I32),
                ],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        assert_eq!(ts1.digest(), ts2.digest());
    }

    #[test]
    fn enum_value_order_independent() {
        let ts1 = {
            let e = make_enum("meta.com/E", vec![("A", 1), ("B", 2), ("C", 3)]);
            make_ts(vec![("meta.com/E", DefinitionNode::Enum(e))])
        };
        let ts2 = {
            let e = make_enum("meta.com/E", vec![("C", 3), ("A", 1), ("B", 2)]);
            make_ts(vec![("meta.com/E", DefinitionNode::Enum(e))])
        };
        assert_eq!(ts1.digest(), ts2.digest());
    }

    #[test]
    fn uri_order_independent() {
        let ts1 = make_ts(vec![
            (
                "meta.com/A",
                DefinitionNode::Struct(make_struct("meta.com/A", vec![], false)),
            ),
            (
                "meta.com/B",
                DefinitionNode::Struct(make_struct("meta.com/B", vec![], false)),
            ),
            (
                "meta.com/C",
                DefinitionNode::Struct(make_struct("meta.com/C", vec![], false)),
            ),
        ]);
        let ts2 = make_ts(vec![
            (
                "meta.com/C",
                DefinitionNode::Struct(make_struct("meta.com/C", vec![], false)),
            ),
            (
                "meta.com/A",
                DefinitionNode::Struct(make_struct("meta.com/A", vec![], false)),
            ),
            (
                "meta.com/B",
                DefinitionNode::Struct(make_struct("meta.com/B", vec![], false)),
            ),
        ]);
        assert_eq!(ts1.digest(), ts2.digest());
    }

    #[test]
    fn annotation_order_independent() {
        let make_annotated = |ann: AnnotationsMap| {
            let node = Arc::new(StructNode::new_shell("meta.com/S".to_owned(), false, ann));
            node.set_fields(vec![]).unwrap();
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(node))])
        };

        let mut ann1 = AnnotationsMap::default();
        ann1.insert(
            "meta.com/Ann1".to_owned(),
            record::SerializableRecord::boolDatum(true),
        );
        ann1.insert(
            "meta.com/Ann2".to_owned(),
            record::SerializableRecord::boolDatum(false),
        );

        let mut ann2 = AnnotationsMap::default();
        ann2.insert(
            "meta.com/Ann2".to_owned(),
            record::SerializableRecord::boolDatum(false),
        );
        ann2.insert(
            "meta.com/Ann1".to_owned(),
            record::SerializableRecord::boolDatum(true),
        );

        assert_eq!(make_annotated(ann1).digest(), make_annotated(ann2).digest());
    }

    // Distinctness

    #[test]
    fn different_field_id_different_digest() {
        let ts1 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    1,
                    "f",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I32,
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        let ts2 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    2,
                    "f",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I32,
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        assert_ne!(ts1.digest(), ts2.digest());
    }

    #[test]
    fn different_field_name_different_digest() {
        let ts1 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    1,
                    "alpha",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I32,
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        let ts2 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    1,
                    "beta",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I32,
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        assert_ne!(ts1.digest(), ts2.digest());
    }

    #[test]
    fn different_field_type_different_digest() {
        let ts1 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    1,
                    "f",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I32,
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        let ts2 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    1,
                    "f",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I64,
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        assert_ne!(ts1.digest(), ts2.digest());
    }

    #[test]
    fn different_presence_different_digest() {
        let ts1 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    1,
                    "f",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I32,
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        let ts2 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    1,
                    "f",
                    PresenceQualifier::OPTIONAL,
                    TypeRef::I32,
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        assert_ne!(ts1.digest(), ts2.digest());
    }

    #[test]
    fn struct_vs_union_different_digest() {
        let ts1 = {
            let s = make_struct(
                "meta.com/T",
                vec![make_field(
                    1,
                    "f",
                    PresenceQualifier::OPTIONAL,
                    TypeRef::I32,
                )],
                false,
            );
            make_ts(vec![("meta.com/T", DefinitionNode::Struct(s))])
        };
        let ts2 = {
            let u = make_union(
                "meta.com/T",
                vec![make_field(
                    1,
                    "f",
                    PresenceQualifier::OPTIONAL,
                    TypeRef::I32,
                )],
            );
            make_ts(vec![("meta.com/T", DefinitionNode::Union(u))])
        };
        assert_ne!(ts1.digest(), ts2.digest());
    }

    #[test]
    fn sealed_vs_unsealed_different_digest() {
        let ts1 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    1,
                    "f",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I32,
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        let ts2 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    1,
                    "f",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I32,
                )],
                true,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        assert_ne!(ts1.digest(), ts2.digest());
    }

    #[test]
    fn different_uri_different_digest() {
        let ts1 = {
            let s = make_struct("meta.com/Foo", vec![], false);
            make_ts(vec![("meta.com/Foo", DefinitionNode::Struct(s))])
        };
        let ts2 = {
            let s = make_struct("meta.com/Bar", vec![], false);
            make_ts(vec![("meta.com/Bar", DefinitionNode::Struct(s))])
        };
        assert_ne!(ts1.digest(), ts2.digest());
    }

    #[test]
    fn with_vs_without_custom_default_different_digest() {
        let ts1 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field_with_default(
                    1,
                    "f",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I32,
                    record::SerializableRecord::int32Datum(0),
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        let ts2 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    1,
                    "f",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I32,
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        assert_ne!(ts1.digest(), ts2.digest());
    }

    #[test]
    fn deterministic() {
        let make = || {
            let s = make_struct(
                "meta.com/S",
                vec![
                    make_field(1, "a", PresenceQualifier::UNQUALIFIED, TypeRef::I32),
                    make_field(2, "b", PresenceQualifier::OPTIONAL, TypeRef::String),
                ],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        assert_eq!(make().digest(), make().digest());
    }

    // DigestMode::Structural

    #[test]
    fn structural_mode_ignores_annotations_and_defaults() {
        use type_system_digest::DigestMode;

        let mut struct_ann = AnnotationsMap::default();
        struct_ann.insert(
            "meta.com/Deprecated".to_owned(),
            record::SerializableRecord::boolDatum(true),
        );
        let mut field_ann = AnnotationsMap::default();
        field_ann.insert(
            "meta.com/Doc".to_owned(),
            record::SerializableRecord::textDatum("docs".to_owned()),
        );

        let annotated_node = Arc::new(StructNode::new_shell(
            "meta.com/S".to_owned(),
            false,
            struct_ann,
        ));
        annotated_node
            .set_fields(vec![FieldDefinition::new(
                FieldIdentity {
                    id: 1,
                    name: "f".to_owned(),
                },
                PresenceQualifier::UNQUALIFIED,
                TypeRef::I32,
                Some(record::SerializableRecord::int32Datum(42)),
                field_ann,
            )])
            .unwrap();
        let ts1 = make_ts(vec![("meta.com/S", DefinitionNode::Struct(annotated_node))]);

        let plain_node = make_struct(
            "meta.com/S",
            vec![make_field(
                1,
                "f",
                PresenceQualifier::UNQUALIFIED,
                TypeRef::I32,
            )],
            false,
        );
        let ts2 = make_ts(vec![("meta.com/S", DefinitionNode::Struct(plain_node))]);

        assert_ne!(
            ts1.digest_with_mode(DigestMode::Full),
            ts2.digest_with_mode(DigestMode::Full),
        );
        assert_eq!(
            ts1.digest_with_mode(DigestMode::Structural),
            ts2.digest_with_mode(DigestMode::Structural),
        );
    }

    #[test]
    fn structural_mode_still_distinguishes_structure() {
        use type_system_digest::DigestMode;

        let ts1 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    1,
                    "f",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I32,
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        let ts2 = {
            let s = make_struct(
                "meta.com/S",
                vec![make_field(
                    1,
                    "f",
                    PresenceQualifier::UNQUALIFIED,
                    TypeRef::I64,
                )],
                false,
            );
            make_ts(vec![("meta.com/S", DefinitionNode::Struct(s))])
        };
        assert_ne!(
            ts1.digest_with_mode(DigestMode::Structural),
            ts2.digest_with_mode(DigestMode::Structural),
        );
    }

    #[test]
    fn structural_mode_runtime_matches_serializable() {
        use type_system_digest::DigestMode;

        let mut ann = AnnotationsMap::default();
        ann.insert(
            "meta.com/Ann".to_owned(),
            record::SerializableRecord::boolDatum(true),
        );
        let node = Arc::new(StructNode::new_shell("meta.com/S".to_owned(), false, ann));
        node.set_fields(vec![make_field_with_default(
            1,
            "f",
            PresenceQualifier::UNQUALIFIED,
            TypeRef::I32,
            record::SerializableRecord::int32Datum(7),
        )])
        .unwrap();
        let ts = make_ts(vec![("meta.com/S", DefinitionNode::Struct(node))]);
        let sts = ts.to_serializable();

        assert_eq!(
            ts.digest_with_mode(DigestMode::Structural),
            sts.digest_with_mode(DigestMode::Structural),
        );
    }
}
