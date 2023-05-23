// Copyright (c) 2019; Facebook; Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::BTreeMap;

use bstr::BString;
pub use oxidized::parser_options::ParserOptions;
use serde::Deserialize;
use serde::Serialize;

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

#[derive(Clone, Debug, Default, PartialEq, Serialize, Deserialize)]
pub struct Hhvm {
    pub include_roots: BTreeMap<BString, BString>,
    pub parser_options: ParserOptions,
    pub jit_enable_rename_function: JitEnableRenameFunction,
}

impl Hhvm {
    pub fn aliased_namespaces_cloned(&self) -> impl Iterator<Item = (String, String)> + '_ {
        self.parser_options.po_auto_namespace_map.iter().cloned()
    }
}

#[derive(Clone, Debug, Default, PartialEq, Serialize, Deserialize)]
pub enum JitEnableRenameFunction {
    #[default]
    Disable,
    Enable,
    RestrictedEnable,
}

#[derive(Clone, PartialEq, Debug)]
pub struct Options {
    pub compiler_flags: CompilerFlags,
    pub hhvm: Hhvm,
    pub hhbc: HhbcFlags,
    pub max_array_elem_size_on_the_stack: usize,
}

impl Options {
    pub fn log_extern_compiler_perf(&self) -> bool {
        self.hhbc.log_extern_compiler_perf
    }
    pub fn builtin_is_renamable(&self) -> bool {
        match self.hhvm.jit_enable_rename_function {
            JitEnableRenameFunction::Enable => true,
            JitEnableRenameFunction::RestrictedEnable | JitEnableRenameFunction::Disable => false,
        }
    }
}

impl Default for Options {
    fn default() -> Options {
        Options {
            max_array_elem_size_on_the_stack: 64,
            compiler_flags: CompilerFlags::default(),
            hhvm: Hhvm::default(),
            hhbc: HhbcFlags::default(),
        }
    }
}

impl Options {
    pub fn array_provenance(&self) -> bool {
        self.hhbc.array_provenance
    }
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct HhbcFlags {
    /// PHP7 left-to-right assignment semantics
    pub ltr_assign: bool,

    /// PHP7 Uniform Variable Syntax
    pub uvs: bool,

    pub repo_authoritative: bool,
    pub log_extern_compiler_perf: bool,
    pub enable_intrinsics_extension: bool,
    pub emit_cls_meth_pointers: bool,
    pub emit_meth_caller_func_pointers: bool,
    pub array_provenance: bool,
    pub fold_lazy_class_keys: bool,
    pub readonly_nonlocal_infer: bool,
    pub optimize_reified_param_checks: bool,
    pub stress_shallow_decl_deps: bool,
    pub stress_folded_decl_deps: bool,
}

impl Default for HhbcFlags {
    fn default() -> Self {
        Self {
            ltr_assign: false,
            uvs: false,
            repo_authoritative: false,
            log_extern_compiler_perf: false,
            enable_intrinsics_extension: false,
            emit_cls_meth_pointers: true,
            emit_meth_caller_func_pointers: true,
            array_provenance: false,
            fold_lazy_class_keys: true,
            readonly_nonlocal_infer: false,
            optimize_reified_param_checks: false,
            stress_shallow_decl_deps: false,
            stress_folded_decl_deps: false,
        }
    }
}
