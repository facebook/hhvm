// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;

use hhbc_string_utils::mangle_xhp_id;
use hhbc_string_utils::strip_global_ns;
use oxidized_by_ref::ast_defs::Abstraction;
use oxidized_by_ref::ast_defs::ClassishKind;
use oxidized_by_ref::direct_decl_parser::ParsedFile;
use oxidized_by_ref::shallow_decl_defs::ClassDecl;
use serde::ser::SerializeSeq;
use serde::Serializer;
use serde_derive::Serialize;
use serde_json::json;

#[derive(Debug, Clone, PartialEq, Serialize)]
#[serde(rename_all = "camelCase")]
enum TypeKind {
    Class,
    Interface,
    Enum,
    Trait,
    TypeAlias,
    Unknown,
    Mixed,
}

impl Default for TypeKind {
    fn default() -> Self {
        Self::Unknown
    }
}

type TypeFactsByName = BTreeMap<String, TypeFacts>;

#[derive(Debug, PartialEq, Serialize, Default)]
#[serde(rename_all = "camelCase")]
struct TypeFacts {
    #[serde(rename = "kindOf")]
    kind: TypeKind,

    flags: u8,
}

#[derive(Default, Debug, PartialEq, Serialize)]
#[serde(rename_all = "camelCase")]
struct Facts {
    #[serde(
        default,
        skip_serializing_if = "TypeFactsByName::is_empty",
        serialize_with = "types_to_json"
    )]
    types: TypeFactsByName,

    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    functions: Vec<String>,

    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    constants: Vec<String>,
}

impl Facts {
    fn to_json(&self, pretty: bool) -> String {
        let json = json!(self);
        if pretty {
            serde_json::to_string_pretty(&json).expect("Could not serialize facts to JSON")
        } else {
            serde_json::to_string(&json).expect("Could not serialize facts to JSON")
        }
    }

    fn from_decls(parsed_file: &ParsedFile<'_>) -> Facts {
        let mut types = TypeFactsByName::new();
        parsed_file.decls.classes().for_each(|(class_name, decl)| {
            let mut name = format(class_name);
            if !parsed_file.disable_xhp_element_mangling && decl.is_xhp {
                // strips the namespace and mangles the class id
                if let Some(id) = name.rsplit('\\').next() {
                    name = id.to_string();
                }
            }
            let type_fact = TypeFacts::of_class_decl(decl);
            add_or_update_classish_decl(name, type_fact, &mut types);
        });
        for (name, _) in parsed_file.decls.typedefs().filter(|(_, decl)| {
            // Ignore context aliases
            !decl.is_ctx
        }) {
            let type_fact = TypeFacts::of_typedef_decl();
            add_or_update_classish_decl(format(name), type_fact, &mut types);
        }

        let mut functions = parsed_file
            .decls
            .funs()
            .filter_map(|(name, _)| {
                let name = format(name);
                if name.eq("__construct") {
                    None
                } else {
                    Some(name)
                }
            })
            .collect::<Vec<String>>();
        let mut constants = parsed_file
            .decls
            .consts()
            .map(|(name, _)| format(name))
            .collect::<Vec<String>>();

        functions.reverse();
        constants.reverse();

        Facts {
            types,
            functions,
            constants,
        }
    }
}

type Flags = u8;

/// Compute minimal Facts in json format, including only the information
/// required by OCaml Facts.facts_from_json().
pub fn decls_to_facts_json(parsed_file: &ParsedFile<'_>, pretty: bool) -> String {
    Facts::from_decls(parsed_file).to_json(pretty)
}

#[derive(Clone, Copy)]
enum Flag {
    Abstract = 1,
    Final = 2,
    MultipleDeclarations = 4,
}

impl Flag {
    fn as_flags(&self) -> Flags {
        *self as Flags
    }

    #[cfg(test)]
    fn zero() -> Flags {
        0
    }

    #[cfg(test)]
    fn is_set(&self, flags: Flags) -> bool {
        (flags & (*self as Flags)) != 0
    }

    fn set(self, flags: Flags) -> Flags {
        flags | (self as Flags)
    }

    fn combine(flags1: Flags, flags2: Flags) -> Flags {
        flags1 | flags2
    }
}

// implementation details

/// Serialize the Map<Name, TypeFacts> as a sequence of JSON objects with `name`
/// as one of the fields.
fn types_to_json<S: Serializer>(types_by_name: &TypeFactsByName, s: S) -> Result<S::Ok, S::Error> {
    let mut seq = s.serialize_seq(None)?;
    for (name, types) in types_by_name.iter() {
        // pull the "name" key into the associated json object, then append to list
        let mut types_json = json!(types);
        if let Some(m) = types_json.as_object_mut() {
            m.insert("name".to_owned(), json!(name));
        };
        seq.serialize_element(&types_json)?;
    }
    seq.end()
}

