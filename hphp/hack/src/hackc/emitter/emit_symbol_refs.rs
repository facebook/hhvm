// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use hhbc::{
    hhas_symbol_refs::IncludePath,
    hhbc_id::{class, constant, function},
};

pub fn add_include<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>, inc: IncludePath<'arena>) {
    e.symbol_refs_state_mut().includes.insert(inc);
}

pub fn add_constant<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>, s: constant::ConstType<'arena>) {
    if !s.unsafe_as_str().is_empty() {
        e.symbol_refs_state_mut().constants.insert(s.as_ffi_str());
    }
}

pub fn add_class<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>, s: class::ClassType<'arena>) {
    if !s.unsafe_as_str().is_empty() {
        e.symbol_refs_state_mut().classes.insert(s.as_ffi_str());
    }
}

pub fn reset<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>) {
    *e.symbol_refs_state_mut() = Default::default();
}

pub fn add_function<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    s: function::FunctionType<'arena>,
) {
    if !s.unsafe_as_str().is_empty() {
        e.symbol_refs_state_mut().functions.insert(s.as_ffi_str());
    }
}
