// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_provider::DeclProvider;
use hhbc_by_ref_adata_state::AdataState;
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhas_adata::{
    HhasAdata, DARRAY_PREFIX, DICT_PREFIX, KEYSET_PREFIX, VARRAY_PREFIX, VEC_PREFIX,
};
use hhbc_by_ref_hhbc_ast::*;
use hhbc_by_ref_hhbc_string_utils as string_utils;
use hhbc_by_ref_instruction_sequence::{Error, InstrSeq};
use hhbc_by_ref_options::HhvmFlags;
use hhbc_by_ref_runtime::TypedValue;

pub fn rewrite_typed_values<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl, D>,
    instrseq: &mut InstrSeq<'arena>,
) -> std::result::Result<(), hhbc_by_ref_instruction_sequence::Error> {
    instrseq.map_result_mut(&mut |instr| rewrite_typed_value(alloc, emitter, instr))
}

fn rewrite_typed_value<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
    instr: &mut Instruct<'arena>,
) -> std::result::Result<(), hhbc_by_ref_instruction_sequence::Error> {
    if let Instruct::ILitConst(InstructLitConst::TypedValue(tv)) = instr {
        *instr = Instruct::ILitConst(match &tv {
            TypedValue::Uninit => {
                return Err(Error::Unrecoverable("rewrite_typed_value: uninit".into()));
            }
            TypedValue::Null => InstructLitConst::Null,
            TypedValue::Bool(true) => InstructLitConst::True,
            TypedValue::Bool(false) => InstructLitConst::False,
            TypedValue::Int(i) => InstructLitConst::Int(*i),
            TypedValue::String(s) => InstructLitConst::String(s),
            TypedValue::LazyClass(s) => {
                let classid: hhbc_by_ref_hhbc_ast::ClassId<'arena> =
                    hhbc_by_ref_hhbc_id::class::Type::from_ast_name_and_mangle(alloc, *s);
                InstructLitConst::LazyClass(classid)
            }
            TypedValue::Float(f) => {
                let fstr = bumpalo::collections::String::from_str_in(
                    string_utils::float::to_string(*f).as_str(),
                    alloc,
                )
                .into_bump_str();
                InstructLitConst::Double(fstr)
            }
            TypedValue::Keyset(_) => {
                let arrayid = get_array_identifier(alloc, e, tv);
                InstructLitConst::Keyset(arrayid)
            }
            TypedValue::Vec(_) => {
                let arrayid = get_array_identifier(alloc, e, tv);
                InstructLitConst::Vec(arrayid)
            }
            TypedValue::Dict(_) => {
                let arrayid = get_array_identifier(alloc, e, tv);
                InstructLitConst::Dict(arrayid)
            }
            TypedValue::HhasAdata(d) if d.is_empty() => {
                return Err(Error::Unrecoverable("HhasAdata may not be empty".into()));
            }
            TypedValue::HhasAdata(d) => {
                let arrayid = get_array_identifier(alloc, e, tv);
                match &d[..1] {
                    VARRAY_PREFIX | VEC_PREFIX => InstructLitConst::Vec(arrayid),
                    DARRAY_PREFIX | DICT_PREFIX => InstructLitConst::Dict(arrayid),
                    KEYSET_PREFIX => InstructLitConst::Keyset(arrayid),
                    _ => {
                        return Err(Error::Unrecoverable(format!(
                            "Unknown HhasAdata data: {}",
                            d
                        )));
                    }
                }
            }
        })
    };
    Ok(())
}

pub fn get_array_identifier<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
    tv: &TypedValue<'arena>,
) -> &'arena str {
    if e.options().hhvm.flags.contains(HhvmFlags::ARRAY_PROVENANCE) {
        next_adata_id(alloc, e, tv)
    } else {
        match e.emit_adata_state_mut(alloc).array_identifier_map.get(tv) {
            None => {
                let id = next_adata_id(alloc, e, tv);
                e.emit_adata_state_mut(alloc)
                    .array_identifier_map
                    .insert(tv.clone(), id);
                id
            }
            Some(id) => id,
        }
    }
}

fn next_adata_id<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
    value: &TypedValue<'arena>,
) -> &'arena str {
    let mut state = e.emit_adata_state_mut(alloc);
    let id = format!("A_{}", state.array_identifier_counter);
    state.array_identifier_counter += 1;
    state.adata.push(HhasAdata {
        id: id.clone(),
        value: value.clone(),
    });
    bumpalo::collections::String::from_str_in(id.as_str(), alloc).into_bump_str()
}

pub fn take<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
) -> AdataState<'arena> {
    let state = e.emit_adata_state_mut(alloc);
    std::mem::take(state)
}

#[cfg(test)]
mod tests {
    use super::*;

    // verify it compiles (no test attribute)
    #[allow(dead_code)]
    #[allow(clippy::needless_lifetimes)]
    fn ref_state_from_emiter<'arena, 'decl, D: DeclProvider<'decl>>(e: &Emitter<'arena, 'decl, D>) {
        let _: &AdataState = e.emit_adata_state();
    }

    // verify it compiles (no test attribute)
    #[allow(dead_code)]
    #[allow(clippy::needless_lifetimes)]
    fn mut_state_from_emiter<'arena, 'decl, D: DeclProvider<'decl>>(
        alloc: &'arena bumpalo::Bump,
        e: &mut Emitter<'arena, 'decl, D>,
    ) {
        let _: &mut AdataState = e.emit_adata_state_mut(alloc);
    }
}
