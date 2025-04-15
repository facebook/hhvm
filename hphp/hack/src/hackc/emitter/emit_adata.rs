// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use error::Error;
use error::Result;
use hhbc::TypedValue;
use instruction_sequence::InstrSeq;
use instruction_sequence::instr;

pub fn typed_value_into_instr(e: &mut Emitter<'_>, tv: TypedValue) -> Result<InstrSeq> {
    match tv {
        TypedValue::Uninit => Err(Error::unrecoverable("rewrite_typed_value: uninit")),
        TypedValue::Null => Ok(instr::null()),
        TypedValue::Bool(true) => Ok(instr::true_()),
        TypedValue::Bool(false) => Ok(instr::false_()),
        TypedValue::Int(i) => Ok(instr::int(i)),
        TypedValue::String(s) => Ok(instr::string_lit(s)),
        TypedValue::LazyClass(s) => Ok(instr::lazy_class(s)),
        TypedValue::Float(f) => Ok(instr::double(f)),
        TypedValue::Keyset(_) => Ok(instr::keyset(intern_array(e, tv))),
        TypedValue::Vec(_) => Ok(instr::vec(intern_array(e, tv))),
        TypedValue::Dict(_) => Ok(instr::dict(intern_array(e, tv))),
    }
}

/// Identical array literals will be shared between funcs in the same unit
fn intern_array(e: &mut Emitter<'_>, tv: TypedValue) -> TypedValue {
    e.adata_state.intern_value(tv).clone()
}
