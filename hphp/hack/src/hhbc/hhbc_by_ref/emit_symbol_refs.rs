// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhbc_id::{class, r#const, function, Id};
use hhbc_by_ref_symbol_refs_state::{IncludePath, SymbolRefsState};

pub fn add_include<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>, inc: IncludePath<'arena>) {
    e.emit_symbol_refs_state_mut().includes.insert(inc);
}

pub fn add_constant<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>, s: r#const::ConstType<'arena>) {
    if !s.to_raw_string().is_empty() {
        e.emit_symbol_refs_state_mut().constants.insert(s.0);
    }
}

pub fn add_class<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>, s: class::ClassType<'arena>) {
    if !s.to_raw_string().is_empty() {
        e.emit_symbol_refs_state_mut().classes.insert(s.0);
    }
}

pub fn reset<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>) {
    *e.emit_symbol_refs_state_mut() = SymbolRefsState::init(e.alloc);
}

pub fn add_function<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    s: function::FunctionType<'arena>,
) {
    if !s.to_raw_string().is_empty() {
        e.emit_symbol_refs_state_mut().functions.insert(s.0);
    }
}

pub fn take<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>) -> SymbolRefsState<'arena> {
    let state = e.emit_symbol_refs_state_mut();
    std::mem::take(state) // Replace `state` with the default
    // `SymbolRefsState` value and return the previous `state` value.
}
