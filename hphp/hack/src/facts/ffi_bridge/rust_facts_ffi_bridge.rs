// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

use cxx::CxxString;
use facts_rust::facts;
use oxidized::relative_path::RelativePath;
use rust_facts_ffi::{extract_facts_as_json_ffi0, extract_facts_ffi0};
use std::collections::{BTreeMap, BTreeSet};

#[cxx::bridge]
mod ffi {
    #[derive(Debug)]
    enum TypeKind {
        Class,
        Record,
        Interface,
        Enum,
        Trait,
        TypeAlias,
        Unknown,
        Mixed,
    }

    #[derive(Debug)]
    struct Attribute {
        name: String,
        args: Vec<String>,
    }

    #[derive(Debug)]
    struct MethodFacts {
        attributes: Vec<Attribute>,
    }

    #[derive(Debug)]
    struct Method {
        name: String,
        methfacts: MethodFacts,
    }

    #[derive(Debug)]
    pub struct TypeFacts {
        pub base_types: Vec<String>,
        pub kind: TypeKind,
        pub attributes: Vec<Attribute>,
        pub flags: isize,
        pub require_extends: Vec<String>,
        pub require_implements: Vec<String>,
        pub methods: Vec<Method>,
    }

    #[derive(Debug)]
    struct TypeFactsByName {
        name: String,
        typefacts: TypeFacts,
    }

    #[derive(Debug, Default)]
    struct Facts {
        pub types: Vec<TypeFactsByName>,
        pub functions: Vec<String>,
        pub constants: Vec<String>,
        pub type_aliases: Vec<String>,
        pub file_attributes: Vec<Attribute>,
    }

    #[derive(Debug, Default)]
    struct FactsResult {
        facts: Facts,
        md5sum: String,
        sha1sum: String,
    }

    extern "Rust" {
        pub fn hackc_extract_facts_as_json_cpp_ffi(
            flags: i32,
            filename: &CxxString,
            source_text: &CxxString,
        ) -> String;
    }

    extern "Rust" {
        pub fn hackc_extract_facts_cpp_ffi(
            flags: i32,
            filename: &CxxString,
            source_text: &CxxString,
        ) -> FactsResult;
    }
}

pub fn hackc_extract_facts_as_json_cpp_ffi(
    flags: i32,
    filename: &CxxString,
    source_text: &CxxString,
) -> String {
    use std::os::unix::ffi::OsStrExt;
    let filepath = RelativePath::make(
        oxidized::relative_path::Prefix::Dummy,
        std::path::PathBuf::from(std::ffi::OsStr::from_bytes(filename.as_bytes())),
    );
    match extract_facts_as_json_ffi0(
        ((1 << 0) & flags) != 0, // php5_compat_mode
        ((1 << 1) & flags) != 0, // hhvm_compat_mode
        ((1 << 2) & flags) != 0, // allow_new_attribute_syntax
        ((1 << 3) & flags) != 0, // enable_xhp_class_modifier
        ((1 << 4) & flags) != 0, // disable_xhp_element_mangling
        ((1 << 5) & flags) != 0, // disallow_hash_comments
        filepath,
        source_text.as_bytes(),
        true, // mangle_xhp
    ) {
        Some(s) => s,
        None => String::new(),
    }
}

pub fn hackc_extract_facts_cpp_ffi(
    flags: i32,
    filename: &CxxString,
    source_text: &CxxString,
) -> ffi::FactsResult {
    use std::os::unix::ffi::OsStrExt;
    let filepath = RelativePath::make(
        oxidized::relative_path::Prefix::Dummy,
        std::path::PathBuf::from(std::ffi::OsStr::from_bytes(filename.as_bytes())),
    );
    let text = source_text.as_bytes();
    match extract_facts_ffi0(
        ((1 << 0) & flags) != 0, // php5_compat_mode
        ((1 << 1) & flags) != 0, // hhvm_compat_mode
        ((1 << 2) & flags) != 0, // allow_new_attribute_syntax
        ((1 << 3) & flags) != 0, // enable_xhp_class_modifier
        ((1 << 4) & flags) != 0, // disable_xhp_element_mangling
        ((1 << 5) & flags) != 0, // disallow_hash_comments
        filepath,
        text,
        true, // mangle_xhp
    ) {
        Some(facts) => {
            let (md5sum, sha1sum) = facts::md5_and_sha1(text);
            ffi::FactsResult {
                facts: facts.into(),
                md5sum,
                sha1sum,
            }
        }
        None => Default::default(),
    }
}

fn vec_to_map<K, V, T>(v: Vec<T>) -> BTreeMap<K, V>
where
    K: std::cmp::Ord,
    T: Into<(K, V)>,
{
    v.into_iter().map(|x| x.into()).collect::<BTreeMap<K, V>>()
}

fn map_to_vec<K, V, T>(m: BTreeMap<K, V>) -> Vec<T>
where
    T: From<(K, V)>,
{
    m.into_iter()
        .map(|(k, v)| T::from((k, v)))
        .collect::<Vec<T>>()
}

fn vec_to_set<T: std::cmp::Ord>(v: Vec<T>) -> BTreeSet<T> {
    v.into_iter().map(|item| item).collect()
}

fn set_to_vec<T>(s: BTreeSet<T>) -> Vec<T> {
    s.into_iter().map(|item| item).collect()
}

