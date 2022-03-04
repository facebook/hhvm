// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use adata_state::AdataState;
use env::emitter::Emitter;
use ffi::Str;
use hhas_adata::{HhasAdata, DARRAY_PREFIX, DICT_PREFIX, KEYSET_PREFIX, VARRAY_PREFIX, VEC_PREFIX};
use hhbc_ast::*;
use instruction_sequence::{Error, InstrSeq};
use options::HhvmFlags;
use runtime::TypedValue;

pub fn rewrite_typed_values<'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    instrseq: &mut InstrSeq<'arena>,
) -> std::result::Result<(), instruction_sequence::Error> {
    for instr in instrseq.iter_mut() {
        rewrite_typed_value(emitter, instr)?;
    }
    Ok(())
}

fn rewrite_typed_value<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    instr: &mut Instruct<'arena>,
) -> std::result::Result<(), instruction_sequence::Error> {
    if let Instruct::TypedValue(tv) = instr {
        *instr = match &tv {
            TypedValue::Uninit => {
                return Err(Error::Unrecoverable("rewrite_typed_value: uninit".into()));
            }
            TypedValue::Null => Instruct::Null,
            TypedValue::Bool(true) => Instruct::True,
            TypedValue::Bool(false) => Instruct::False,
            TypedValue::Int(i) => Instruct::Int(*i),
            TypedValue::String(s) => Instruct::String(*s),
            TypedValue::LazyClass(s) => {
                let classid: hhbc_ast::ClassId<'arena> =
                    hhbc_id::class::ClassType::from_ast_name_and_mangle(e.alloc, s.unsafe_as_str());
                Instruct::LazyClass(classid)
            }
            TypedValue::Double(f) => Instruct::Double(f.to_f64()),
            TypedValue::Keyset(_) => {
                let arrayid = Str::from(get_array_identifier(e, tv));
                Instruct::Keyset(arrayid)
            }
            TypedValue::Vec(_) => {
                let arrayid = Str::from(get_array_identifier(e, tv));
                Instruct::Vec(arrayid)
            }
            TypedValue::Dict(_) => {
                let arrayid = Str::from(get_array_identifier(e, tv));
                Instruct::Dict(arrayid)
            }
            TypedValue::HhasAdata(d) if d.is_empty() => {
                return Err(Error::Unrecoverable("HhasAdata may not be empty".into()));
            }
            TypedValue::HhasAdata(d) => {
                let arrayid = Str::from(get_array_identifier(e, tv));
                let d = d.unsafe_as_str();
                match &d[..1] {
                    VARRAY_PREFIX | VEC_PREFIX => Instruct::Vec(arrayid),
                    DARRAY_PREFIX | DICT_PREFIX => Instruct::Dict(arrayid),
                    KEYSET_PREFIX => Instruct::Keyset(arrayid),
                    _ => {
                        return Err(Error::Unrecoverable(format!(
                            "Unknown HhasAdata data: {}",
                            d,
                        )));
                    }
                }
            }
        }
    };
    Ok(())
}

pub fn get_array_identifier<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    tv: &TypedValue<'arena>,
) -> &'arena str {
    if e.options().hhvm.flags.contains(HhvmFlags::ARRAY_PROVENANCE) {
        next_adata_id(e, tv)
    } else {
        match e.emit_adata_state_mut().array_identifier_map.get(tv) {
            None => {
                let id = next_adata_id(e, tv);
                e.emit_adata_state_mut()
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
    let mut state = e.emit_adata_state_mut();
    let id: &str = alloc.alloc_str(&format!("A_{}", state.array_identifier_counter));
    state.array_identifier_counter += 1;
    state.adata.push(HhasAdata {
        id: id.into(),
        value: value.clone(),
    });
    id
}

pub fn take<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>) -> AdataState<'arena> {
    let state = e.emit_adata_state_mut();
    std::mem::take(state)
}

#[cfg(test)]
mod tests {
    use super::*;

    // verify it compiles (no test attribute)
    #[allow(dead_code)]
    fn ref_state_from_emiter<'arena, 'decl>(e: &Emitter<'arena, 'decl>) {
        let _: &AdataState<'_> = e.emit_adata_state();
    }

    // verify it compiles (no test attribute)
    #[allow(dead_code)]
    fn mut_state_from_emiter<'arena, 'decl>(e: &mut Emitter<'arena, 'decl>) {
        let _: &mut AdataState<'_> = e.emit_adata_state_mut();
    }
}
