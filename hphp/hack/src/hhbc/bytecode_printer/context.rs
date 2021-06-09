// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;

use crate::special_class_resolver::SpecialClassResolver;
use crate::write::*;
use decl_provider::DeclProvider;
use hhbc_by_ref_env::emitter::Emitter;
use oxidized::relative_path::RelativePath;

/// Indent is an abstraction of indentation. Configurable indentation
/// and perf tweaking will be easier.
struct Indent(usize);

impl Indent {
    pub fn new() -> Self {
        Self(0)
    }

    pub fn inc(&mut self) {
        self.0 += 1;
    }

    pub fn dec(&mut self) {
        self.0 -= 1;
    }

    pub fn write<W: Write>(&self, w: &mut W) -> Result<(), W::Error> {
        Ok(for _ in 0..self.0 {
            w.write("  ")?;
        })
    }
}

pub struct Context<'a> {
    pub special_class_resolver: &'a dyn SpecialClassResolver,
    pub path: Option<&'a RelativePath>,

    dump_symbol_refs: bool,
    pub dump_lambdas: bool,
    indent: Indent,
    is_system_lib: bool,

    pub include_roots: &'a BTreeMap<String, String>,
    pub include_search_paths: &'a [String],
    pub doc_root: &'a str,
    pub array_provenance: bool,
}

impl<'a> Context<'a> {
    pub fn new<'decl, D: DeclProvider<'decl>>(
        emitter: &'a Emitter<'a, 'decl, D>,
        path: Option<&'a RelativePath>,
        dump_symbol_refs: bool,
        is_system_lib: bool,
    ) -> Self {
        let include_roots = emitter.options().hhvm.include_roots.get();
        let include_search_paths = emitter.options().server.include_search_paths.get();
        let doc_root = emitter.options().doc_root.get();
        let array_provenance = emitter.options().array_provenance();

        Self {
            special_class_resolver: emitter,
            path,
            dump_symbol_refs,
            dump_lambdas: false,
            indent: Indent::new(),
            is_system_lib,

            include_roots,
            include_search_paths,
            doc_root,
            array_provenance,
        }
    }

    pub fn dump_symbol_refs(&self) -> bool {
        self.dump_symbol_refs
    }

    /// Insert a newline with indentation
    pub fn newline<W: Write>(&self, w: &mut W) -> Result<(), W::Error> {
        newline(w)?;
        self.indent.write(w)
    }

    /// Start a new indented block
    pub fn block<W, F>(&mut self, w: &mut W, f: F) -> Result<(), W::Error>
    where
        W: Write,
        F: FnOnce(&mut Self, &mut W) -> Result<(), W::Error>,
    {
        self.indent.inc();
        let r = f(self, w);
        self.indent.dec();
        r
    }

    pub fn unblock<W, F>(&mut self, w: &mut W, f: F) -> Result<(), W::Error>
    where
        W: Write,
        F: FnOnce(&mut Self, &mut W) -> Result<(), W::Error>,
    {
        self.indent.dec();
        let r = f(self, w);
        self.indent.inc();
        r
    }

    /// Printing instruction list requies manually control indentation,
    /// where indent_inc/indent_dec are called
    pub fn indent_inc(&mut self) {
        self.indent.inc();
    }

    pub fn indent_dec(&mut self) {
        self.indent.dec();
    }

    pub fn is_system_lib(&self) -> bool {
        self.is_system_lib
    }
}
