// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::BTreeMap;
use std::collections::BTreeSet;

use itertools::Itertools;

use super::*;

impl From<compile_ffi::TypeKind> for facts::TypeKind {
    fn from(type_kind: compile_ffi::TypeKind) -> Self {
        match type_kind {
            compile_ffi::TypeKind::Class => Self::Class,
            compile_ffi::TypeKind::Interface => Self::Interface,
            compile_ffi::TypeKind::Enum => Self::Enum,
            compile_ffi::TypeKind::Trait => Self::Trait,
            compile_ffi::TypeKind::TypeAlias => Self::TypeAlias,
            compile_ffi::TypeKind::Unknown => Self::Unknown,
            compile_ffi::TypeKind::Mixed => Self::Mixed,
            _ => panic!("impossible"),
        }
    }
}

impl From<facts::TypeKind> for compile_ffi::TypeKind {
    fn from(typekind: facts::TypeKind) -> Self {
        match typekind {
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

impl IntoKeyValue<String, Vec<serde_json::Value>> for compile_ffi::Attribute {
    fn into_key_value(self) -> (String, Vec<serde_json::Value>) {
        let args = (self.args.into_iter())
            .map(|s| serde_json::from_str(&s).unwrap())
            .collect();
        (self.name, args)
    }
}

impl FromKeyValue<String, Vec<serde_json::Value>> for compile_ffi::Attribute {
    fn from_key_value(name: String, args: Vec<serde_json::Value>) -> Self {
        let args = (args.into_iter())
            .map(|v| serde_json::to_string(&v).unwrap())
            .collect();
        Self { name, args }
    }
}

impl IntoKeyValue<String, facts::MethodFacts> for compile_ffi::MethodDetails {
    fn into_key_value(mut self) -> (String, facts::MethodFacts) {
        let name = std::mem::take(&mut self.name);
        (name, self.into())
    }
}

impl FromKeyValue<String, facts::MethodFacts> for compile_ffi::MethodDetails {
    fn from_key_value(name: String, methfacts: facts::MethodFacts) -> Self {
        Self {
            name,
            ..methfacts.into()
        }
    }
}

impl From<compile_ffi::MethodDetails> for facts::MethodFacts {
    fn from(md: compile_ffi::MethodDetails) -> Self {
        Self {
            attributes: vec_to_map(md.attributes),
        }
    }
}

impl From<facts::MethodFacts> for compile_ffi::MethodDetails {
    fn from(method_facts: facts::MethodFacts) -> Self {
        Self {
            name: String::default(),
            attributes: map_to_vec(method_facts.attributes),
        }
    }
}

impl From<compile_ffi::TypeDetails> for facts::TypeFacts {
    fn from(facts: compile_ffi::TypeDetails) -> Self {
        Self {
            base_types: vec_to_set(facts.base_types),
            kind: facts.kind.into(),
            attributes: vec_to_map(facts.attributes),
            flags: facts.flags,
            require_extends: vec_to_set(facts.require_extends),
            require_implements: vec_to_set(facts.require_implements),
            require_class: vec_to_set(facts.require_class),
            methods: vec_to_map(facts.methods),
        }
    }
}

impl From<facts::TypeFacts> for compile_ffi::TypeDetails {
    fn from(facts: facts::TypeFacts) -> Self {
        Self {
            name: String::default(),
            base_types: set_to_vec(facts.base_types),
            kind: facts.kind.into(),
            attributes: map_to_vec(facts.attributes),
            flags: facts.flags,
            require_extends: set_to_vec(facts.require_extends),
            require_implements: set_to_vec(facts.require_implements),
            require_class: set_to_vec(facts.require_class),
            methods: map_to_vec(facts.methods),
        }
    }
}

impl IntoKeyValue<String, facts::TypeFacts> for compile_ffi::TypeDetails {
    fn into_key_value(mut self) -> (String, facts::TypeFacts) {
        let name = std::mem::take(&mut self.name);
        (name, self.into())
    }
}

impl FromKeyValue<String, facts::TypeFacts> for compile_ffi::TypeDetails {
    fn from_key_value(name: String, typefacts: facts::TypeFacts) -> Self {
        Self {
            name,
            ..typefacts.into()
        }
    }
}

impl From<compile_ffi::FileFacts> for facts::Facts {
    fn from(facts: compile_ffi::FileFacts) -> Self {
        Self {
            types: vec_to_map(facts.types),
            functions: facts.functions,
            constants: facts.constants,
            modules: facts
                .modules
                .into_iter()
                .map(|x| (x.name, facts::ModuleFacts {}))
                .collect(),
            file_attributes: vec_to_map(facts.attributes),
        }
    }
}

impl From<facts::Facts> for compile_ffi::FileFacts {
    fn from(facts: facts::Facts) -> Self {
        Self {
            types: map_to_vec(facts.types),
            functions: facts.functions,
            constants: facts.constants,
            modules: facts
                .modules
                .into_keys()
                .map(|name| compile_ffi::ModuleDetails { name })
                .collect(),
            attributes: map_to_vec(facts.file_attributes),
            sha1hex: String::default(),
        }
    }
}

trait IntoKeyValue<K, V> {
    fn into_key_value(self) -> (K, V);
}

trait FromKeyValue<K, V> {
    fn from_key_value(k: K, v: V) -> Self;
}

fn vec_to_map<K, V, T>(v: Vec<T>) -> BTreeMap<K, V>
where
    K: std::cmp::Ord,
    T: IntoKeyValue<K, V>,
{
    v.into_iter()
        .map(|x| x.into_key_value())
        .collect::<BTreeMap<K, V>>()
}

fn map_to_vec<K, V, T>(m: BTreeMap<K, V>) -> Vec<T>
where
    T: FromKeyValue<K, V>,
{
    m.into_iter()
        .map(|(k, v)| T::from_key_value(k, v))
        .collect_vec()
}

fn vec_to_set<T: std::cmp::Ord>(v: Vec<T>) -> BTreeSet<T> {
    v.into_iter().collect()
}

fn set_to_vec<T>(s: BTreeSet<T>) -> Vec<T> {
    s.into_iter().collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_method_facts_1() {
        let (ffi_method_facts, rust_method_facts) = create_method_facts();
        assert_eq!(
            facts::MethodFacts::from(ffi_method_facts),
            rust_method_facts
        )
    }

    #[test]
    fn test_method_facts_2() {
        let (ffi_method_facts, mut rust_method_facts) = create_method_facts();
        rust_method_facts.attributes.remove_entry("MyAttribute2");
        assert_ne!(
            compile_ffi::MethodDetails::from(rust_method_facts),
            ffi_method_facts
        )
    }

    #[test]
    fn test_methods_1() {
        let (ffi_methods, rust_methods) = create_methods();
        assert_eq!(
            map_to_vec::<String, facts::MethodFacts, compile_ffi::MethodDetails>(rust_methods),
            ffi_methods
        )
    }

    #[test]
    fn test_methods_2() {
        let (ffi_methods, mut rust_methods) = create_methods();
        rust_methods.clear();
        assert_ne!(vec_to_map(ffi_methods), rust_methods)
    }

    #[test]
    fn test_type_facts() {
        let (ffi_type_facts, rust_type_facts) = create_type_facts();
        assert_eq!(
            compile_ffi::TypeDetails::from(rust_type_facts),
            ffi_type_facts
        )
    }

    #[test]
    fn test_type_facts_by_name() {
        let (ffi_type_facts_by_name, rust_type_facts_by_name) = create_type_facts_by_name();
        assert_eq!(
            map_to_vec::<String, facts::TypeFacts, compile_ffi::TypeDetails>(
                rust_type_facts_by_name
            ),
            ffi_type_facts_by_name
        )
    }

    #[test]
    fn test_facts() {
        let (ffi_type_facts_by_name, rust_type_facts_by_name) = create_type_facts_by_name();
        let (ffi_module_facts_by_name, rust_module_facts_by_name) = create_module_facts_by_name();
        let (ffi_attributes, rust_attributes) = create_attributes();
        let ffi_facts = compile_ffi::FileFacts {
            types: ffi_type_facts_by_name,
            functions: vec!["f1".to_string(), "f2".to_string()],
            constants: vec!["C".to_string()],
            modules: ffi_module_facts_by_name,
            attributes: ffi_attributes,
            sha1hex: String::default(),
        };
        let rust_facts = facts::Facts {
            types: rust_type_facts_by_name,
            functions: vec!["f1".to_string(), "f2".to_string()],
            constants: vec!["C".to_string()],
            modules: rust_module_facts_by_name,
            file_attributes: rust_attributes,
        };
        assert_eq!(facts::Facts::from(ffi_facts), rust_facts)
    }

    fn create_attributes() -> (Vec<compile_ffi::Attribute>, facts::Attributes) {
        let ffi_attributes = vec![
            compile_ffi::Attribute {
                name: "MyAttribute1".to_string(),
                args: vec!["\"arg1\"".into(), "\"arg2\"".into(), "\"arg3\"".into()],
            },
            compile_ffi::Attribute {
                name: "MyAttribute2".to_string(),
                args: vec![],
            },
        ];

        let mut rust_attributes = BTreeMap::new();
        rust_attributes.insert(
            "MyAttribute1".to_string(),
            vec!["arg1".into(), "arg2".into(), "arg3".into()],
        );
        rust_attributes.insert("MyAttribute2".to_string(), vec![]);

        (ffi_attributes, rust_attributes)
    }

    fn create_method_facts() -> (compile_ffi::MethodDetails, facts::MethodFacts) {
        let (ffi_attributes, rust_attributes) = create_attributes();

        let ffi_method_facts = compile_ffi::MethodDetails {
            name: String::default(),
            attributes: ffi_attributes,
        };
        let rust_method_facts = facts::MethodFacts {
            attributes: rust_attributes,
        };

        (ffi_method_facts, rust_method_facts)
    }

    fn create_methods() -> (Vec<compile_ffi::MethodDetails>, facts::Methods) {
        let (ffi_method_facts, rust_method_facts) = create_method_facts();
        let ffi_methods = vec![compile_ffi::MethodDetails {
            name: "m".to_string(),
            ..ffi_method_facts
        }];
        let mut rust_methods = BTreeMap::new();
        rust_methods.insert("m".to_string(), rust_method_facts);

        (ffi_methods, rust_methods)
    }

    fn create_type_facts() -> (compile_ffi::TypeDetails, facts::TypeFacts) {
        let (ffi_attributes, rust_attributes) = create_attributes();
        let (ffi_methods, rust_methods) = create_methods();
        let base_types = vec!["int".to_string(), "string".to_string()];
        let require_extends = vec!["A".to_string()];
        let require_implements = vec!["B".to_string()];
        let require_class = vec!["D".to_string()];

        let rust_type_facts = facts::TypeFacts {
            base_types: vec_to_set(base_types.clone()),
            kind: facts::TypeKind::Class,
            attributes: rust_attributes,
            flags: 0,
            require_extends: vec_to_set(require_extends.clone()),
            require_implements: vec_to_set(require_implements.clone()),
            require_class: vec_to_set(require_class.clone()),
            methods: rust_methods,
        };

        let ffi_type_facts = compile_ffi::TypeDetails {
            name: String::default(),
            base_types,
            kind: compile_ffi::TypeKind::Class,
            attributes: ffi_attributes,
            flags: 0,
            require_extends,
            require_implements,
            require_class,
            methods: ffi_methods,
        };

        (ffi_type_facts, rust_type_facts)
    }

    fn create_type_facts_by_name() -> (Vec<compile_ffi::TypeDetails>, facts::TypeFactsByName) {
        let (ffi_type_facts, rust_type_facts) = create_type_facts();

        let ffi_type_facts_by_name = vec![compile_ffi::TypeDetails {
            name: "C".into(),
            ..ffi_type_facts
        }];

        let mut rust_type_facts_by_name = BTreeMap::new();
        rust_type_facts_by_name.insert("C".to_string(), rust_type_facts);

        (ffi_type_facts_by_name, rust_type_facts_by_name)
    }

    fn create_module_facts_by_name() -> (Vec<compile_ffi::ModuleDetails>, facts::ModuleFactsByName)
    {
        let ffi_module_facts_by_name = vec![compile_ffi::ModuleDetails {
            name: "mfoo".to_string(),
        }];

        let mut rust_module_facts_by_name = BTreeMap::new();
        rust_module_facts_by_name.insert("mfoo".to_string(), facts::ModuleFacts {});

        (ffi_module_facts_by_name, rust_module_facts_by_name)
    }
}
