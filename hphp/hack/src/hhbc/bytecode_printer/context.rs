// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{special_class_resolver::SpecialClassResolver, write::newline};
use hhbc_by_ref_env::emitter::Emitter;
use oxidized::relative_path::RelativePath;
use std::collections::BTreeMap;
use std::io::{Result, Write};

/// Indent is an abstraction of indentation. Configurable indentation
/// and perf tweaking will be easier.
#[derive(Clone)]
struct Indent(usize);

impl Indent {
    fn new() -> Self {
        Self(0)
    }

    fn inc(&mut self) {
        self.0 += 1;
    }

    fn dec(&mut self) {
        self.0 -= 1;
    }

    fn write<W: Write>(&self, w: &mut W) -> Result<()> {
        for _ in 0..self.0 {
            w.write_all(b"  ")?;
        }
        Ok(())
    }
}

#[derive(Clone)]
pub struct Context<'a> {
    pub(crate) special_class_resolver: &'a dyn SpecialClassResolver,
    pub(crate) path: Option<&'a RelativePath>,

    dump_symbol_refs: bool,
    pub(crate) dump_lambdas: bool,
    indent: Indent,
    is_system_lib: bool,

    pub(crate) include_roots: &'a BTreeMap<String, String>,
    pub(crate) include_search_paths: &'a [String],
    pub(crate) doc_root: &'a str,
    pub(crate) array_provenance: bool,
}

impl<'a> Context<'a> {
    pub fn new<'arena, 'decl>(
        emitter: &'a Emitter<'arena, 'decl>,
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

    pub(crate) fn dump_symbol_refs(&self) -> bool {
        self.dump_symbol_refs
    }

    /// Insert a newline with indentation
    pub(crate) fn newline<W: Write>(&self, w: &mut W) -> Result<()> {
        newline(w)?;
        self.indent.write(w)
    }

    /// Start a new indented block
    pub(crate) fn block<W, F>(&self, w: &mut W, f: F) -> Result<()>
    where
        W: Write,
        F: FnOnce(&Self, &mut W) -> Result<()>,
    {
        let mut ctx = self.clone();
        ctx.indent.inc();
        f(&ctx, w)
    }

    pub(crate) fn unblock<W, F>(&self, w: &mut W, f: F) -> Result<()>
    where
        W: Write,
        F: FnOnce(&Self, &mut W) -> Result<()>,
    {
        let mut ctx = self.clone();
        ctx.indent.dec();
        f(&ctx, w)
    }

    /// Printing instruction list requies manually control indentation,
    /// where indent_inc/indent_dec are called
    pub(crate) fn indent_inc(&mut self) {
        self.indent.inc();
    }

    pub(crate) fn indent_dec(&mut self) {
        self.indent.dec();
    }

    pub(crate) fn is_system_lib(&self) -> bool {
        self.is_system_lib
    }
}