impl From<ffi::TypeKind> for facts::TypeKind {
    fn from(type_kind: ffi::TypeKind) -> facts::TypeKind {
        match type_kind {
            ffi::TypeKind::Class => facts::TypeKind::Class,
            ffi::TypeKind::Record => facts::TypeKind::Record,
            ffi::TypeKind::Interface => facts::TypeKind::Interface,
            ffi::TypeKind::Enum => facts::TypeKind::Enum,
            ffi::TypeKind::Trait => facts::TypeKind::Trait,
            ffi::TypeKind::TypeAlias => facts::TypeKind::TypeAlias,
            ffi::TypeKind::Unknown => facts::TypeKind::Unknown,
            ffi::TypeKind::Mixed => facts::TypeKind::Mixed,
            _ => panic!("impossible"),
        }
    }
}
impl From<facts::TypeKind> for ffi::TypeKind {
    fn from(typekind: facts::TypeKind) -> ffi::TypeKind {
        match typekind {
            facts::TypeKind::Class => ffi::TypeKind::Class,
            facts::TypeKind::Record => ffi::TypeKind::Record,
            facts::TypeKind::Interface => ffi::TypeKind::Interface,
            facts::TypeKind::Enum => ffi::TypeKind::Enum,
            facts::TypeKind::Trait => ffi::TypeKind::Trait,
            facts::TypeKind::TypeAlias => ffi::TypeKind::TypeAlias,
            facts::TypeKind::Unknown => ffi::TypeKind::Unknown,
            facts::TypeKind::Mixed => ffi::TypeKind::Mixed,
        }
    }
}

impl From<ffi::Attribute> for (String, Vec<String>) {
    fn from(attr: ffi::Attribute) -> (String, Vec<String>) {
        (attr.name, attr.args)
    }
}
impl From<(String, Vec<String>)> for ffi::Attribute {
    fn from(attr: (String, Vec<String>)) -> ffi::Attribute {
        ffi::Attribute {
            name: attr.0,
            args: attr.1,
        }
    }
}

impl From<ffi::Method> for (String, facts::MethodFacts) {
    fn from(meth: ffi::Method) -> (String, facts::MethodFacts) {
        let ffi::Method { name, methfacts } = meth;
        (name, methfacts.into())
    }
}
impl From<(String, facts::MethodFacts)> for ffi::Method {
    fn from(methodfacts: (String, facts::MethodFacts)) -> ffi::Method {
        ffi::Method {
            name: methodfacts.0.into(),
            methfacts: methodfacts.1.into(),
        }
    }
}

impl From<ffi::MethodFacts> for facts::MethodFacts {
    fn from(methodfacts: ffi::MethodFacts) -> facts::MethodFacts {
        facts::MethodFacts {
            attributes: vec_to_map(methodfacts.attributes),
        }
    }
}
impl From<facts::MethodFacts> for ffi::MethodFacts {
    fn from(method_facts: facts::MethodFacts) -> ffi::MethodFacts {
        ffi::MethodFacts {
            attributes: map_to_vec(method_facts.attributes),
        }
    }
}

impl From<ffi::TypeFacts> for facts::TypeFacts {
    fn from(facts: ffi::TypeFacts) -> facts::TypeFacts {
        facts::TypeFacts {
            base_types: vec_to_set(facts.base_types),
            kind: facts.kind.into(),
            attributes: vec_to_map(facts.attributes),
            flags: facts.flags,
            require_extends: vec_to_set(facts.require_extends),
            require_implements: vec_to_set(facts.require_implements),
            methods: vec_to_map(facts.methods),
        }
    }
}
impl From<facts::TypeFacts> for ffi::TypeFacts {
    fn from(facts: facts::TypeFacts) -> ffi::TypeFacts {
        ffi::TypeFacts {
            base_types: set_to_vec(facts.base_types),
            kind: facts.kind.into(),
            attributes: map_to_vec(facts.attributes),
            flags: facts.flags,
            require_extends: set_to_vec(facts.require_extends),
            require_implements: set_to_vec(facts.require_implements),
            methods: map_to_vec(facts.methods),
        }
    }
}

impl From<ffi::TypeFactsByName> for (String, facts::TypeFacts) {
    fn from(typefacts_by_name: ffi::TypeFactsByName) -> (String, facts::TypeFacts) {
        let ffi::TypeFactsByName { name, typefacts } = typefacts_by_name;
        (name, typefacts.into())
    }
}
impl From<(String, facts::TypeFacts)> for ffi::TypeFactsByName {
    fn from((name, typefacts): (String, facts::TypeFacts)) -> ffi::TypeFactsByName {
        ffi::TypeFactsByName {
            name,
            typefacts: typefacts.into(),
        }
    }
}

impl From<ffi::Facts> for facts::Facts {
    fn from(facts: ffi::Facts) -> facts::Facts {
        facts::Facts {
            types: vec_to_map(facts.types),
            functions: facts.functions,
            constants: facts.constants,
            type_aliases: facts.type_aliases,
            file_attributes: vec_to_map(facts.file_attributes),
        }
    }
}
impl From<facts::Facts> for ffi::Facts {
    fn from(facts: facts::Facts) -> ffi::Facts {
        ffi::Facts {
            types: map_to_vec(facts.types),
            functions: facts.functions,
            constants: facts.constants,
            type_aliases: facts.type_aliases,
            file_attributes: map_to_vec(facts.file_attributes),
        }
    }
}
