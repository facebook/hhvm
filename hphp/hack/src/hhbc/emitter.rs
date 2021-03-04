// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use label_rust as label;
use options::Options;

use iterator::Iter;

use adata_state::AdataState;
use global_state::GlobalState;
use statement_state::StatementState;
use symbol_refs_state::SymbolRefsState;

#[derive(Debug, Default)]
pub struct Emitter {
    /// Options are frozen/const after emitter is constructed
    opts: Options,
    /// systemlib is part of context, changed externally
    systemlib: bool,
    // the rest is being mutated during emittance
    label_gen: label::Gen,
    local_gen: local::Gen,
    iterator: Iter,

    pub for_debugger_eval: bool,

    pub adata_state_: Option<AdataState>,
    pub statement_state_: Option<StatementState>,
    pub symbol_refs_state_: Option<SymbolRefsState>,
    /// State is also frozen and set after closure conversion
    pub global_state_: Option<GlobalState>,
}

impl Emitter {
    pub fn new(opts: Options, systemlib: bool, for_debugger_eval: bool) -> Emitter {
        Emitter {
            opts,
            systemlib,
            for_debugger_eval,
            ..Default::default()
        }
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

    pub fn emit_adata_state(&self) -> &AdataState {
        self.adata_state_.as_ref().expect("uninit'd adata_state")
    }
    pub fn emit_adata_state_mut(&mut self) -> &mut AdataState {
        self.adata_state_.get_or_insert_with(AdataState::init)
    }
    pub fn into_adata_emit_state(self) -> AdataState {
        self.adata_state_.expect("uninit'd adata_state")
    }

    pub fn emit_statement_state(&self) -> &StatementState {
        self.statement_state_
            .as_ref()
            .expect("uninit'd statement_state")
    }
    pub fn emit_statement_state_mut(&mut self) -> &mut StatementState {
        self.statement_state_
            .get_or_insert_with(StatementState::init)
    }
    pub fn into_statement_emit_state(self) -> StatementState {
        self.statement_state_.expect("uninit'd statement_state")
    }

    pub fn emit_symbol_refs_state(&self) -> &SymbolRefsState {
        self.symbol_refs_state_
            .as_ref()
            .expect("uninit'd symbol_refs_state")
    }
    pub fn emit_symbol_refs_state_mut(&mut self) -> &mut SymbolRefsState {
        self.symbol_refs_state_
            .get_or_insert_with(SymbolRefsState::init)
    }
    pub fn into_symbol_refs_emit_state(self) -> SymbolRefsState {
        self.symbol_refs_state_.expect("uninit'd symbol_refs_state")
    }

    pub fn emit_global_state(&self) -> &GlobalState {
        self.global_state_.as_ref().expect("uninit'd global_state")
    }
    pub fn emit_global_state_mut(&mut self) -> &mut GlobalState {
        self.global_state_.get_or_insert_with(GlobalState::init)
    }
    pub fn into_global_emit_state(self) -> GlobalState {
        self.global_state_.expect("uninit'd global_state")
    }
}
