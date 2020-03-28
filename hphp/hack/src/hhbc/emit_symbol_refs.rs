// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

use env::emitter::Emitter;
use hhas_symbol_refs_rust::*;
use hhbc_id_rust::{class, function, r#const, Id};
use std::collections::BTreeSet;

#[derive(Default)]
pub struct State {
    pub symbol_refs: HhasSymbolRefs,
}

impl State {
    fn init() -> Box<dyn std::any::Any> {
        Box::new(State {
            symbol_refs: HhasSymbolRefs {
                includes: BTreeSet::new(),
                constants: BTreeSet::new(),
                functions: BTreeSet::new(),
                classes: BTreeSet::new(),
            },
        })
    }

    pub fn get_symbol_refs(e: &mut Emitter) -> &HhasSymbolRefs {
        &e.emit_state_mut().symbol_refs
    }

    pub fn set_symbol_refs(e: &mut Emitter, s: HhasSymbolRefs) {
        e.emit_state_mut().symbol_refs = s;
    }

    pub fn add_include(e: &mut Emitter, inc: IncludePath) {
        e.emit_state_mut().symbol_refs.includes.insert(inc);
    }

    pub fn add_constant(e: &mut Emitter, s: r#const::Type) {
        if !s.to_raw_string().is_empty() {
            e.emit_state_mut().symbol_refs.constants.insert(s.into());
        }
    }

    pub fn add_class(e: &mut Emitter, s: class::Type) {
        if !s.to_raw_string().is_empty() {
            e.emit_state_mut().symbol_refs.classes.insert(s.into());
        }
    }

    pub fn reset(e: &mut Emitter) {
        e.emit_state_mut().symbol_refs = HhasSymbolRefs {
            includes: BTreeSet::new(),
            constants: BTreeSet::new(),
            functions: BTreeSet::new(),
            classes: BTreeSet::new(),
        };
    }
}

pub fn add_function(e: &mut Emitter, s: function::Type) {
    if !s.to_raw_string().is_empty() {
        e.emit_state_mut().symbol_refs.functions.insert(s.into());
    }
}

pub fn take(e: &mut Emitter) -> State {
    let state = e.emit_state_mut();
    std::mem::take(state)
}

env::lazy_emit_state!(symbol_refs_state, State, State::init);
