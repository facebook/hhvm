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

//! Tests for the `TypeSystem` builder and runtime, focused on recursive and
//! mutually recursive types (the motivation for the `Weak`-edge design) and the
//! validation rules carried over from the C++ reference.

use std::sync::Arc;

use type_id::TypeId;

use crate::BasicTypeSystem;
use crate::InvalidTypeError;
use crate::TypeSystem;
use crate::TypeSystemBuilder;
use crate::structured_node::StructuredNode;
use crate::type_ref::DefinitionRef;

fn uri_type(uri: &str) -> TypeId {
    TypeId::userDefinedType(uri.to_owned())
}

fn list_type(element: TypeId) -> TypeId {
    TypeId::listType(type_id::ListTypeId {
        elementType: Some(Box::new(element)),
        ..Default::default()
    })
}

fn field(
    id: i16,
    name: &str,
    presence: type_system::PresenceQualifier,
    ty: TypeId,
) -> type_system::SerializableFieldDefinition {
    type_system::SerializableFieldDefinition {
        identity: type_system::FieldIdentity {
            id,
            name: name.to_owned(),
            ..Default::default()
        },
        presence,
        r#type: ty,
        ..Default::default()
    }
}

fn optional_field(id: i16, name: &str, ty: TypeId) -> type_system::SerializableFieldDefinition {
    field(id, name, type_system::PresenceQualifier::OPTIONAL, ty)
}

fn struct_def(
    fields: Vec<type_system::SerializableFieldDefinition>,
) -> type_system::SerializableTypeDefinition {
    type_system::SerializableTypeDefinition::structDef(type_system::SerializableStructDefinition {
        fields,
        isSealed: false,
        ..Default::default()
    })
}

fn union_def(
    fields: Vec<type_system::SerializableFieldDefinition>,
) -> type_system::SerializableTypeDefinition {
    type_system::SerializableTypeDefinition::unionDef(type_system::SerializableUnionDefinition {
        fields,
        isSealed: false,
        ..Default::default()
    })
}

fn enum_def(values: &[(&str, i32)]) -> type_system::SerializableTypeDefinition {
    type_system::SerializableTypeDefinition::enumDef(type_system::SerializableEnumDefinition {
        values: values
            .iter()
            .map(
                |(name, datum)| type_system::SerializableEnumValueDefinition {
                    name: (*name).to_owned(),
                    datum: *datum,
                    ..Default::default()
                },
            )
            .collect(),
        ..Default::default()
    })
}

fn opaque_alias_def(target: TypeId) -> type_system::SerializableTypeDefinition {
    type_system::SerializableTypeDefinition::opaqueAliasDef(
        type_system::SerializableOpaqueAliasDefinition {
            targetType: target,
            ..Default::default()
        },
    )
}

fn build(
    defs: Vec<(&str, type_system::SerializableTypeDefinition)>,
) -> Result<BasicTypeSystem, InvalidTypeError> {
    let mut builder = TypeSystemBuilder::new();
    for (uri, definition) in defs {
        builder.add_entry(
            uri.to_owned(),
            type_system::SerializableTypeDefinitionEntry {
                definition,
                ..Default::default()
            },
        )?;
    }
    builder.build()
}

/// Downgrades the node behind a `DefinitionRef` to a `Weak` for leak testing.
fn weak_struct(def: &DefinitionRef) -> std::sync::Weak<crate::StructNode> {
    match def {
        DefinitionRef::Struct(s) => Arc::downgrade(s),
        _ => panic!("expected a struct definition"),
    }
}

