// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_iterator::Iter;
use hhbc_by_ref_options::Options;

use hhbc_by_ref_adata_state::AdataState;
use hhbc_by_ref_global_state::GlobalState;
use hhbc_by_ref_statement_state::StatementState;
use hhbc_by_ref_symbol_refs_state::SymbolRefsState;
use unified_decl_provider::DeclProvider;

#[derive(Debug)]
pub struct Emitter<'arena, 'decl> {
    /// Options are frozen/const after emitter is constructed
    opts: Options,
    /// systemlib is part of context, changed externally
    systemlib: bool,
    // the rest is being mutated during emittance
    label_gen: hhbc_by_ref_label::Gen,
    local_gen: hhbc_by_ref_local::Gen<'arena>,
    iterator: Iter,

    pub for_debugger_eval: bool,

    pub adata_state_: Option<AdataState<'arena>>,
    pub statement_state_: Option<StatementState<'arena>>,
    pub symbol_refs_state_: Option<SymbolRefsState<'arena>>,
    /// State is also frozen and set after closure conversion
    pub global_state_: Option<GlobalState<'arena>>,

    pub decl_provider: DeclProvider<'decl>,

    __phaton_data: std::marker::PhantomData<&'decl ()>,
}

impl<'arena, 'decl> Emitter<'arena, 'decl> {
    pub fn new(
        opts: Options,
        systemlib: bool,
        for_debugger_eval: bool,
        decl_provider: unified_decl_provider::DeclProvider<'decl>,
    ) -> Emitter<'arena, 'decl> {
        Emitter {
            opts,
            systemlib,
            for_debugger_eval,
            decl_provider,

            label_gen: Default::default(),
            local_gen: Default::default(),
            iterator: Default::default(),

            adata_state_: None,
            statement_state_: None,
            symbol_refs_state_: None,
            global_state_: None,

            __phaton_data: std::marker::PhantomData,
        }
    }

    pub fn decl_provider(&self) -> &unified_decl_provider::DeclProvider<'decl> {
        &self.decl_provider
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

    pub fn label_gen_mut(&mut self) -> &mut hhbc_by_ref_label::Gen {
        &mut self.label_gen
    }

    pub fn local_gen_mut(&mut self) -> &mut hhbc_by_ref_local::Gen<'arena> {
        &mut self.local_gen
    }

    pub fn local_gen(&self) -> &hhbc_by_ref_local::Gen<'arena> {
        &self.local_gen
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
    pub fn emit_adata_state_mut(
        &mut self,
        alloc: &'arena bumpalo::Bump,
    ) -> &mut AdataState<'arena> {
        self.adata_state_
            .get_or_insert_with(|| AdataState::init(alloc))
    }

    pub fn emit_statement_state(&self) -> &StatementState<'arena> {
        self.statement_state_
            .as_ref()
            .expect("uninit'd statement_state")
    }
    pub fn emit_statement_state_mut(
        &mut self,
        alloc: &'arena bumpalo::Bump,
    ) -> &mut StatementState<'arena> {
        self.statement_state_
            .get_or_insert_with(|| StatementState::init(alloc))
    }

    pub fn emit_symbol_refs_state(&self) -> &SymbolRefsState<'arena> {
        self.symbol_refs_state_
            .as_ref()
            .expect("uninit'd symbol_refs_state")
    }
    pub fn emit_symbol_refs_state_mut(
        &mut self,
        alloc: &'arena bumpalo::Bump,
    ) -> &mut SymbolRefsState<'arena> {
        self.symbol_refs_state_
            .get_or_insert_with(|| SymbolRefsState::init(alloc))
    }

    pub fn emit_global_state(&self) -> &GlobalState<'arena> {
        self.global_state_.as_ref().expect("uninit'd global_state")
    }
    pub fn emit_global_state_mut(&mut self) -> &mut GlobalState<'arena> {
        self.global_state_.get_or_insert_with(GlobalState::init)
    }
}