impl TypeFacts {
    fn of_class_decl(decl: &ClassDecl<'_>) -> TypeFacts {
        let ClassDecl { kind, final_, .. } = decl;

        // Set flags according to modifiers - abstract, final, static (abstract + final)
        let mut flags = Flags::default();
        let kind = match kind {
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
            ClassishKind::Cenum => TypeKind::Enum,
            ClassishKind::CenumClass(abstraction) => {
                flags = modifiers_to_flags(flags, *final_, *abstraction);
                TypeKind::Enum
            }
        };

        TypeFacts { kind, flags }
    }

    fn of_typedef_decl() -> TypeFacts {
        TypeFacts {
            kind: TypeKind::TypeAlias,
            flags: Flags::default(),
        }
    }
}

fn add_or_update_classish_decl(name: String, delta: TypeFacts, types: &mut TypeFactsByName) {
    types
        .entry(name)
        .and_modify(|tf| {
            if tf.kind != delta.kind {
                tf.kind = TypeKind::Mixed;
            }
            tf.flags = Flag::MultipleDeclarations.set(tf.flags);
            tf.flags = Flag::combine(tf.flags, delta.flags);
        })
        .or_insert(delta);
}

fn format(original_name: &str) -> String {
    let unqualified = strip_global_ns(original_name);
    match unqualified.rsplit('\\').next() {
        Some(id) if original_name.starts_with('\\') && id.starts_with(':') => {
            // only mangle already qualified xhp ids - avoid mangling string literals
            // containing an xhp name, for example an attribute param ':foo:bar'
            mangle_xhp_id(id.to_string())
        }
        _ => String::from(unqualified),
    }
}

fn modifiers_to_flags(flags: Flags, is_final: bool, abstraction: Abstraction) -> Flags {
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

// inline tests (so stuff can remain hidden) - compiled only when tests are run (no overhead)

#[cfg(test)]
mod tests {
    use pretty_assertions::assert_eq;

    use super::*; // make assert_eq print huge diffs more human-readable

    #[test]
    fn type_kind_to_json() {
        assert_eq!(json!(TypeKind::Unknown).to_string(), "\"unknown\"");
        assert_eq!(json!(TypeKind::Interface).to_string(), "\"interface\"");
    }

    fn fake_facts() -> Facts {
        let mut types = TypeFactsByName::new();
        types.insert(
            String::from("include_empty_both_when_trait_kind"),
            TypeFacts {
                kind: TypeKind::Trait,
                flags: 6,
            },
        );
        // verify requireImplements, requireExtends and requireClass are skipped if empty and Class kind
        types.insert(
            String::from("include_empty_neither_when_class_kind"),
            TypeFacts {
                kind: TypeKind::Class,
                flags: 0,
            },
        );
        // verify only requireImplements is skipped if empty and Interface kind
        types.insert(
            String::from("include_empty_req_extends_when_interface_kind"),
            TypeFacts {
                kind: TypeKind::Interface,
                flags: 1,
            },
        );
        // verify non-empty require* is included
        types.insert(
            String::from("include_nonempty_always"),
            TypeFacts {
                kind: TypeKind::Unknown,
                flags: 9,
            },
        );
        types.insert(
            String::from("include_method_attrs"),
            TypeFacts {
                kind: TypeKind::Class,
                flags: 6,
            },
        );
        types.insert(
            String::from("my_type_alias"),
            TypeFacts {
                kind: TypeKind::TypeAlias,
                ..Default::default()
            },
        );
        Facts {
            constants: vec!["c1".into(), "c2".into()],
            functions: vec![],
            types,
        }
    }

    #[test]
    fn to_json() {
        // test to_string_pretty()
        let facts = fake_facts();
        assert_eq!(
            facts.to_json(true),
            r#"{
  "constants": [
    "c1",
    "c2"
  ],
  "types": [
    {
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
      "name": "include_method_attrs"
    },
    {
      "flags": 9,
      "kindOf": "unknown",
      "name": "include_nonempty_always"
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
    fn test_flags() {
        let flags = Flag::zero();
        assert!(!Flag::Final.is_set(flags));
        let flags = Flag::Final.set(flags);
        let flags = Flag::Abstract.set(flags);
        assert!(Flag::Final.is_set(flags));
        assert!(Flag::Abstract.is_set(flags));
    }
}