#[test]
fn recursive_struct_navigates_back_to_itself() {
    // struct Tree { 1: optional list<Tree> children }
    let ts = build(vec![(
        "x/Tree",
        struct_def(vec![optional_field(
            1,
            "children",
            list_type(uri_type("x/Tree")),
        )]),
    )])
    .expect("recursive Tree should build");

    let tree = ts.get("x/Tree").expect("Tree is defined");
    let children = tree
        .as_struct()
        .unwrap()
        .field_by_name("children")
        .expect("children field exists");
    let element = children
        .type_ref()
        .as_list()
        .unwrap()
        .element_type()
        .as_struct()
        .expect("list element is a struct");

    assert_eq!(element.uri(), "x/Tree", "the element type is Tree itself");

    let DefinitionRef::Struct(tree_node) = &tree else {
        panic!("Tree is a struct");
    };
    assert!(
        Arc::ptr_eq(&element, tree_node),
        "list<Tree> element resolves to the same Tree node (identity)"
    );
}

#[test]
fn mutually_recursive_structs_resolve() {
    // struct A { 1: optional B b }   struct B { 1: optional A a }
    let ts = build(vec![
        (
            "x/A",
            struct_def(vec![optional_field(1, "b", uri_type("x/B"))]),
        ),
        (
            "x/B",
            struct_def(vec![optional_field(1, "a", uri_type("x/A"))]),
        ),
    ])
    .expect("mutually recursive A/B should build");

    let a = ts.get("x/A").unwrap();
    let b = a
        .as_struct()
        .unwrap()
        .field_by_name("b")
        .unwrap()
        .type_ref()
        .as_struct()
        .unwrap();
    let a_again = b
        .field_by_name("a")
        .unwrap()
        .type_ref()
        .as_struct()
        .unwrap();

    let DefinitionRef::Struct(a_node) = &a else {
        panic!("A is a struct");
    };
    assert!(
        Arc::ptr_eq(&a_again, a_node),
        "A -> b -> B -> a returns to the same A node"
    );
}

#[test]
fn recursive_type_is_freed_when_type_system_is_dropped() {
    // The decisive leak-proof: a recursive type must not keep itself alive via
    // its own field edge once the TypeSystem (the sole strong owner) is dropped.
    let ts = build(vec![(
        "x/Tree",
        struct_def(vec![optional_field(
            1,
            "children",
            list_type(uri_type("x/Tree")),
        )]),
    )])
    .unwrap();

    let tree = ts.get("x/Tree").unwrap();
    let weak = weak_struct(&tree);
    // Exactly two strong owners — the TypeSystem's storage and this held
    // `DefinitionRef` handle. The recursive field edge is Weak, so it adds none.
    assert_eq!(
        weak.strong_count(),
        2,
        "only the TypeSystem storage and the held `tree` handle own the node"
    );

    drop(tree); // drop the held DefinitionRef handle
    drop(ts); // drop the TypeSystem (the remaining strong owner)

    assert!(
        weak.upgrade().is_none(),
        "recursive Tree leaked: still strong-reachable after its TypeSystem was dropped"
    );
}

#[test]
fn union_field_must_be_optional() {
    let result = build(vec![(
        "x/U",
        union_def(vec![field(
            1,
            "v",
            type_system::PresenceQualifier::UNQUALIFIED,
            TypeId::i32Type(Default::default()),
        )]),
    )]);
    assert!(
        matches!(result, Err(InvalidTypeError::NonOptionalUnionField(1, _))),
        "a non-optional union field should be rejected, got {result:?}"
    );
}

#[test]
fn duplicate_field_id_is_rejected() {
    let result = build(vec![(
        "x/S",
        struct_def(vec![
            optional_field(1, "a", TypeId::i32Type(Default::default())),
            optional_field(1, "b", TypeId::i32Type(Default::default())),
        ]),
    )]);
    assert!(
        matches!(result, Err(InvalidTypeError::DuplicateFieldId(1, _))),
        "duplicate field id should be rejected, got {result:?}"
    );
}

#[test]
fn duplicate_enum_value_is_rejected() {
    let result = build(vec![("x/E", enum_def(&[("A", 1), ("B", 1)]))]);
    assert!(
        matches!(result, Err(InvalidTypeError::DuplicateEnumValue(1, _))),
        "duplicate enum value should be rejected, got {result:?}"
    );
}

