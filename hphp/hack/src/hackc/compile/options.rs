// Copyright (c) 2019; Facebook; Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::BTreeMap;

#[cfg(fbcode_build)]
mod options_gen;
#[cfg(not(fbcode_build))]
mod options_gen {
    include!(concat!(env!("OUT_DIR"), "/options_gen.rs"));
}

use bstr::BString;
pub use options_gen::HhbcFlags;
pub use options_gen::ParserFlags;
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
}

impl Hhvm {
    pub fn aliased_namespaces_cloned(&self) -> impl Iterator<Item = (String, String)> + '_ {
        self.parser_options.auto_namespace_map.iter().cloned()
    }
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
