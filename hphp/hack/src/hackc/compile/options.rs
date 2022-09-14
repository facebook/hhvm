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

use bstr::BString;
use bstr::ByteSlice;
use hhbc::IncludePath;
pub use oxidized::parser_options::ParserOptions;
use oxidized::relative_path::RelativePath;

#[derive(Debug, Clone, PartialEq)]
pub struct CompilerFlags {
    pub constant_folding: bool,
    pub optimize_null_checks: bool,
    pub relabel: bool,
}

impl Default for CompilerFlags {
    fn default() -> Self {
        Self {
            constant_folding: true,
            optimize_null_checks: false,
            relabel: true,
        }
    }
}

#[derive(Clone, Debug, Default, PartialEq)]
pub struct Hhvm {
    pub include_roots: BTreeMap<BString, BString>,
    pub emit_class_pointers: i32,
    pub check_int_overflow: i32,
    pub parser_options: ParserOptions,
}

impl Hhvm {
    pub fn aliased_namespaces_cloned(&self) -> impl Iterator<Item = (String, String)> + '_ {
        self.parser_options.po_auto_namespace_map.iter().cloned()
    }
}

#[derive(Clone, PartialEq, Debug)]
pub struct Options {
    pub doc_root: BString,
    pub compiler_flags: CompilerFlags,
    pub hhvm: Hhvm,
    pub hhbc: HhbcFlags,
    pub max_array_elem_size_on_the_stack: usize,
    pub include_search_paths: Vec<BString>,
}

impl Options {
    pub fn log_extern_compiler_perf(&self) -> bool {
        self.hhbc.log_extern_compiler_perf
    }
}

impl Default for Options {
    fn default() -> Options {
        Options {
            max_array_elem_size_on_the_stack: 64,
            compiler_flags: CompilerFlags::default(),
            hhvm: Hhvm::default(),
            hhbc: HhbcFlags::default(),
            include_search_paths: Default::default(),
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
        let search_paths = &self.include_search_paths;
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
        self.hhbc.array_provenance
    }

    pub fn check_int_overflow(&self) -> bool {
        self.hhvm.check_int_overflow > 0
    }

    pub fn emit_class_pointers(&self) -> i32 {
        self.hhvm.emit_class_pointers
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct HhbcFlags {
    /// PHP7 left-to-right assignment semantics
    pub ltr_assign: bool,

    /// PHP7 Uniform Variable Syntax
    pub uvs: bool,

    pub repo_authoritative: bool,
    pub jit_enable_rename_function: bool,
    pub log_extern_compiler_perf: bool,
    pub enable_intrinsics_extension: bool,
    pub emit_cls_meth_pointers: bool,
    pub emit_meth_caller_func_pointers: bool,
    pub array_provenance: bool,
    pub fold_lazy_class_keys: bool,
}

impl Default for HhbcFlags {
    fn default() -> Self {
        Self {
            ltr_assign: false,
            uvs: false,
            repo_authoritative: false,
            jit_enable_rename_function: false,
            log_extern_compiler_perf: false,
            enable_intrinsics_extension: false,
            emit_cls_meth_pointers: true,
            emit_meth_caller_func_pointers: true,
            array_provenance: false,
            fold_lazy_class_keys: true,
        }
    }
}
