// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use adata_state::AdataState;
use env::emitter::Emitter;
use error::Error;
use error::Result;
use ffi::Str;
use hhbc::Adata;
use hhbc::ClassName;
use hhbc::TypedValue;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use options::HhvmFlags;

pub fn typed_value_to_instr<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    tv: &TypedValue<'arena>,
) -> Result<InstrSeq<'arena>> {
    match &tv {
        TypedValue::Uninit => Err(Error::unrecoverable("rewrite_typed_value: uninit")),
        TypedValue::Null => Ok(instr::null()),
        TypedValue::Bool(true) => Ok(instr::true_()),
        TypedValue::Bool(false) => Ok(instr::false_()),
        TypedValue::Int(i) => Ok(instr::int(*i)),
        TypedValue::String(s) => Ok(instr::string(e.alloc, s.unsafe_as_str())),
        TypedValue::LazyClass(s) => {
            let classid: ClassName<'arena> =
                ClassName::from_ast_name_and_mangle(e.alloc, s.unsafe_as_str());
            Ok(instr::lazy_class(classid))
        }
        TypedValue::Float(f) => Ok(instr::double(*f)),
        TypedValue::Keyset(_) => {
            let arrayid = Str::from(get_array_identifier(e, tv));
            Ok(instr::keyset(arrayid))
        }
        TypedValue::Vec(_) => {
            let arrayid = Str::from(get_array_identifier(e, tv));
            Ok(instr::vec(arrayid))
        }
        TypedValue::Dict(_) => {
            let arrayid = Str::from(get_array_identifier(e, tv));
            Ok(instr::dict(arrayid))
        }
    }
}

pub fn get_array_identifier<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    tv: &TypedValue<'arena>,
) -> &'arena str {
    if e.options().hhvm.flags.contains(HhvmFlags::ARRAY_PROVENANCE) {
        next_adata_id(e, tv)
    } else {
        match e.adata_state_mut().array_identifier_map.get(tv) {
            None => {
                let id = next_adata_id(e, tv);
                e.adata_state_mut()
                    .array_identifier_map
                    .insert(tv.clone(), id);
                id
            }
            Some(id) => id,
        }
    }
}

fn next_adata_id<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    value: &TypedValue<'arena>,
) -> &'arena str {
    let alloc = e.alloc;
    let mut state = e.adata_state_mut();
    let id: &str = alloc.alloc_str(&format!("A_{}", state.array_identifier_counter));
    state.array_identifier_counter += 1;
    state.adata.push(Adata {
        id: id.into(),
        value: value.clone(),
    });
    id
}

pub fn take<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>) -> AdataState<'arena> {
    let state = e.adata_state_mut();
    std::mem::take(state)
}

#[cfg(test)]
mod tests {
    use super::*;

    // verify it compiles (no test attribute)
    #[allow(dead_code)]
    fn ref_state_from_emiter<'arena, 'decl>(e: &Emitter<'arena, 'decl>) {
        let _: &AdataState<'_> = e.adata_state();
    }

    // verify it compiles (no test attribute)
    #[allow(dead_code)]
    fn mut_state_from_emiter<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>) {
        let _: &mut AdataState<'_> = e.adata_state_mut();
    }
}
