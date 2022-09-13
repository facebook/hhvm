// Copyright (c) 2019; Facebook; Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::BTreeMap;
use std::ffi::OsStr;
use std::os::unix::ffi::OsStrExt;
use std::path::Path;
use std::path::PathBuf;

use bitflags::bitflags;
use bstr::BString;
use bstr::ByteSlice;
use hhbc::IncludePath;
use oxidized::relative_path::RelativePath;

macro_rules! prefixed_flags {
    ($class:ident, $($field:ident),*,) => { // require trailing comma
        bitflags! {
            pub struct $class: u64 {
                // TODO(leoo) expand RHS this into 1 << i, using equivalent of C++ index_sequence
                $( const $field = Flags::$field.bits(); )*
            }
        }
    }
}

prefixed_flags!(
    CompilerFlags,
    CONSTANT_FOLDING,
    OPTIMIZE_NULL_CHECKS,
    RELABEL,
);
impl Default for CompilerFlags {
    fn default() -> CompilerFlags {
        CompilerFlags::CONSTANT_FOLDING | CompilerFlags::RELABEL
    }
}

prefixed_flags!(
    HhvmFlags,
    ARRAY_PROVENANCE,
    EMIT_CLS_METH_POINTERS,
    EMIT_METH_CALLER_FUNC_POINTERS,
    ENABLE_INTRINSICS_EXTENSION,
    FOLD_LAZY_CLASS_KEYS,
    JIT_ENABLE_RENAME_FUNCTION,
    LOG_EXTERN_COMPILER_PERF,
);
impl Default for HhvmFlags {
    fn default() -> HhvmFlags {
        HhvmFlags::EMIT_CLS_METH_POINTERS
            | HhvmFlags::EMIT_METH_CALLER_FUNC_POINTERS
            | HhvmFlags::FOLD_LAZY_CLASS_KEYS
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct Hhvm {
    pub aliased_namespaces: BTreeMap<String, String>,
    pub include_roots: BTreeMap<BString, BString>,
    pub emit_class_pointers: String,
    pub flags: HhvmFlags,
    pub hack_lang: HackLang,
}

impl Default for Hhvm {
    fn default() -> Self {
        Self {
            aliased_namespaces: Default::default(),
            include_roots: Default::default(),
            emit_class_pointers: "0".into(),
            flags: Default::default(),
            hack_lang: Default::default(),
        }
    }
}

impl Hhvm {
    pub fn aliased_namespaces_cloned(&self) -> impl Iterator<Item = (String, String)> + '_ {
        self.aliased_namespaces
            .iter()
            .map(|(k, v)| (k.clone(), v.clone()))
    }
}

#[derive(Clone, Debug, Default, PartialEq)]
pub struct HackLang {
    pub flags: LangFlags,
    pub check_int_overflow: String,
}

prefixed_flags!(
    LangFlags,
    ABSTRACT_STATIC_PROPS,
    ALLOW_NEW_ATTRIBUTE_SYNTAX,
    ALLOW_UNSTABLE_FEATURES,
    CONST_DEFAULT_FUNC_ARGS,
    CONST_DEFAULT_LAMBDA_ARGS,
    CONST_STATIC_PROPS,
    DISABLE_LEGACY_ATTRIBUTE_SYNTAX,
    DISABLE_LEGACY_SOFT_TYPEHINTS,
    DISABLE_LVAL_AS_AN_EXPRESSION,
    DISABLE_XHP_ELEMENT_MANGLING,
    DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS,
    DISALLOW_INST_METH,
    DISALLOW_FUNC_PTRS_IN_CONSTANTS,
    ENABLE_CLASS_LEVEL_WHERE_CLAUSES,
    ENABLE_ENUM_CLASSES,
    ENABLE_XHP_CLASS_MODIFIER,
    RUST_EMITTER,
);
impl Default for LangFlags {
    fn default() -> LangFlags {
        LangFlags::DISABLE_LEGACY_SOFT_TYPEHINTS | LangFlags::ENABLE_ENUM_CLASSES
    }
}

prefixed_flags!(
    Php7Flags, LTR_ASSIGN, // Left to right assignment
    UVS,        // uniform variable syntax
);
impl Default for Php7Flags {
    fn default() -> Php7Flags {
        Php7Flags::empty()
    }
}

prefixed_flags!(RepoFlags, AUTHORITATIVE,);
impl Default for RepoFlags {
    fn default() -> RepoFlags {
        RepoFlags::empty()
    }
}

#[derive(Clone, Default, PartialEq, Debug)]
pub struct Server {
    pub include_search_paths: Vec<BString>,
}

#[derive(Clone, PartialEq, Debug)]
pub struct Options {
    pub doc_root: BString,
    pub hack_compiler_flags: CompilerFlags,
    pub hhvm: Hhvm,
    pub max_array_elem_size_on_the_stack: usize,
    pub php7_flags: Php7Flags,
    pub repo_flags: RepoFlags,
    pub server: Server,
}

impl Options {
    pub fn log_extern_compiler_perf(&self) -> bool {
        self.hhvm
            .flags
            .contains(HhvmFlags::LOG_EXTERN_COMPILER_PERF)
    }
}

impl Default for Options {
    fn default() -> Options {
        Options {
            max_array_elem_size_on_the_stack: 64,
            hack_compiler_flags: CompilerFlags::default(),
            hhvm: Hhvm::default(),
            php7_flags: Php7Flags::default(),
            repo_flags: RepoFlags::default(),
            server: Server::default(),
            // the rest is zeroed out (cannot do ..Default::default() as it'd be recursive)
            doc_root: "".into(),
        }
    }
}

impl bytecode_printer::IncludeProcessor for Options {
    fn convert_include<'a>(
        &'a self,
        include_path: IncludePath<'a>,
        cur_path: Option<&'a RelativePath>,
    ) -> Option<PathBuf> {
        let alloc = bumpalo::Bump::new();
        let include_roots = &self.hhvm.include_roots;
        let search_paths = &self.server.include_search_paths;
        let doc_root = self.doc_root.as_bstr();
        match include_path.into_doc_root_relative(&alloc, include_roots) {
            IncludePath::Absolute(p) => {
                let path = Path::new(OsStr::from_bytes(&p));
                if path.exists() {
                    Some(path.to_owned())
                } else {
                    None
                }
            }
            IncludePath::SearchPathRelative(p) => {
                let path_from_cur_dirname = cur_path
                    .and_then(|p| p.path().parent())
                    .unwrap_or_else(|| Path::new(""))
                    .join(OsStr::from_bytes(&p));
                if path_from_cur_dirname.exists() {
                    Some(path_from_cur_dirname)
                } else {
                    for prefix in search_paths.iter() {
                        let path = Path::new(OsStr::from_bytes(prefix)).join(OsStr::from_bytes(&p));
                        if path.exists() {
                            return Some(path);
                        }
                    }
                    None
                }
            }
            IncludePath::IncludeRootRelative(v, p) => {
                if !p.is_empty() {
                    if let Some(ir) = include_roots.get(v.as_bstr()) {
                        let resolved = Path::new(OsStr::from_bytes(doc_root))
                            .join(OsStr::from_bytes(ir))
                            .join(OsStr::from_bytes(&p));
                        if resolved.exists() {
                            return Some(resolved);
                        }
                    }
                }
                None
            }
            IncludePath::DocRootRelative(p) => {
                let resolved = Path::new(OsStr::from_bytes(doc_root)).join(OsStr::from_bytes(&p));
                if resolved.exists() {
                    Some(resolved)
                } else {
                    None
                }
            }
        }
    }
}

