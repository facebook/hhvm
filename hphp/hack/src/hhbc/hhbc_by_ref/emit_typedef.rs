// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc_by_ref_emit_attribute as emit_attribute;
use hhbc_by_ref_emit_body as emit_body;
use hhbc_by_ref_emit_type_constant as emit_type_constant;
use hhbc_by_ref_emit_type_hint as emit_type_hint;
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhas_pos::Span;
use hhbc_by_ref_hhas_type::Info as TypeInfo;
use hhbc_by_ref_hhas_typedef::Typedef;
use hhbc_by_ref_hhbc_id::{class, Id};
use hhbc_by_ref_instruction_sequence::Result;
use hhbc_by_ref_runtime::TypedValue;
use oxidized::{aast_defs::Hint, ast as tast};

use std::collections::BTreeMap;

pub fn emit_typedefs_from_program<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena>,
    prog: &'a [tast::Def],
) -> Result<Vec<Typedef<'arena>>> {
    prog.iter()
        .filter_map(|def| def.as_typedef().map(|td| emit_typedef(alloc, e, td)))
        .collect()
}

fn emit_typedef<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    typedef: &'a tast::Typedef,
) -> Result<Typedef<'arena>> {
    let name = class::Type::<'arena>::from_ast_name(alloc, &typedef.name.1);
    let attributes_res = emit_attribute::from_asts(alloc, emitter, &typedef.user_attributes);
    let tparams = emit_body::get_tp_names(&typedef.tparams.as_slice());
    let type_info_res = kind_to_type_info(alloc, &tparams, &typedef.kind);
    let type_structure_res = kind_to_type_structure(
        alloc,
        emitter,
        &tparams,
        typedef.kind.clone(),
        typedef.vis.is_opaque(),
    );
    let span = Span::from_pos(&typedef.span);

    attributes_res.and_then(|attributes| {
        type_info_res.and_then(|type_info| {
            type_structure_res.map(|type_structure| Typedef {
                name,
                attributes,
                type_info,
                type_structure,
                span,
            })
        })
    })
}

fn kind_to_type_info<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    h: &Hint,
) -> Result<TypeInfo> {
    use emit_type_hint::Kind;
    emit_type_hint::hint_to_type_info(alloc, &Kind::TypeDef, false, h.1.is_hoption(), tparams, h)
}

fn kind_to_type_structure<'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    tparams: &[&str],
    h: Hint,
    is_opaque: bool,
) -> Result<TypedValue<'arena>> {
    emit_type_constant::hint_to_type_constant(
        alloc,
        emitter.options(),
        tparams,
        &BTreeMap::new(),
        &h,
        true,
        is_opaque,
    )
}
