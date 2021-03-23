// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use emit_attribute_rust as emit_attribute;
use emit_body_rust as emit_body;
use emit_type_constant_rust as emit_type_constant;
use emit_type_hint_rust as emit_type_hint;
use env::emitter::Emitter;
use hhas_pos_rust::Span;
use hhas_type::Info as TypeInfo;
use hhas_typedef_rust::Typedef;
use hhbc_id_rust::{class, Id};
use instruction_sequence::Result;
use oxidized::{aast_defs::Hint, ast as tast};
use runtime::TypedValue;

use std::collections::BTreeMap;

pub fn emit_typedefs_from_program<'a>(
    e: &mut Emitter,
    prog: &'a tast::Program,
) -> Result<Vec<Typedef<'a>>> {
    prog.iter()
        .filter_map(|def| def.as_typedef().map(|td| emit_typedef(e, td)))
        .collect()
}

fn emit_typedef<'a>(emitter: &mut Emitter, typedef: &'a tast::Typedef) -> Result<Typedef<'a>> {
    let name = class::Type::from_ast_name(&typedef.name.1);
    let attributes_res =
        emit_attribute::from_asts(emitter, &typedef.namespace, &typedef.user_attributes);
    let tparams = emit_body::get_tp_names(&typedef.tparams.as_slice());
    let type_info_res = kind_to_type_info(&tparams, &typedef.kind);
    let type_structure_res = kind_to_type_structure(
        emitter,
        &tparams,
        typedef.kind.clone(),
        typedef.vis.is_opaque(),
    );
    let span = Span::from_pos(&typedef.span);

    attributes_res.and_then(|attributes| {
        type_info_res.and_then(|type_info| {
            type_structure_res.and_then(|type_structure| {
                Ok(Typedef {
                    name,
                    attributes,
                    type_info,
                    type_structure,
                    span,
                })
            })
        })
    })
}

fn kind_to_type_info(tparams: &[&str], h: &Hint) -> Result<TypeInfo> {
    use emit_type_hint::Kind;
    emit_type_hint::hint_to_type_info(&Kind::TypeDef, false, h.1.is_hoption(), tparams, h)
}

fn kind_to_type_structure(
    emitter: &mut Emitter,
    tparams: &[&str],
    h: Hint,
    is_opaque: bool,
) -> Result<TypedValue> {
    emit_type_constant::hint_to_type_constant(
        emitter.options(),
        tparams,
        &BTreeMap::new(),
        &h,
        true,
        is_opaque,
    )
}
