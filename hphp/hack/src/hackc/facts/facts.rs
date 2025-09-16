// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::collections::BTreeSet;

use hash::IndexSet;
use hhbc_string_utils::mangle_xhp_id;
use hhbc_string_utils::strip_global_ns;
use naming_special_names_rust::user_attributes;
use oxidized::ast_defs::Abstraction;
use oxidized::ast_defs::ClassishKind;
use oxidized::direct_decl_parser::ParsedFile;
use oxidized::shallow_decl_defs::ClassDecl;
use oxidized::shallow_decl_defs::DeclConstraintRequirement;
use oxidized::shallow_decl_defs::TypedefDecl;
use oxidized::typing_defs::Ty;
use oxidized::typing_defs::Ty_;
use oxidized::typing_defs::UserAttribute;
use oxidized::typing_defs::UserAttributeParam;
use serde::Deserialize;
use serde::Serialize;

#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub enum TypeKind {
    Class,
    Interface,

    /// Used for legacy enum and enum class
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

impl TypeKind {
    #[inline]
    pub fn is_classish(&self) -> bool {
        matches!(
            self,
            Self::Class | Self::Interface | Self::Trait | Self::Enum
        )
    }

    #[inline]
    pub fn is_typedef(&self) -> bool {
        *self == Self::TypeAlias
    }
}

// TODO: all variants are represented as string for no good reason;
// as of april 2024, attributes are encoded as json in sqlite FactsDB.
// However, attribute values are hack values and only consumed in hack,
// so hack/php serializing would be much better than json. This will
// require carefully migrating the FactsDB schema.
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub enum AttrValue {
    Classname(String),
    String(String),
    Int(String),
}

impl AttrValue {
    pub fn into_json(self) -> serde_json::Value {
        match self {
            Self::Classname(s) | Self::String(s) | Self::Int(s) => serde_json::Value::String(s),
        }
    }

    pub fn to_json(&self) -> serde_json::Value {
        match self {
            Self::Classname(s) | Self::String(s) | Self::Int(s) => {
                serde_json::Value::String(s.clone())
            }
        }
    }

    pub fn value(&self) -> String {
        match self {
            Self::Classname(s) | Self::String(s) | Self::Int(s) => s.clone(),
        }
    }
}

pub type Attributes = BTreeMap<String, Vec<AttrValue>>;
pub type TypeFactsByName = BTreeMap<String, TypeFacts>;

#[derive(Debug, PartialEq, Serialize, Deserialize, Clone)]
pub struct MethodFacts {
    pub attributes: Attributes,
}

#[derive(Debug, PartialEq, Serialize, Deserialize, Default, Clone)]
pub struct TypeFacts {
    pub base_types: BTreeSet<String>,
    pub kind: TypeKind,
    pub attributes: Attributes,
    pub flags: Flags,
    pub require_extends: BTreeSet<String>,
    pub require_implements: BTreeSet<String>,
    pub require_class: BTreeSet<String>,
    pub methods: BTreeMap<String, MethodFacts>,
}

// Currently module facts are empty, but added for backward compatibility
#[derive(Debug, PartialEq, Serialize, Deserialize, Clone)]
pub struct ModuleFacts {}

pub type ModuleFactsByName = BTreeMap<String, ModuleFacts>;

#[derive(Default, Debug, PartialEq, Serialize, Deserialize, Clone)]
pub struct Facts {
    pub types: TypeFactsByName,
    pub functions: Vec<String>,
    pub constants: Vec<String>,
    pub file_attributes: Attributes,
    pub modules: ModuleFactsByName,
    pub module_membership: Option<String>,
}

