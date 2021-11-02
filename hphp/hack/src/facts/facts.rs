// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::{BTreeMap, BTreeSet};

use digest::Digest;
use hhbc_by_ref_hhbc_string_utils::strip_global_ns;
use naming_special_names_rust::user_attributes;
use oxidized_by_ref::{
    ast_defs::{Abstraction, ClassishKind},
    direct_decl_parser::Decls,
    shallow_decl_defs::{ClassDecl, TypedefDecl},
    typing_defs::{Ty, Ty_, UserAttribute},
};
use serde::ser::SerializeSeq;
use serde::Serializer;
use serde_derive::Serialize;
use serde_json::json;

use crate::facts_parser::add_or_update_classish_decl;

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

#[derive(Debug, PartialEq, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct MethodFacts {
    #[serde(skip_serializing_if = "Attributes::is_empty")]
    pub attributes: Attributes,
}

pub type Methods = BTreeMap<String, MethodFacts>;

#[derive(Debug, PartialEq, Serialize)]
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

#[derive(Default, Debug, PartialEq, Serialize)]
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

    pub fn facts_of_decls<'a>(decls: &Decls<'a>) -> Facts {
        // 10/15/2021 TODO(T103413083): Fill out the implementation

        let mut types = TypeFactsByName::new();
        decls.classes().for_each(|(name, decl)| {
            let name = if decl.is_xhp && !decl.has_xhp_keyword {
                format_xhp(name)
            } else {
                format(name)
            };
            let type_fact = TypeFacts::of_class_decl(decl);
            add_or_update_classish_decl(name, type_fact, &mut types);
        });
        let mut type_aliases = decls
            .typedefs()
            .filter_map(|(name, decl)| {
                if decl.is_ctx {
                    // ignore context aliases
                    None
                } else {
                    types.insert(format(name), TypeFacts::of_typedef_decl(decl));
                    Some(format(name))
                }
            })
            .collect::<Vec<String>>();
        let mut functions = decls
            .funs()
            .map(|(name, _)| format(name))
            .collect::<Vec<String>>();
        let mut constants = decls
            .consts()
            .map(|(name, _)| format(name))
            .collect::<Vec<String>>();

        functions.reverse();
        constants.reverse();
        type_aliases.reverse();

        Facts {
            types,
            functions,
            constants,
            type_aliases,
            // TODO: file attributes
            ..Default::default()
        }
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

    fn of_class_decl<'a>(decl: &'a ClassDecl<'a>) -> TypeFacts {
        let ClassDecl {
            kind,
            final_,
            req_implements,
            req_extends,
            uses,
            extends,
            implements,
            user_attributes,
            methods,
            static_methods,
            enum_type,
            is_xhp,
            ..
        } = decl;

        // Collect base types from uses, extends, and implements
        let mut base_types = StringSet::new();
        uses.iter().for_each(|ty| {
            base_types.insert(extract_type_name(ty));
        });
        extends.iter().for_each(|ty| {
            let mut type_name = extract_type_name(ty);
            if *is_xhp {
                type_name = format_xhp(&type_name);
            }
            base_types.insert(type_name);
        });
        implements.iter().for_each(|ty| {
            base_types.insert(extract_type_name(ty));
        });

        // Set flags according to modifiers - abstract, final, static (abstract + final)
        let mut flags = Flags::default();
        let typekind = match kind {
            ClassishKind::Cclass(abstraction) => {
                flags = modifiers_to_flags(flags, *final_, *abstraction);
                TypeKind::Class
            }
            ClassishKind::Cinterface => {
                flags = Flag::Abstract.as_flags();
                TypeKind::Interface
            }
            ClassishKind::Ctrait => {
                flags = Flag::Abstract.as_flags();
                TypeKind::Trait
            }
            ClassishKind::Cenum => {
                if let Some(et) = enum_type {
                    et.includes.iter().for_each(|ty| {
                        base_types.insert(extract_type_name(ty));
                    });
                }
                TypeKind::Enum
            }
            ClassishKind::CenumClass(abstraction) => {
                flags = modifiers_to_flags(flags, *final_, *abstraction);
                TypeKind::Enum
            }
        };

        let check_require = match typekind {
            TypeKind::Interface | TypeKind::Trait => true,
            _ => false,
        };

        // Collect the requires
        let require_extends = req_extends
            .into_iter()
            .filter_map(|&ty| {
                if check_require {
                    Some(extract_type_name(ty))
                } else {
                    None
                }
            })
            .collect::<StringSet>();
        let require_implements = req_implements
            .into_iter()
            .filter_map(|&ty| {
                if check_require {
                    Some(extract_type_name(ty))
                } else {
                    None
                }
            })
            .collect::<StringSet>();

        // TODO(T101762617): modify the direct decl parser to
        // preserve the attribute params that facts expects
        let attributes = to_facts_attributes(user_attributes);

        let methods = methods
            .iter()
            .chain(static_methods.into_iter())
            .filter_map(|m| {
                let attributes = to_facts_attributes(m.attributes);
                // Add this method to the output iff it's
                // decorated with non-builtin attributes
                if attributes.is_empty() {
                    None
                } else {
                    Some((format(m.name.1), MethodFacts { attributes }))
                }
            })
            .collect::<Methods>();

        TypeFacts {
            base_types,
            kind: typekind,
            require_extends,
            flags,
            require_implements,
            attributes,
            methods,
        }
    }

    fn of_typedef_decl<'a>(decl: &'a TypedefDecl<'a>) -> TypeFacts {
        TypeFacts {
            base_types: StringSet::new(),
            kind: TypeKind::TypeAlias,
            attributes: to_facts_attributes(decl.attributes),
            require_extends: StringSet::new(),
            flags: Flags::default(),
            require_implements: StringSet::new(),
            methods: BTreeMap::new(),
        }
    }
}

fn hash_and_hexify<D: Digest>(mut digest: D, text: &[u8]) -> String {
    digest.input(text);
    hex::encode(digest.result())
}

fn hex_number_to_i64(s: &str) -> i64 {
    u64::from_str_radix(s, 16).unwrap() as i64
}

// TODO: move to typing_defs_core_impl.rs once completed
fn extract_type_name<'a>(ty: &Ty<'a>) -> String {
    match ty.get_node() {
        Ty_::Tapply(((_, id), _)) => format(*id),
        Ty_::Tgeneric((id, _)) => format(*id),
        _ => unimplemented!("{:?}", ty),
    }
}

fn format<'a>(type_name: &'a str) -> String {
    String::from(strip_global_ns(type_name))
}

fn format_xhp<'a>(name: &'a str) -> String {
    format!("xhp_{}", strip_global_ns(name))
        .replace("\\", "__")
        .replace("-", "_")
}

fn modifiers_to_flags(flags: isize, is_final: bool, abstraction: &Abstraction) -> isize {
    let flags = match abstraction {
        Abstraction::Abstract => Flag::Abstract.set(flags),
        Abstraction::Concrete => flags,
    };
    if is_final {
        Flag::Final.set(flags)
    } else {
        flags
    }
}

fn to_facts_attributes<'a>(attributes: &'a [&'a UserAttribute<'a>]) -> Attributes {
    attributes
        .iter()
        .filter_map(|ua| {
            let attr_name = format(ua.name.1);
            if user_attributes::AS_SET.contains(attr_name.as_str()) {
                // skip builtins
                None
            } else {
                Some((
                    attr_name,
                    ua.classname_params
                        .iter()
                        .map(|param| format(*param).to_string())
                        .collect::<Vec<String>>(),
                ))
            }
        })
        .collect::<Attributes>()
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
