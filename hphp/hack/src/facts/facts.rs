// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::{BTreeMap, BTreeSet};

use digest::Digest;
use serde::ser::SerializeSeq;
use serde::Serializer;
use serde_derive::Serialize;
use serde_json::json;

#[derive(Debug, Clone, PartialEq, Serialize)]
#[serde(rename_all = "camelCase")]
pub enum TypeKind {
    Class,
    Record,
    Interface,
    Enum,
    Trait,
    TypeAlias,
    Unknown,
    Mixed,
}

pub type StringSet = BTreeSet<String>;
pub type Attributes = BTreeMap<String, Vec<String>>;

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
pub struct MethodFacts {
    #[serde(skip_serializing_if = "Attributes::is_empty")]
    pub attributes: Attributes,
}

pub type Methods = BTreeMap<String, MethodFacts>;

#[derive(Serialize)]
#[serde(rename_all = "camelCase")]
pub struct TypeFacts {
    pub base_types: StringSet,
    #[serde(rename = "kindOf")]
    pub kind: TypeKind,
    pub attributes: Attributes,
    pub flags: isize,
    pub require_extends: StringSet,
    pub require_implements: StringSet,
    #[serde(skip_serializing_if = "Methods::is_empty")]
    pub methods: Methods,
}

pub type TypeFactsByName = BTreeMap<String, TypeFacts>;

#[derive(Default, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct Facts {
    #[serde(serialize_with = "types_to_json")]
    pub types: TypeFactsByName,
    pub functions: Vec<String>,
    pub constants: Vec<String>,
    pub type_aliases: Vec<String>,
    pub file_attributes: Attributes,
}
impl Facts {
    pub fn to_json(&self, text: &[u8]) -> String {
        let (md5sum, sha1sum) = md5_and_sha1(text);
        let mut json = json!(&self);
        json.as_object_mut().map(|m| {
            m.insert(
                String::from("md5sum0"),
                json!(hex_number_to_i64(&md5sum[0..16])),
            );
            m.insert(
                String::from("md5sum1"),
                json!(hex_number_to_i64(&md5sum[16..32])),
            );
            m.insert(String::from("sha1sum"), json!(sha1sum));
            if self.skip_file_attributes() {
                m.remove("fileAttributes");
            }
        });
        serde_json::to_string_pretty(&json).expect("Could not serialize facts to JSON")
    }
    fn skip_file_attributes(&self) -> bool {
        self.file_attributes.is_empty()
    }
}

pub type Flags = isize;
#[derive(Clone, Copy)]
pub enum Flag {
    Abstract = 1,
    Final = 2,
    MultipleDeclarations = 4,
}
impl Flag {
    pub fn as_flags(&self) -> Flags {
        *self as isize
    }
    pub fn default() -> Flags {
        0
    }
    #[allow(dead_code)]
    pub fn is_set(&self, flags: Flags) -> bool {
        (flags & (*self as Flags)) != 0
    }
    pub fn set(self, flags: Flags) -> Flags {
        flags | (self as Flags)
    }
    pub fn combine(flags1: Flags, flags2: Flags) -> Flags {
        flags1 | flags2
    }
}

pub fn md5_and_sha1(text: &[u8]) -> (String, String) {
    (
        hash_and_hexify(md5::Md5::new(), text),
        hash_and_hexify(sha1::Sha1::new(), text),
    )
}

// implementation details

fn types_to_json<S: Serializer>(types_by_name: &TypeFactsByName, s: S) -> Result<S::Ok, S::Error> {
    let mut seq = s.serialize_seq(None)?;
    for (name, types) in types_by_name.iter() {
        // pull the "name" key into the associated json object, then append to list
        let mut types_json = json!(types);
        types_json.as_object_mut().map(|m| {
            m.insert("name".to_owned(), json!(name));
        });

        // possibly skip non-empty attributes/require*, depending on the kind
        if types.skip_attributes() {
            types_json.as_object_mut().map(|m| m.remove("attributes"));
        }
        if types.skip_require_extends() {
            types_json
                .as_object_mut()
                .map(|m| m.remove("requireExtends"));
        }
        if types.skip_require_implements() {
            types_json
                .as_object_mut()
                .map(|m| m.remove("requireImplements"));
        }
        seq.serialize_element(&types_json)?;
    }
    seq.end()
}

impl TypeFacts {
    fn skip_require_extends(&self) -> bool {
        match self.kind {
            TypeKind::Trait | TypeKind::Interface => false,
            _ => self.require_extends.is_empty(),
        }
    }
    fn skip_require_implements(&self) -> bool {
        match self.kind {
            TypeKind::Trait => false,
            _ => self.require_implements.is_empty(),
        }
    }
    fn skip_attributes(&self) -> bool {
        self.attributes.is_empty()
    }
}

fn hash_and_hexify<D: Digest>(mut digest: D, text: &[u8]) -> String {
    digest.input(text);
    hex::encode(digest.result())
}

fn hex_number_to_i64(s: &str) -> i64 {
    u64::from_str_radix(s, 16).unwrap() as i64
}

