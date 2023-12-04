// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Module containing conversion methods between the Rust Facts and
//! Rust/C++ shared Facts (in the ffi module)

use serde::Deserializer;
use serde::Serialize;
use serde::Serializer;

use crate::ffi;

impl From<facts::Facts> for ffi::FileSymbols {
    fn from(facts: facts::Facts) -> Self {
        Self {
            types: facts.types.into_keys().collect(),
            functions: facts.functions,
            constants: facts.constants,
            modules: facts.modules.into_keys().collect(),
        }
    }
}

impl From<facts::TypeKind> for ffi::TypeKind {
    fn from(kind: facts::TypeKind) -> Self {
        match kind {
            facts::TypeKind::Class => Self::Class,
            facts::TypeKind::Interface => Self::Interface,
            facts::TypeKind::Enum => Self::Enum,
            facts::TypeKind::Trait => Self::Trait,
            facts::TypeKind::TypeAlias => Self::TypeAlias,
            facts::TypeKind::Unknown => Self::Unknown,
            facts::TypeKind::Mixed => Self::Mixed,
        }
    }
}

impl ffi::FileFacts {
    pub(crate) fn from_facts(facts: facts::Facts, sha1sum: String) -> Self {
        Self {
            sha1sum,
            types: facts
                .types
                .into_iter()
                .map(|(name, tf)| ffi::TypeFacts {
                    name,
                    base_types: tf.base_types.into_iter().collect(),
                    kind: tf.kind.into(),
                    attributes: (tf.attributes.into_iter())
                        .map(ffi::AttrFacts::from)
                        .collect(),
                    flags: tf.flags,
                    require_extends: tf.require_extends.into_iter().collect(),
                    require_implements: tf.require_implements.into_iter().collect(),
                    require_class: tf.require_class.into_iter().collect(),
                    methods: (tf.methods.into_iter())
                        .map(|(name, mf)| ffi::MethodFacts {
                            name,
                            details: ffi::MethodDetails {
                                attributes: (mf.attributes.into_iter())
                                    .map(ffi::AttrFacts::from)
                                    .collect(),
                            },
                        })
                        .collect(),
                })
                .collect(),
            functions: facts.functions,
            constants: facts.constants,
            file_attributes: (facts.file_attributes.into_iter())
                .map(ffi::AttrFacts::from)
                .collect(),
            modules: (facts.modules.into_iter())
                .map(|(name, facts::ModuleFacts {})| ffi::ModuleFacts { name })
                .collect(),
        }
    }
}

impl From<(String, Vec<serde_json::Value>)> for ffi::AttrFacts {
    fn from((name, args): (String, Vec<serde_json::Value>)) -> Self {
        Self {
            name,
            args: (args.into_iter())
                .map(|v| match v {
                    serde_json::Value::String(s) => s,
                    x => unimplemented!("{x}: unsupported type"),
                })
                .collect(),
        }
    }
}

pub(crate) fn attrs_to_json<S: Serializer>(
    attrs: &Vec<ffi::AttrFacts>,
    s: S,
) -> Result<S::Ok, S::Error> {
    use serde::ser::SerializeMap;
    let mut map = s.serialize_map(Some(attrs.len()))?;
    for ffi::AttrFacts { name, args } in attrs.iter() {
        map.serialize_entry(name, args)?;
    }
    map.end()
}

pub(crate) fn json_to_attrs<'de, D: Deserializer<'de>>(
    d: D,
) -> Result<Vec<ffi::AttrFacts>, D::Error> {
    use serde::de::MapAccess;
    use serde::de::Visitor;
    struct AttrFactsMapVisitor;
    impl<'de> Visitor<'de> for AttrFactsMapVisitor {
        type Value = Vec<ffi::AttrFacts>;
        fn expecting(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            write!(f, "a map of AttrFacts")
        }
        fn visit_map<M: MapAccess<'de>>(self, mut access: M) -> Result<Self::Value, M::Error> {
            let mut attrs = Vec::with_capacity(access.size_hint().unwrap_or(0));
            while let Some((name, args)) = access.next_entry()? {
                attrs.push(ffi::AttrFacts { name, args });
            }
            Ok(attrs)
        }
    }
    d.deserialize_map(AttrFactsMapVisitor)
}

pub(crate) fn methods_to_json<S: Serializer>(
    methods: &Vec<ffi::MethodFacts>,
    s: S,
) -> Result<S::Ok, S::Error> {
    use serde::ser::SerializeMap;
    let mut map = s.serialize_map(Some(methods.len()))?;
    for ffi::MethodFacts { name, details } in methods.iter() {
        map.serialize_entry(name, details)?;
    }
    map.end()
}