#[test]
fn opaque_alias_rejects_user_defined_target() {
    // An opaque alias whose target is a user-defined type must be rejected.
    let result = build(vec![
        ("x/S", struct_def(vec![])),
        ("x/Alias", opaque_alias_def(uri_type("x/S"))),
    ]);
    assert!(
        matches!(result, Err(InvalidTypeError::InvalidOpaqueAlias(_))),
        "opaque alias targeting a user-defined type should be rejected, got {result:?}"
    );
}

#[test]
fn opaque_alias_allows_primitive_target() {
    let ts = build(vec![(
        "x/Id",
        opaque_alias_def(TypeId::i64Type(Default::default())),
    )])
    .expect("opaque alias over a primitive should build");
    let alias = ts.get("x/Id").unwrap();
    assert!(alias.as_opaque_alias().unwrap().target_type().is_i64());
}

#[test]
fn unknown_reference_is_rejected() {
    let result = build(vec![(
        "x/S",
        struct_def(vec![optional_field(
            1,
            "missing",
            uri_type("x/DoesNotExist"),
        )]),
    )]);
    assert!(
        matches!(result, Err(InvalidTypeError::UnknownUri(_))),
        "reference to an undefined type should be rejected, got {result:?}"
    );
}

#[test]
fn duplicate_uri_is_rejected() {
    let mut builder = TypeSystemBuilder::new();
    builder
        .add_entry(
            "x/S".to_owned(),
            type_system::SerializableTypeDefinitionEntry {
                definition: struct_def(vec![]),
                ..Default::default()
            },
        )
        .unwrap();
    let dup = builder.add_entry(
        "x/S".to_owned(),
        type_system::SerializableTypeDefinitionEntry {
            definition: struct_def(vec![]),
            ..Default::default()
        },
    );
    assert!(matches!(dup, Err(InvalidTypeError::DuplicateUri(_))));
}

#[test]
fn overlay_field_resolves_to_base_node_identity() {
    let base = build(vec![("x/Base", struct_def(vec![]))]).unwrap();

    let mut overlay_builder = TypeSystemBuilder::new();
    overlay_builder
        .add_entry(
            "x/Holder".to_owned(),
            type_system::SerializableTypeDefinitionEntry {
                definition: struct_def(vec![optional_field(1, "base", uri_type("x/Base"))]),
                ..Default::default()
            },
        )
        .unwrap();
    let overlay = overlay_builder
        .build_layered_on(base)
        .expect("layering should build");

    let holder = overlay.get("x/Holder").unwrap();
    let base_via_field = holder
        .as_struct()
        .unwrap()
        .field_by_name("base")
        .unwrap()
        .type_ref()
        .as_struct()
        .unwrap();
    let base_direct = overlay.get("x/Base").unwrap();
    let DefinitionRef::Struct(base_node) = &base_direct else {
        panic!("Base is a struct");
    };
    assert!(
        Arc::ptr_eq(&base_via_field, base_node),
        "overlay field and base lookup resolve to the same node"
    );
}

#[test]
fn overlay_uri_conflicting_with_base_is_rejected() {
    let base = build(vec![("x/Base", struct_def(vec![]))]).unwrap();

    let mut overlay_builder = TypeSystemBuilder::new();
    overlay_builder
        .add_entry(
            "x/Base".to_owned(),
            type_system::SerializableTypeDefinitionEntry {
                definition: struct_def(vec![]),
                ..Default::default()
            },
        )
        .unwrap();

    let result = overlay_builder.build_layered_on(base);
    assert!(
        matches!(result, Err(InvalidTypeError::DuplicateUri(_))),
        "overlaying a URI already present in the base should be rejected, got {result:?}"
    );
}

