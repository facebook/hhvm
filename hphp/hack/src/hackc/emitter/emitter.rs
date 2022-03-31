// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use adata_state::AdataState;
use decl_provider::{DeclProvider, Result};
use ffi::Str;
use global_state::GlobalState;
use hash::IndexSet;
use iterator::Iter;
use local::Local;
use options::Options;
use oxidized_by_ref::{file_info::NameType, shallow_decl_defs::Decl};
use stack_limit::StackLimit;
use statement_state::StatementState;
use symbol_refs_state::SymbolRefsState;

#[derive(Debug)]
pub struct Emitter<'arena, 'decl> {
    /// Options are frozen/const after emitter is constructed
    opts: Options,

    /// systemlib is part of context, changed externally
    systemlib: bool,

    // the rest is being mutated during emittance
    label_gen: label::Gen,
    local_gen: local::Gen,
    iterator: Iter,
    named_locals: IndexSet<Str<'arena>>,

    pub for_debugger_eval: bool,

    pub alloc: &'arena bumpalo::Bump,

    pub adata_state_: Option<AdataState<'arena>>,
    pub statement_state_: Option<StatementState<'arena>>,
    pub symbol_refs_state_: Option<SymbolRefsState<'arena>>,
    /// State is also frozen and set after closure conversion
    pub global_state_: Option<GlobalState<'arena>>,

    /// Controls whether we call the decl provider for testing purposes.
    /// Some(provider) => use the given DeclProvider, which may return NotFound.
    /// None => do not look up any decls. For now this is the same as as a
    /// DeclProvider that always returns NotFound, but this behavior may later
    /// diverge from None provider behavior.
    decl_provider: Option<&'decl dyn DeclProvider<'decl>>,

    /// For checking elastic_stack and restarting if necessary.
    pub stack_limit: Option<&'decl StackLimit>,
}

impl<'arena, 'decl> Emitter<'arena, 'decl> {
    pub fn new(
        opts: Options,
        systemlib: bool,
        for_debugger_eval: bool,
        alloc: &'arena bumpalo::Bump,
        decl_provider: Option<&'decl dyn DeclProvider<'decl>>,
        stack_limit: Option<&'decl StackLimit>,
    ) -> Emitter<'arena, 'decl> {
        Emitter {
            opts,
            systemlib,
            for_debugger_eval,
            decl_provider,
            alloc,

            label_gen: Default::default(),
            local_gen: Default::default(),
            iterator: Default::default(),
            named_locals: Default::default(),

            adata_state_: None,
            statement_state_: None,
            symbol_refs_state_: None,
            global_state_: None,

            stack_limit,
        }
    }

    pub fn decl_provider(&self) -> Option<&'decl dyn DeclProvider<'decl>> {
        self.decl_provider
    }

    pub fn options(&self) -> &Options {
        &self.opts
    }

    /// Destruct the emitter but salvage its options (for use in emitting fatal program).
    pub fn into_options(self) -> Options {
        self.opts
    }

    pub fn iterator(&self) -> &Iter {
        &self.iterator
    }

    pub fn iterator_mut(&mut self) -> &mut Iter {
        &mut self.iterator
    }

    pub fn label_gen_mut(&mut self) -> &mut label::Gen {
        &mut self.label_gen
    }

    pub fn local_gen_mut(&mut self) -> &mut local::Gen {
        &mut self.local_gen
    }

    pub fn local_gen(&self) -> &local::Gen {
        &self.local_gen
    }

    /// Initialize the named locals table. Canonical HHBC numbering
    /// puts the parameters first in left-to-right order, then local varables
    /// that have names from the source text. In HHAS those names must appear
    /// in the `.decl_vars` directive.
    pub fn init_named_locals(&mut self, names: impl IntoIterator<Item = Str<'arena>>) {
        assert!(self.named_locals.is_empty());
        self.named_locals = names.into_iter().collect();
    }

    pub fn clear_named_locals(&mut self) {
        self.named_locals = Default::default();
    }

    /// Given a name, return corresponding local. Panic if the local is unknown,
    /// indicating a logic bug in the compiler; all params and named locals must
    /// be provided in advance to init_named_locals().
    pub fn named_local(&self, name: Str<'_>) -> Local {
        match self.named_locals.get_index_of(&name).map(Local::new) {
            Some(local) => local,
            None => panic!(
                "{}: local not found among {:#?}",
                name.unsafe_as_str(),
                self.named_locals
                    .iter()
                    .map(|name| name.unsafe_as_str())
                    .collect::<Vec<_>>()
            ),
        }
    }

    /// Given a named local, return its name. Panic for unnamed locals
    /// indicating a logic bug in the compiler.
    pub fn local_name(&self, local: Local) -> &Str<'_> {
        self.named_locals.get_index(local.idx as usize).unwrap()
    }

    pub fn local_scope<R, F: FnOnce(&mut Self) -> R>(&mut self, f: F) -> R {
        let counter = self.local_gen.counter;
        self.local_gen.dedicated.temp_map.push();
        let r = f(self);
        self.local_gen.counter = counter;
        self.local_gen.dedicated.temp_map.pop();
        r
    }

    pub fn systemlib(&self) -> bool {
        self.systemlib
    }

    pub fn emit_adata_state(&self) -> &AdataState<'arena> {
        self.adata_state_.as_ref().expect("uninit'd adata_state")
    }
    pub fn emit_adata_state_mut(&mut self) -> &mut AdataState<'arena> {
        self.adata_state_.get_or_insert_with(Default::default)
    }

    pub fn emit_statement_state(&self) -> &StatementState<'arena> {
        self.statement_state_
            .as_ref()
            .expect("uninit'd statement_state")
    }
    pub fn emit_statement_state_mut(&mut self) -> &mut StatementState<'arena> {
        self.statement_state_
            .get_or_insert_with(StatementState::init)
    }

    pub fn emit_symbol_refs_state(&self) -> &SymbolRefsState<'arena> {
        self.symbol_refs_state_
            .as_ref()
            .expect("uninit'd symbol_refs_state")
    }
    pub fn emit_symbol_refs_state_mut(&mut self) -> &mut SymbolRefsState<'arena> {
        self.symbol_refs_state_
            .get_or_insert_with(|| SymbolRefsState::init(self.alloc))
    }

    pub fn emit_global_state(&self) -> &GlobalState<'arena> {
        self.global_state_.as_ref().expect("uninit'd global_state")
    }
    pub fn emit_global_state_mut(&mut self) -> &mut GlobalState<'arena> {
        self.global_state_.get_or_insert_with(GlobalState::init)
    }

    pub fn get_decl(&self, kind: NameType, sym: &str) -> Result<Decl<'decl>> {
        match self.decl_provider {
            Some(provider) => provider.get_decl(kind, sym),
            None => Err(decl_provider::Error::NotFound),
        }
    }
}
