// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use super::*;

impl From<facts::Facts> for compile_ffi::FileSymbols {
    fn from(facts: facts::Facts) -> Self {
        Self {
            types: facts.types.into_keys().collect(),
            functions: facts.functions,
            constants: facts.constants,
            modules: facts.modules.into_keys().collect(),
        }
    }
}

trait IntoKeyValue<K, V> {
    fn into_key_value(self) -> (K, V);
}

trait FromKeyValue<K, V> {
    fn from_key_value(k: K, v: V) -> Self;
}

#[cfg(test)]
mod tests {
    use std::collections::BTreeMap;

    use super::*;

    #[test]
    fn test_facts() {
        let (ffi_type_facts_by_name, rust_type_facts_by_name) = create_type_facts_by_name();
        let (ffi_module_facts_by_name, rust_module_facts_by_name) = create_module_facts_by_name();
        let ffi_facts = compile_ffi::FileSymbols {
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
        assert_eq!(compile_ffi::FileSymbols::from(rust_facts), ffi_facts)
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
}
