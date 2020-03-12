// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use hhas_adata_rust::{
    HhasAdata, ARRAY_PREFIX, DARRAY_PREFIX, DICT_PREFIX, KEYSET_PREFIX, VARRAY_PREFIX, VEC_PREFIX,
};
use hhbc_ast_rust::*;
use hhbc_string_utils_rust as string_utils;
use instruction_sequence_rust::{Error, InstrSeq, Result};
use options::HhvmFlags;
use runtime::TypedValue as TV;

use std::collections::BTreeMap;

#[derive(Default)]
pub struct State {
    pub array_identifier_counter: usize,
    pub array_identifier_map: BTreeMap<TV, String>,
    pub adata: Vec<HhasAdata>,
}

impl State {
    fn init() -> Box<dyn std::any::Any> {
        Box::new(State {
            array_identifier_counter: 0,
            array_identifier_map: BTreeMap::new(),
            adata: vec![],
        })
    }
}
env::lazy_emit_state!(adata_state, State, State::init);

pub fn rewrite_typed_values(emitter: &mut Emitter, instrseq: &mut InstrSeq) -> Result<()> {
    instrseq.map_result_mut(&mut |instr| rewrite_typed_value(emitter, instr))
}

fn rewrite_typed_value(e: &mut Emitter, instr: &mut Instruct) -> Result<()> {
    use InstructLitConst::*;
    let hack_arr_dv_arrs = e.options().hhvm.flags.contains(HhvmFlags::HACK_ARR_DV_ARRS);
    if let Instruct::ILitConst(TypedValue(tv)) = instr {
        *instr = Instruct::ILitConst(match &tv {
            TV::Uninit => return Err(Error::Unrecoverable("rewrite_typed_value: uninit".into())),
            TV::Null => Null,
            TV::Bool(true) => True,
            TV::Bool(false) => False,
            TV::Int(i) => Int(*i),
            TV::String(s) => String(s.to_owned()),
            TV::Float(f) => Double(string_utils::float::to_string(*f)),
            TV::Keyset(_) => Keyset(get_array_identifier(e, tv)),
            TV::Array(_) => Array(get_array_identifier(e, tv)),
            TV::VArray(_) | TV::DArray(_) if !hack_arr_dv_arrs => {
                Array(get_array_identifier(e, tv))
            }
            TV::Vec(_) | TV::VArray(_) => Vec(get_array_identifier(e, tv)),
            TV::Dict(_) | TV::DArray(_) => Dict(get_array_identifier(e, tv)),
            TV::HhasAdata(d) if d.is_empty() => {
                return Err(Error::Unrecoverable("HhasAdata may not be empty".into()));
            }
            TV::HhasAdata(d) => {
                let identifier = get_array_identifier(e, tv);
                match &d[..1] {
                    VARRAY_PREFIX if hack_arr_dv_arrs => Vec(identifier),
                    DARRAY_PREFIX if hack_arr_dv_arrs => Dict(identifier),
                    VARRAY_PREFIX | DARRAY_PREFIX | ARRAY_PREFIX => Array(identifier),
                    VEC_PREFIX => Vec(identifier),
                    DICT_PREFIX => Dict(identifier),
                    KEYSET_PREFIX => Keyset(identifier),
                    _ => {
                        return Err(Error::Unrecoverable(format!(
                            "Unknown HhasAdata data: {}",
                            d
                        )))
                    }
                }
            }
        })
    };
    Ok(())
}

pub fn get_array_identifier(e: &mut Emitter, tv: &TV) -> String {
    if e.options().hhvm.flags.contains(HhvmFlags::ARRAY_PROVENANCE) {
        next_adata_id(e, tv)
    } else {
        match e.emit_state_mut().array_identifier_map.get(tv) {
            None => {
                let id = next_adata_id(e, tv);
                e.emit_state_mut()
                    .array_identifier_map
                    .insert(tv.clone(), id.clone());
                id
            }
            Some(id) => id.clone(),
        }
    }
}

fn next_adata_id(e: &mut Emitter, value: &TV) -> String {
    let mut state = e.emit_state_mut();
    let id = format!("A_{}", state.array_identifier_counter);
    state.array_identifier_counter += 1;
    state.adata.push(HhasAdata {
        id: id.clone(),
        value: value.clone(),
    });
    id
}

pub fn take(e: &mut Emitter) -> State {
    let state = e.emit_state_mut();
    std::mem::take(state)
}

#[cfg(test)]
mod tests {
    use super::*;

    // verify it compiles (no test attribute)
    #[allow(dead_code)]
    fn ref_state_from_emiter(e: &Emitter) {
        let _: &State = e.emit_state();
    }

    // verify it compiles (no test attribute)
    #[allow(dead_code)]
    fn mut_state_from_emiter(e: &mut Emitter) {
        let _: &mut State = e.emit_state_mut();
    }
}