impl Options {
    pub fn array_provenance(&self) -> bool {
        self.hhvm.flags.contains(HhvmFlags::ARRAY_PROVENANCE)
    }

    pub fn check_int_overflow(&self) -> bool {
        self.hhvm
            .hack_lang
            .check_int_overflow
            .parse::<i32>()
            .map_or(false, |x| x.is_positive())
    }

    pub fn emit_class_pointers(&self) -> i32 {
        self.hhvm.emit_class_pointers.parse::<i32>().unwrap()
    }
}

// boilerplate code that could eventually be avoided via procedural macros

bitflags! {
    struct Flags: u64 {
        const CONSTANT_FOLDING = 1 << 0;
        const OPTIMIZE_NULL_CHECKS = 1 << 1;
        // No longer using bit 2.
        const UVS = 1 << 3;
        const LTR_ASSIGN = 1 << 4;
        /// If true, then renumber labels after generating code for a method
        /// body. Semantic diff doesn't care about labels, but for visual diff against
        /// HHVM it's helpful to renumber in order that the labels match more closely
        const RELABEL = 1 << 5;
        // No longer using bit 6.
        // No longer using bit 7.
        // No longer using bit 8.
        const AUTHORITATIVE = 1 << 9;
        const JIT_ENABLE_RENAME_FUNCTION = 1 << 10;
        // No longer using bits 11-13.
        const LOG_EXTERN_COMPILER_PERF = 1 << 14;
        const ENABLE_INTRINSICS_EXTENSION = 1 << 15;
        // No longer using bits 16-22.
        // No longer using bits 23-25.
        const EMIT_CLS_METH_POINTERS = 1 << 26;
        // No longer using bit 27.
        const EMIT_METH_CALLER_FUNC_POINTERS = 1 << 28;
        // No longer using bit 29.
        const DISABLE_LVAL_AS_AN_EXPRESSION = 1 << 30;
        // No longer using bits 31-32.
        const ARRAY_PROVENANCE = 1 << 33;
        // No longer using bit 34.
        const ENABLE_CLASS_LEVEL_WHERE_CLAUSES = 1 << 35;
        const DISABLE_LEGACY_SOFT_TYPEHINTS = 1 << 36;
        const ALLOW_NEW_ATTRIBUTE_SYNTAX = 1 << 37;
        const DISABLE_LEGACY_ATTRIBUTE_SYNTAX = 1 << 38;
        const CONST_DEFAULT_FUNC_ARGS = 1 << 39;
        const CONST_STATIC_PROPS = 1 << 40;
        const ABSTRACT_STATIC_PROPS = 1 << 41;
        // No longer using bit 42
        const DISALLOW_FUNC_PTRS_IN_CONSTANTS = 1 << 43;
        // No longer using bit 44.
        const CONST_DEFAULT_LAMBDA_ARGS = 1 << 45;
        const ENABLE_XHP_CLASS_MODIFIER = 1 << 46;
        // No longer using bit 47.
        const ENABLE_ENUM_CLASSES = 1 << 48;
        const DISABLE_XHP_ELEMENT_MANGLING = 1 << 49;
        // No longer using bit 50.
        const RUST_EMITTER = 1 << 51;
        // No longer using bits 52-54.
        const ALLOW_UNSTABLE_FEATURES = 1 << 55;
        // No longer using bit 56.
        const DISALLOW_FUN_AND_CLS_METH_PSEUDO_FUNCS = 1 << 57;
        const FOLD_LAZY_CLASS_KEYS = 1 << 58;
        // No longer using bit 59.
        const DISALLOW_INST_METH = 1 << 60;
    }
}