fn build_serializable(
    defs: Vec<(&str, type_system::SerializableTypeDefinition)>,
) -> type_system::SerializableTypeSystem {
    let types = defs
        .into_iter()
        .map(|(uri, def)| {
            (
                uri.to_owned(),
                type_system::SerializableTypeDefinitionEntry {
                    definition: def,
                    ..Default::default()
                },
            )
        })
        .collect();
    type_system::SerializableTypeSystem {
        types,
        ..Default::default()
    }
}

#[test]
fn serialize_roundtrip_struct_with_fields() {
    let original = build_serializable(vec![(
        "x/Person",
        struct_def(vec![
            field(
                1,
                "name",
                type_system::PresenceQualifier::UNQUALIFIED,
                TypeId::stringType(Default::default()),
            ),
            optional_field(2, "age", TypeId::i32Type(Default::default())),
        ]),
    )]);
    let live = TypeSystemBuilder::from_serializable(original.clone())
        .unwrap()
        .build()
        .unwrap();
    let reserialized = live.to_serializable();
    assert_eq!(reserialized.types.len(), 1);
    let entry = &reserialized.types["x/Person"];
    match &entry.definition {
        type_system::SerializableTypeDefinition::structDef(s) => {
            assert_eq!(s.fields.len(), 2);
            assert_eq!(s.fields[0].identity.name, "name");
            assert!(matches!(s.fields[0].r#type, TypeId::stringType(_)));
            assert_eq!(s.fields[1].identity.id, 2);
            assert!(matches!(s.fields[1].r#type, TypeId::i32Type(_)));
        }
        _ => panic!("expected structDef"),
    }
}

#[test]
fn serialize_roundtrip_cross_referencing_types() {
    let original = build_serializable(vec![
        (
            "x/A",
            struct_def(vec![optional_field(1, "b", uri_type("x/B"))]),
        ),
        (
            "x/B",
            struct_def(vec![optional_field(
                1,
                "value",
                TypeId::i32Type(Default::default()),
            )]),
        ),
    ]);
    let live = TypeSystemBuilder::from_serializable(original.clone())
        .unwrap()
        .build()
        .unwrap();
    let reserialized = live.to_serializable();
    assert_eq!(reserialized.types.len(), 2);
    let a_entry = &reserialized.types["x/A"];
    match &a_entry.definition {
        type_system::SerializableTypeDefinition::structDef(s) => {
            assert!(matches!(
                &s.fields[0].r#type,
                TypeId::userDefinedType(uri) if uri == "x/B"
            ));
        }
        _ => panic!("expected structDef"),
    }
}

#[test]
fn serialize_roundtrip_enum() {
    let original = build_serializable(vec![("x/Color", enum_def(&[("RED", 0), ("GREEN", 1)]))]);
    let live = TypeSystemBuilder::from_serializable(original.clone())
        .unwrap()
        .build()
        .unwrap();
    let reserialized = live.to_serializable();
    let entry = &reserialized.types["x/Color"];
    match &entry.definition {
        type_system::SerializableTypeDefinition::enumDef(e) => {
            assert_eq!(e.values.len(), 2);
            assert_eq!(e.values[0].name, "RED");
            assert_eq!(e.values[1].datum, 1);
        }
        _ => panic!("expected enumDef"),
    }
}

#[test]
fn serialize_roundtrip_opaque_alias() {
    let original = build_serializable(vec![(
        "x/UserId",
        opaque_alias_def(TypeId::i64Type(Default::default())),
    )]);
    let live = TypeSystemBuilder::from_serializable(original.clone())
        .unwrap()
        .build()
        .unwrap();
    let reserialized = live.to_serializable();
    let entry = &reserialized.types["x/UserId"];
    match &entry.definition {
        type_system::SerializableTypeDefinition::opaqueAliasDef(o) => {
            assert!(matches!(o.targetType, TypeId::i64Type(_)));
        }
        _ => panic!("expected opaqueAliasDef"),
    }
}

#[test]
fn serialize_roundtrip_union() {
    let original = build_serializable(vec![(
        "x/U",
        union_def(vec![
            optional_field(1, "s", TypeId::stringType(Default::default())),
            optional_field(2, "n", TypeId::i32Type(Default::default())),
        ]),
    )]);
    let live = TypeSystemBuilder::from_serializable(original.clone())
        .unwrap()
        .build()
        .unwrap();
    let reserialized = live.to_serializable();
    let entry = &reserialized.types["x/U"];
    match &entry.definition {
        type_system::SerializableTypeDefinition::unionDef(u) => {
            assert_eq!(u.fields.len(), 2);
        }
        _ => panic!("expected unionDef"),
    }
}

#[test]
fn serialize_roundtrip_container_types() {
    let original = build_serializable(vec![(
        "x/S",
        struct_def(vec![
            optional_field(1, "items", list_type(TypeId::i32Type(Default::default()))),
            optional_field(
                2,
                "tags",
                TypeId::setType(type_id::SetTypeId {
                    elementType: Some(Box::new(TypeId::stringType(Default::default()))),
                    ..Default::default()
                }),
            ),
            optional_field(
                3,
                "props",
                TypeId::mapType(type_id::MapTypeId {
                    keyType: Some(Box::new(TypeId::stringType(Default::default()))),
                    valueType: Some(Box::new(TypeId::i32Type(Default::default()))),
                    ..Default::default()
                }),
            ),
        ]),
    )]);
    let live = TypeSystemBuilder::from_serializable(original.clone())
        .unwrap()
        .build()
        .unwrap();
    let reserialized = live.to_serializable();
    let entry = &reserialized.types["x/S"];
    match &entry.definition {
        type_system::SerializableTypeDefinition::structDef(s) => {
            assert!(matches!(s.fields[0].r#type, TypeId::listType(_)));
            assert!(matches!(s.fields[1].r#type, TypeId::setType(_)));
            assert!(matches!(s.fields[2].r#type, TypeId::mapType(_)));
        }
        _ => panic!("expected structDef"),
    }
}

#[test]
fn digest_live_equals_digest_serializable() {
    use type_system_digest::TypeSystemDigest;

    let serializable = build_serializable(vec![
        (
            "x/A",
            struct_def(vec![optional_field(1, "b", uri_type("x/B"))]),
        ),
        (
            "x/B",
            struct_def(vec![optional_field(
                1,
                "value",
                TypeId::i32Type(Default::default()),
            )]),
        ),
        ("x/Color", enum_def(&[("RED", 0), ("GREEN", 1)])),
        (
            "x/UserId",
            opaque_alias_def(TypeId::i64Type(Default::default())),
        ),
    ]);
    let live = TypeSystemBuilder::from_serializable(serializable.clone())
        .unwrap()
        .build()
        .unwrap();

    assert_eq!(live.digest(), serializable.digest());
}

#[test]
fn get_or_err_returns_error_for_unknown_uri() {
    let ts = build(vec![("x/A", struct_def(vec![]))]).unwrap();
    assert!(ts.get_or_err("x/A").is_ok());
    assert!(matches!(
        ts.get_or_err("x/Missing"),
        Err(InvalidTypeError::UnknownUri(_))
    ));
}

/// Builds a `SerializableTypeDefinitionEntry` carrying source info.
fn entry_with_source(
    def: type_system::SerializableTypeDefinition,
    locator: &str,
    name: &str,
) -> type_system::SerializableTypeDefinitionEntry {
    type_system::SerializableTypeDefinitionEntry {
        definition: def,
        sourceInfo: Some(type_system::SerializableThriftSourceInfo {
            locator: locator.to_owned(),
            name: name.to_owned(),
            ..Default::default()
        }),
        ..Default::default()
    }
}

#[test]
fn layered_to_serializable_preserves_source_info() {
    let mut base_builder = TypeSystemBuilder::new();
    base_builder
        .add_entry(
            "x/Base".to_owned(),
            entry_with_source(struct_def(vec![]), "base.thrift", "Base"),
        )
        .unwrap();
    let base = base_builder.build().unwrap();

    let mut overlay_builder = TypeSystemBuilder::new();
    overlay_builder
        .add_entry(
            "x/Overlay".to_owned(),
            entry_with_source(struct_def(vec![]), "overlay.thrift", "Overlay"),
        )
        .unwrap();
    let overlay = overlay_builder
        .build_layered_on(base)
        .expect("layering should build");

    let reserialized = overlay.to_serializable();
    assert_eq!(
        reserialized.types["x/Overlay"]
            .sourceInfo
            .as_ref()
            .map(|s| s.name.clone()),
        Some("Overlay".to_owned()),
        "overlay source info should survive serialization"
    );
    assert_eq!(
        reserialized.types["x/Base"]
            .sourceInfo
            .as_ref()
            .map(|s| s.name.clone()),
        Some("Base".to_owned()),
        "base source info should survive serialization"
    );
}

/// Builds a `SerializableTypeSystem` whose entries carry source info.
fn serializable_with_sources(
    defs: Vec<(&str, type_system::SerializableTypeDefinition, &str, &str)>,
) -> type_system::SerializableTypeSystem {
    let types = defs
        .into_iter()
        .map(|(uri, def, locator, name)| (uri.to_owned(), entry_with_source(def, locator, name)))
        .collect();
    type_system::SerializableTypeSystem {
        types,
        ..Default::default()
    }
}

#[test]
fn add_entry_rejects_duplicate_source_identifier() {
    let mut builder = TypeSystemBuilder::new();
    builder
        .add_entry(
            "x/A".to_owned(),
            entry_with_source(struct_def(vec![]), "shared.thrift", "Shared"),
        )
        .unwrap();
    let dup = builder.add_entry(
        "x/B".to_owned(),
        entry_with_source(struct_def(vec![]), "shared.thrift", "Shared"),
    );
    assert!(
        matches!(dup, Err(InvalidTypeError::DuplicateSourceIdentifier(_, _))),
        "two URIs sharing a source identifier should be rejected, got {dup:?}"
    );
}

#[test]
fn from_serializable_rejects_duplicate_source_identifier() {
    // Two distinct URIs sharing one source identifier must be rejected the same
    // way `add_entry` rejects them; the invariant cannot depend on constructor.
    let serializable = serializable_with_sources(vec![
        ("x/A", struct_def(vec![]), "shared.thrift", "Shared"),
        ("x/B", struct_def(vec![]), "shared.thrift", "Shared"),
    ]);
    let result = TypeSystemBuilder::from_serializable(serializable);
    assert!(
        matches!(
            result,
            Err(InvalidTypeError::DuplicateSourceIdentifier(_, _))
        ),
        "from_serializable should reject duplicate source identifiers"
    );
}

#[test]
fn from_serializable_preserves_source_info() {
    let original = serializable_with_sources(vec![
        ("x/A", struct_def(vec![]), "a.thrift", "A"),
        ("x/B", struct_def(vec![]), "b.thrift", "B"),
    ]);
    let live = TypeSystemBuilder::from_serializable(original)
        .unwrap()
        .build()
        .unwrap();
    let reserialized = live.to_serializable();
    assert_eq!(
        reserialized.types["x/A"]
            .sourceInfo
            .as_ref()
            .map(|s| (s.locator.clone(), s.name.clone())),
        Some(("a.thrift".to_owned(), "A".to_owned())),
    );
    assert_eq!(
        reserialized.types["x/B"]
            .sourceInfo
            .as_ref()
            .map(|s| (s.locator.clone(), s.name.clone())),
        Some(("b.thrift".to_owned(), "B".to_owned())),
    );
}