pub(crate) fn json_to_methods<'de, D: Deserializer<'de>>(
    d: D,
) -> Result<Vec<ffi::MethodFacts>, D::Error> {
    use serde::de::MapAccess;
    use serde::de::Visitor;
    struct MethodFactsMapVisitor;
    impl<'de> Visitor<'de> for MethodFactsMapVisitor {
        type Value = Vec<ffi::MethodFacts>;
        fn expecting(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            write!(f, "a map of MethodFacts")
        }
        fn visit_map<M: MapAccess<'de>>(self, mut access: M) -> Result<Self::Value, M::Error> {
            let mut methods = Vec::with_capacity(access.size_hint().unwrap_or(0));
            while let Some((name, details)) = access.next_entry()? {
                methods.push(ffi::MethodFacts { name, details });
            }
            Ok(methods)
        }
    }
    d.deserialize_map(MethodFactsMapVisitor)
}

// Cannot derive this because cxx enums are really structs with a repr field.
impl Serialize for ffi::TypeKind {
    fn serialize<S: Serializer>(&self, s: S) -> Result<S::Ok, S::Error> {
        use serde::ser::Error;
        s.serialize_str(match *self {
            ffi::TypeKind::Class => "class",
            ffi::TypeKind::Interface => "interface",
            ffi::TypeKind::Trait => "trait",
            ffi::TypeKind::Enum => "enum",
            ffi::TypeKind::TypeAlias => "typeAlias",
            ffi::TypeKind::Unknown => "unknown",
            ffi::TypeKind::Mixed => "mixed",
            _ => return Err(S::Error::custom(format!("Unknown kind {self:?}"))),
        })
    }
}

impl Default for ffi::TypeKind {
    fn default() -> Self {
        Self::Unknown
    }
}

#[cfg(test)]
mod tests {
    use std::collections::BTreeMap;
    use std::collections::BTreeSet;

    use super::*;

    #[test]
    fn test_facts_symbols() {
        let (ffi_type_facts_by_name, rust_type_facts_by_name) = create_type_facts_by_name();
        let (ffi_module_facts_by_name, rust_module_facts_by_name) = create_module_facts_by_name();
        let ffi_symbols = ffi::FileSymbols {
            types: ffi_type_facts_by_name,
            functions: vec!["f1".to_string(), "f2".to_string()],
            constants: vec!["C".to_string()],
            modules: ffi_module_facts_by_name,
        };
        let rust_facts = facts::Facts {
            types: rust_type_facts_by_name,
            functions: vec!["f1".to_string(), "f2".to_string()],
            constants: vec!["C".to_string()],
            modules: rust_module_facts_by_name,
            file_attributes: Default::default(),
        };
        assert_eq!(ffi::FileSymbols::from(rust_facts), ffi_symbols)
    }

    fn create_type_facts_by_name() -> (Vec<String>, facts::TypeFactsByName) {
        let rust_type_facts = facts::TypeFacts {
            kind: facts::TypeKind::Class,
            ..Default::default()
        };
        let ffi_type_facts_by_name = vec!["C".into()];
        let mut rust_type_facts_by_name = BTreeMap::new();
        rust_type_facts_by_name.insert("C".into(), rust_type_facts);
        (ffi_type_facts_by_name, rust_type_facts_by_name)
    }

    fn create_module_facts_by_name() -> (Vec<String>, facts::ModuleFactsByName) {
        let ffi_module_facts_by_name = vec!["mfoo".into()];
        let mut rust_module_facts_by_name = BTreeMap::new();
        rust_module_facts_by_name.insert("mfoo".into(), facts::ModuleFacts {});
        (ffi_module_facts_by_name, rust_module_facts_by_name)
    }

    fn same_json(facts: facts::Facts) {
        assert_eq!(
            facts.to_json(false, "abc"),
            serde_json::to_string(&ffi::FileFacts::from_facts(facts, "abc".into())).unwrap()
        );
    }

    #[test]
    fn test_conversion() {
        same_json(facts::Facts {
            types: BTreeMap::from_iter(vec![(
                "t".into(),
                facts::TypeFacts {
                    attributes: BTreeMap::from_iter(vec![(
                        "A2".into(),
                        vec![
                            serde_json::Value::String("x".into()),
                            serde_json::Value::String(r#""quoted""#.into()),
                        ],
                    )]),
                    base_types: BTreeSet::from_iter(vec!["B".into()]),
                    require_extends: BTreeSet::from_iter(vec!["E".into()]),
                    methods: BTreeMap::from_iter(vec![(
                        "g".into(),
                        facts::MethodFacts {
                            attributes: BTreeMap::from_iter(vec![(
                                "A3".into(),
                                vec![
                                    serde_json::Value::String("x".into()),
                                    serde_json::Value::String(r#""quoted""#.into()),
                                ],
                            )]),
                        },
                    )]),

                    ..Default::default()
                },
            )]),
            constants: vec!["C".into()],
            functions: vec!["f".into()],
            file_attributes: BTreeMap::from_iter(vec![(
                "A1".into(),
                vec![
                    serde_json::Value::String("x".into()),
                    serde_json::Value::String(r#""quoted""#.into()),
                ],
            )]),
            modules: BTreeMap::from_iter(vec![("m".into(), facts::ModuleFacts {})]),
            ..Default::default()
        })
    }
}