// inline tests (so stuff can remain hidden) - compiled only when tests are run (no overhead)

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq; // make assert_eq print huge diffs more human-readable

    #[test]
    fn type_kind_to_json() {
        assert_eq!(json!(TypeKind::Unknown).to_string(), "\"unknown\"");
        assert_eq!(json!(TypeKind::Interface).to_string(), "\"interface\"");
    }

    #[test]
    fn hex_number_to_json() {
        assert_eq!(hex_number_to_i64("23"), 35);
        assert_eq!(hex_number_to_i64("fffffffffffffffe"), -2);
    }

    #[test]
    fn md5_and_sha1_some_text() {
        let text = b"some text";
        assert_eq!(
            md5_and_sha1(text),
            (
                String::from("552e21cd4cd9918678e3c1a0df491bc3"),
                String::from("37aa63c77398d954473262e1a0057c1e632eda77"),
            ),
        );
    }

    #[test]
    fn string_set_to_json() {
        let mut ss = StringSet::new();
        ss.insert("foo".into());
        ss.insert("bar".into());
        assert_eq!(
            r#"[
  "bar",
  "foo"
]"#,
            serde_json::to_string_pretty(&ss).unwrap(),
        );
    }

    #[test]
    fn to_json() {
        let mut types = TypeFactsByName::new();
        {
            let mut base_types = StringSet::new();
            base_types.insert("bt3".into());
            base_types.insert("bt1".into());
            base_types.insert("bt2".into());
            types.insert(
                String::from("include_empty_both_when_trait_kind"),
                TypeFacts {
                    kind: TypeKind::Trait,
                    attributes: Attributes::new(),
                    require_extends: StringSet::new(),
                    require_implements: StringSet::new(),
                    base_types,
                    flags: 6,
                    methods: Methods::new(),
                },
            );
            // verify requireImplements and requireExtends are skipped if empty and Class kind
            types.insert(
                String::from("include_empty_neither_when_class_kind"),
                TypeFacts {
                    kind: TypeKind::Class,
                    attributes: Attributes::new(),
                    require_extends: StringSet::new(),
                    require_implements: StringSet::new(),
                    base_types: StringSet::new(),
                    flags: 0,
                    methods: Methods::new(),
                },
            );
            // verify only requireImplements is skipped if empty and Interface kind
            types.insert(
                String::from("include_empty_req_extends_when_interface_kind"),
                TypeFacts {
                    kind: TypeKind::Interface,
                    attributes: Attributes::new(),
                    require_extends: StringSet::new(),
                    require_implements: StringSet::new(),
                    base_types: StringSet::new(),
                    flags: 1,
                    methods: Methods::new(),
                },
            );
            // verify non-empty require* is included
            types.insert(
                String::from("include_nonempty_always"),
                TypeFacts {
                    kind: TypeKind::Unknown,
                    attributes: {
                        let mut map = Attributes::new();
                        map.insert("A".into(), {
                            let mut set = Vec::new();
                            set.push("'B'".into());
                            set
                        });
                        map.insert("C".into(), Vec::new());
                        map
                    },
                    require_extends: {
                        let mut set = StringSet::new();
                        set.insert("extends1".into());
                        set
                    },
                    require_implements: {
                        let mut set = StringSet::new();
                        set.insert("impl1".into());
                        set
                    },
                    base_types: StringSet::new(),
                    flags: 9,
                    methods: Methods::new(),
                },
            );
            types.insert(
                String::from("include_method_attrs"),
                TypeFacts {
                    kind: TypeKind::Class,
                    attributes: Attributes::new(),
                    require_extends: StringSet::new(),
                    require_implements: StringSet::new(),
                    base_types: StringSet::new(),
                    flags: 6,
                    methods: vec![
                        (
                            String::from("no_attrs"),
                            MethodFacts {
                                attributes: Attributes::new(),
                            },
                        ),
                        (
                            String::from("one_attr_with_arg"),
                            MethodFacts {
                                attributes: vec![(
                                    String::from("attr_with_arg"),
                                    vec![String::from("arg")],
                                )]
                                .into_iter()
                                .collect(),
                            },
                        ),
                    ]
                    .into_iter()
                    .collect(),
                },
            );
        }
        // to_string_pretty
        assert_eq!(
            (Facts {
                constants: vec!["c1".into(), "c2".into()],
                file_attributes: BTreeMap::new(),
                functions: vec![],
                type_aliases: vec!["my_type_alias".into()],
                types,
            })
            .to_json(b"some text"),
            r#"{
  "constants": [
    "c1",
    "c2"
  ],
  "functions": [],
  "md5sum0": 6137880507793904006,
  "md5sum1": 8711019000949709763,
  "sha1sum": "37aa63c77398d954473262e1a0057c1e632eda77",
  "typeAliases": [
    "my_type_alias"
  ],
  "types": [
    {
      "baseTypes": [
        "bt1",
        "bt2",
        "bt3"
      ],
      "flags": 6,
      "kindOf": "trait",
      "name": "include_empty_both_when_trait_kind",
      "requireExtends": [],
      "requireImplements": []
    },
    {
      "baseTypes": [],
      "flags": 0,
      "kindOf": "class",
      "name": "include_empty_neither_when_class_kind"
    },
    {
      "baseTypes": [],
      "flags": 1,
      "kindOf": "interface",
      "name": "include_empty_req_extends_when_interface_kind",
      "requireExtends": []
    },
    {
      "baseTypes": [],
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
      "baseTypes": [],
      "flags": 9,
      "kindOf": "unknown",
      "name": "include_nonempty_always",
      "requireExtends": [
        "extends1"
      ],
      "requireImplements": [
        "impl1"
      ]
    }
  ]
}"#,
        )
    }

    #[test]
    fn test_flags() {
        let flags = Flag::default();
        assert!(!Flag::Final.is_set(flags));
        let flags = Flag::Final.set(flags);
        let flags = Flag::Abstract.set(flags);
        assert!(Flag::Final.is_set(flags));
        assert!(Flag::Abstract.is_set(flags));
    }
}
