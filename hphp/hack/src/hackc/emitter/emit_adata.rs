// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use error::Error;
use error::Result;
use hhbc::AdataId;
use hhbc::ClassName;
use hhbc::TypedValue;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;

pub fn typed_value_into_instr(e: &mut Emitter<'_>, tv: TypedValue) -> Result<InstrSeq> {
    match tv {
        TypedValue::Uninit => Err(Error::unrecoverable("rewrite_typed_value: uninit")),
        TypedValue::Null => Ok(instr::null()),
        TypedValue::Bool(true) => Ok(instr::true_()),
        TypedValue::Bool(false) => Ok(instr::false_()),
        TypedValue::Int(i) => Ok(instr::int(i)),
        TypedValue::String(s) => Ok(instr::string_lit(s)),
        TypedValue::LazyClass(s) => Ok(instr::lazy_class(ClassName::from_ast_name_and_mangle(s))),
        TypedValue::Float(f) => Ok(instr::double(f)),
        TypedValue::Keyset(_) => {
            let arrayid = intern_array(e, tv);
            Ok(instr::keyset(arrayid))
        }
        TypedValue::Vec(_) => {
            let arrayid = intern_array(e, tv);
            Ok(instr::vec(arrayid))
        }
        TypedValue::Dict(_) => {
            let arrayid = intern_array(e, tv);
            Ok(instr::dict(arrayid))
        }
    }
}

fn intern_array(e: &mut Emitter<'_>, tv: TypedValue) -> AdataId {
    e.adata_state.intern(tv)
}