// This must keep in sync with hack/src/facebook/hh_distc/facts/facts_sqlite.rs.
impl Facts {
    pub fn from_decls(parsed_file: &ParsedFile) -> Facts {
        let mut types = Default::default();
        parsed_file.decls.classes().for_each(|(class_name, decl)| {
            let mut name = format(class_name);
            if !parsed_file.disable_xhp_element_mangling && decl.is_xhp {
                // strips the namespace and mangles the class id
                if let Some(id) = name.rsplit('\\').next() {
                    name = id.to_string();
                }
            }
            let type_fact = TypeFacts::from_class_decl(decl);
            add_or_update_classish_decl(name, type_fact, &mut types);
        });
        for (name, decl) in parsed_file.decls.typedefs().filter(|(_, decl)| {
            // Ignore context aliases
            !decl.is_ctx
        }) {
            let type_fact = TypeFacts::from_typedef_decl(decl);
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

        let mut modules = ModuleFactsByName::new();
        parsed_file
            .decls
            .modules()
            .for_each(|(module_name, _decl)| {
                let name = format(module_name);
                add_or_update_module_decl(name, ModuleFacts {}, &mut modules);
            });
        functions.reverse();
        constants.reverse();

        let file_attributes = to_facts_attributes(&parsed_file.file_attributes);

        Facts {
            types,
            functions,
            constants,
            file_attributes,
            modules,
            module_membership: parsed_file.module_membership.clone(),
        }
    }

    /// If indexed_method_attrs is non-empty, then remove method attributes that are
    /// not in the provided set. Then, remove all MethodFacts with no attributes.
    pub fn filter_method_attributes(&mut self, indexed_method_attrs: &IndexSet<String>) {
        // Only keep methods that have some attributes after accounting for indexed_method_attrs
        for types in self.types.values_mut() {
            types.methods.retain(|_, method_facts| {
                if !indexed_method_attrs.is_empty() {
                    method_facts
                        .attributes
                        .retain(|attr, _| indexed_method_attrs.contains(attr));
                }
                !method_facts.attributes.is_empty()
            })
        }
    }
}

pub type Flags = u8;
#[derive(Clone, Copy)]
pub enum Flag {
    Abstract = 1,
    Final = 2,
    MultipleDeclarations = 4,
}

impl Flag {
    pub fn as_flags(&self) -> Flags {
        *self as Flags
    }
    pub fn zero() -> Flags {
        0
    }
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

impl TypeFacts {
    fn from_class_decl(decl: &ClassDecl) -> TypeFacts {
        let ClassDecl {
            kind,
            final_,
            req_implements,
            req_extends,
            req_constraints,
            uses,
            extends,
            implements,
            user_attributes,
            methods,
            static_methods,
            enum_type,
            ..
        } = decl;

        // Collect base types from uses, extends, and implements
        let mut base_types: BTreeSet<String> = Default::default();
        uses.iter().for_each(|ty| {
            base_types.insert(extract_type_name(ty));
        });
        extends.iter().for_each(|ty| {
            base_types.insert(extract_type_name(ty));
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
            .iter()
            .filter_map(|ty| {
                if check_require {
                    Some(extract_type_name(ty))
                } else {
                    None
                }
            })
            .collect();
        let require_implements = req_implements
            .iter()
            .filter_map(|ty| {
                if check_require {
                    Some(extract_type_name(ty))
                } else {
                    None
                }
            })
            .collect();
        let require_class = req_constraints
            .iter()
            .filter_map(|dcr| {
                if check_require {
                    match dcr {
                        DeclConstraintRequirement::DCREqual(ty) => Some(extract_type_name(ty)),
                        DeclConstraintRequirement::DCRSubtype(_) => None,
                    }
                } else {
                    None
                }
            })
            .collect();

        // TODO(T101762617): modify the direct decl parser to
        // preserve the attribute params that facts expects
        let attributes = to_facts_attributes(user_attributes);

        let methods = methods
            .iter()
            .chain(static_methods.iter())
            .filter_map(|m| {
                let attributes = to_facts_attributes(&m.attributes);
                // Add this method to the output iff it's
                // decorated with non-builtin attributes
                if attributes.is_empty() {
                    None
                } else {
                    Some((format(&m.name.1), MethodFacts { attributes }))
                }
            })
            .collect();

        TypeFacts {
            base_types,
            kind: typekind,
            require_extends,
            flags,
            require_implements,
            require_class,
            attributes,
            methods,
        }
    }

    fn from_typedef_decl(decl: &TypedefDecl) -> TypeFacts {
        TypeFacts {
            kind: TypeKind::TypeAlias,
            attributes: to_facts_attributes(&decl.attributes),
            ..Default::default()
        }
    }
}

fn add_or_update_classish_decl(name: String, mut delta: TypeFacts, types: &mut TypeFactsByName) {
    types
        .entry(name)
        .and_modify(|tf| {
            if tf.kind != delta.kind {
                tf.kind = TypeKind::Mixed;
            }
            tf.flags = Flag::MultipleDeclarations.set(tf.flags);
            tf.flags = Flag::combine(tf.flags, delta.flags);
            tf.base_types.append(&mut delta.base_types);
            tf.attributes.append(&mut delta.attributes);
            tf.require_extends.append(&mut delta.require_extends);
            tf.require_implements.append(&mut delta.require_implements);
            tf.require_class.append(&mut delta.require_class)
        })
        .or_insert(delta);
}

fn add_or_update_module_decl(name: String, delta: ModuleFacts, types: &mut ModuleFactsByName) {
    types
        .entry(name)
        // .and_modify(|mf| {}) (to be added when modules have actual bodies)
        .or_insert(delta);
}

// TODO: move to typing_defs_core_impl.rs once completed
fn extract_type_name(ty: &Ty) -> String {
    match ty.get_node() {
        Ty_::Tapply((_, id), _) => format(id),
        Ty_::Tgeneric(id) => format(id),
        _ => unimplemented!("{:?}", ty),
    }
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

fn to_facts_attributes(attributes: &[UserAttribute]) -> Attributes {
    attributes
        .iter()
        .filter_map(|ua| {
            let params = (ua.params.iter())
                .filter_map(|p| match p {
                    UserAttributeParam::Classname(cn) => Some(AttrValue::Classname(format(cn))),
                    UserAttributeParam::String(s) => Some(AttrValue::String(s.to_string())),
                    UserAttributeParam::Int(i) => Some(AttrValue::Int(i.to_owned())),
                    _ => None,
                })
                .collect();
            let attr_name = format(&ua.name.1);
            if user_attributes::is_reserved(&attr_name) {
                // skip builtins
                None
            } else {
                Some((attr_name, params))
            }
        })
        .collect::<Attributes>()
}

/// A wrapper to group together a Facts object with the sha1 hash of the source
/// file from which it was parsed.
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct FactsSha1 {
    pub facts: Facts,

    /// Sha1 digest of source file from which we parsed these facts.
    pub sha1sum: String,
}

#[cfg(test)]
mod tests {
    use super::*;

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
