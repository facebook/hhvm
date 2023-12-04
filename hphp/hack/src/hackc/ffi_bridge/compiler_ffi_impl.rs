// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Module containing conversion methods between the Rust Facts and
//! Rust/C++ shared Facts (in the ffi module)

use serde::de::Visitor;
use serde::Deserialize;
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

impl From<(String, Vec<facts::AttrValue>)> for ffi::AttrFacts {
    fn from((name, args): (String, Vec<facts::AttrValue>)) -> Self {
        use facts::AttrValue;
        Self {
            name,
            args: (args.into_iter())
                .map(|v| match v {
                    AttrValue::Classname(s) | AttrValue::String(s) | AttrValue::Int(s) => s,
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

impl<'de> Deserialize<'de> for ffi::TypeKind {
    fn deserialize<D: Deserializer<'de>>(d: D) -> Result<Self, D::Error> {
        struct KindVisitor;
        impl<'de> Visitor<'de> for KindVisitor {
            type Value = ffi::TypeKind;
            fn expecting(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                write!(f, "a lowercase type kind")
            }
            fn visit_str<E: serde::de::Error>(self, v: &str) -> Result<Self::Value, E> {
                match v {
                    "class" => Ok(ffi::TypeKind::Class),
                    "interface" => Ok(ffi::TypeKind::Interface),
                    "trait" => Ok(ffi::TypeKind::Trait),
                    "enum" => Ok(ffi::TypeKind::Enum),
                    "typeAlias" => Ok(ffi::TypeKind::TypeAlias),
                    "unknown" => Ok(ffi::TypeKind::Unknown),
                    "mixed" => Ok(ffi::TypeKind::Mixed),
                    x => Err(E::custom(format!("{x}: unexpected TypeKind"))),
                }
            }
        }

        d.deserialize_str(KindVisitor)
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

    fn sha1(text: &[u8]) -> String {
        use sha1::digest::Digest;
        let mut digest = sha1::Sha1::new();
        digest.update(text);
        hex::encode(digest.finalize())
    }

    #[test]
    fn sha1_some_text() {
        let text = b"some text";
        assert_eq!(
            sha1(text),
            String::from("37aa63c77398d954473262e1a0057c1e632eda77"),
        );
    }

    pub fn fake_facts() -> (facts::Facts, String) {
        let mut types = facts::TypeFactsByName::new();
        let base_types = BTreeSet::from_iter(vec!["bt3".into(), "bt1".into(), "bt2".into()]);
        types.insert(
            String::from("include_empty_both_when_trait_kind"),
            facts::TypeFacts {
                kind: facts::TypeKind::Trait,
                base_types,
                flags: 6,
                ..Default::default()
            },
        );
        // verify requireImplements, requireExtends and requireClass are skipped if empty and Class kind
        types.insert(
            String::from("include_empty_neither_when_class_kind"),
            facts::TypeFacts {
                kind: facts::TypeKind::Class,
                flags: 0,
                ..Default::default()
            },
        );
        // verify only requireImplements is skipped if empty and Interface kind
        types.insert(
            String::from("include_empty_req_extends_when_interface_kind"),
            facts::TypeFacts {
                kind: facts::TypeKind::Interface,
                flags: 1,
                ..Default::default()
            },
        );
        // verify non-empty require* is included
        types.insert(
            String::from("include_nonempty_always"),
            facts::TypeFacts {
                kind: facts::TypeKind::Unknown,
                flags: 9,
                attributes: {
                    let mut map = facts::Attributes::new();
                    map.insert("A".into(), vec![facts::AttrValue::String("'B'".into())]);
                    map.insert("C".into(), Vec::new());
                    map
                },
                require_extends: {
                    let mut set = facts::StringSet::new();
                    set.insert("extends1".into());
                    set
                },
                require_implements: {
                    let mut set = facts::StringSet::new();
                    set.insert("impl1".into());
                    set
                },
                require_class: {
                    let mut set = facts::StringSet::new();
                    set.insert("class1".into());
                    set
                },
                ..Default::default()
            },
        );
        types.insert(
            String::from("include_method_attrs"),
            facts::TypeFacts {
                kind: facts::TypeKind::Class,
                flags: 6,
                methods: vec![
                    (
                        String::from("no_attrs"),
                        facts::MethodFacts {
                            attributes: facts::Attributes::new(),
                        },
                    ),
                    (
                        String::from("one_attr_with_arg"),
                        facts::MethodFacts {
                            attributes: vec![(
                                "attr_with_arg".into(),
                                vec![facts::AttrValue::String("arg".into())],
                            )]
                            .into_iter()
                            .collect(),
                        },
                    ),
                ]
                .into_iter()
                .collect(),
                ..Default::default()
            },
        );
        types.insert(
            String::from("my_type_alias"),
            facts::TypeFacts {
                kind: facts::TypeKind::TypeAlias,
                ..Default::default()
            },
        );
        let mut modules = facts::ModuleFactsByName::new();
        modules.insert(String::from("foo"), facts::ModuleFacts {});
        let facts = facts::Facts {
            constants: vec!["c1".into(), "c2".into()],
            file_attributes: BTreeMap::new(),
            functions: vec![],
            modules,
            types,
        };
        (facts, sha1(b"fake source text"))
    }

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

    #[test]
    fn to_json() {
        // test to_string_pretty()
        let (facts, sha1) = fake_facts();
        let file_facts = ffi::FileFacts::from_facts(facts, sha1);
        assert_eq!(
            serde_json::to_string_pretty(&file_facts).unwrap(),
            r#"{
  "constants": [
    "c1",
    "c2"
  ],
  "modules": [
    {
      "name": "foo"
    }
  ],
  "sha1sum": "883c01f3eb209c249f88908675c8632a04a817cf",
  "types": [
    {
      "baseTypes": [
        "bt1",
        "bt2",
        "bt3"
      ],
      "flags": 6,
      "kindOf": "trait",
      "name": "include_empty_both_when_trait_kind"
    },
    {
      "flags": 0,
      "kindOf": "class",
      "name": "include_empty_neither_when_class_kind"
    },
    {
      "flags": 1,
      "kindOf": "interface",
      "name": "include_empty_req_extends_when_interface_kind"
    },
    {
      "flags": 6,
      "kindOf": "class",
      "methods": {
        "no_attrs": {},
        "one_attr_with_arg": {
          "attributes": {
            "attr_with_arg": [
              "arg"
            ]
          }
        }
      },
      "name": "include_method_attrs"
    },
    {
      "attributes": {
        "A": [
          "'B'"
        ],
        "C": []
      },
      "flags": 9,
      "kindOf": "unknown",
      "name": "include_nonempty_always",
      "requireClass": [
        "class1"
      ],
      "requireExtends": [
        "extends1"
      ],
      "requireImplements": [
        "impl1"
      ]
    },
    {
      "flags": 0,
      "kindOf": "typeAlias",
      "name": "my_type_alias"
    }
  ]
}"#,
        )
    }

    #[test]
    fn round_trip_json() -> serde_json::Result<()> {
        let (f1, _) = fake_facts();
        let ugly = serde_json::to_string(&f1).unwrap();
        let f2 = serde_json::from_str(&ugly)?;
        assert_eq!(f1, f2);
        let pretty = serde_json::to_string_pretty(&f1).unwrap();
        let f2 = serde_json::from_str(&pretty)?;
        assert_eq!(f1, f2);
        Ok(())
    }

    #[test]
    fn type_kind_to_json() {
        assert_eq!(
            serde_json::json!(ffi::TypeKind::Unknown).to_string(),
            "\"unknown\""
        );
        assert_eq!(
            serde_json::json!(ffi::TypeKind::Interface).to_string(),
            "\"interface\""
        );
    }
}
