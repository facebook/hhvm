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

pub fn typed_value_into_instr<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    tv: TypedValue<'arena>,
) -> Result<InstrSeq<'arena>> {
    match tv {
        TypedValue::Uninit => Err(Error::unrecoverable("rewrite_typed_value: uninit")),
        TypedValue::Null => Ok(instr::null()),
        TypedValue::Bool(true) => Ok(instr::true_()),
        TypedValue::Bool(false) => Ok(instr::false_()),
        TypedValue::Int(i) => Ok(instr::int(i)),
        TypedValue::String(s) => Ok(instr::string(e.alloc, s.unsafe_as_str())),
        TypedValue::LazyClass(s) => {
            let classid = ClassName::from_ast_name_and_mangle(e.alloc, s.unsafe_as_str());
            Ok(instr::lazy_class(classid))
        }
        TypedValue::Float(f) => Ok(instr::double(f)),
        TypedValue::Keyset(_) => {
            let arrayid = get_array_identifier(e, tv);
            Ok(instr::keyset(arrayid))
        }
        TypedValue::Vec(_) => {
            let arrayid = get_array_identifier(e, tv);
            Ok(instr::vec(arrayid))
        }
        TypedValue::Dict(_) => {
            let arrayid = get_array_identifier(e, tv);
            Ok(instr::dict(arrayid))
        }
    }
}

fn get_array_identifier<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    tv: TypedValue<'arena>,
) -> AdataId<'arena> {
    let alloc = e.alloc;
    if e.options().hhbc.array_provenance {
        e.adata_state_mut().push(alloc, tv)
    } else {
        e.adata_state_mut().intern(alloc, tv)
    }
}
