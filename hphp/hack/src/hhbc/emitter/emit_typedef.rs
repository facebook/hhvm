// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::emitter::Emitter;
use hhbc_by_ref_hhas_pos::HhasSpan;
use hhbc_by_ref_hhas_type::HhasTypeInfo;
use hhbc_by_ref_hhas_typedef::HhasTypedef;
use hhbc_by_ref_hhbc_id::{class::ClassType, Id};
use instruction_sequence::Result;
use oxidized::{aast_defs::Hint, ast};
use runtime::TypedValue;

use std::collections::BTreeMap;

pub fn emit_typedefs_from_program<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    prog: &'a [ast::Def],
) -> Result<Vec<HhasTypedef<'arena>>> {
    prog.iter()
        .filter_map(|def| {
            def.as_typedef().and_then(|td| {
                if !td.is_ctx {
                    Some(emit_typedef(e, td))
                } else {
                    None
                }
            })
        })
        .collect()
}

fn emit_typedef<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    typedef: &'a ast::Typedef,
) -> Result<HhasTypedef<'arena>> {
    let name = ClassType::<'arena>::from_ast_name(emitter.alloc, &typedef.name.1);
    let attributes_res = emit_attribute::from_asts(emitter, &typedef.user_attributes);
    let tparams = emit_body::get_tp_names(&typedef.tparams.as_slice());
    let type_info_res = kind_to_type_info(emitter.alloc, &tparams, &typedef.kind);
    let type_structure_res = kind_to_type_structure(
        emitter,
        &tparams,
        typedef.kind.clone(),
        typedef.vis.is_opaque(),
    );
    let span = HhasSpan::from_pos(&typedef.span);

    attributes_res.and_then(|attributes| {
        type_info_res.and_then(|type_info| {
            type_structure_res.map(|type_structure| HhasTypedef {
                name,
                attributes: emitter
                    .alloc
                    .alloc_slice_fill_iter(attributes.into_iter())
                    .into(),
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
) -> Result<HhasTypeInfo<'arena>> {
    use emit_type_hint::Kind;
    emit_type_hint::hint_to_type_info(alloc, &Kind::TypeDef, false, h.1.is_hoption(), tparams, h)
}

fn kind_to_type_structure<'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    tparams: &[&str],
    h: Hint,
    is_opaque: bool,
) -> Result<TypedValue<'arena>> {
    emit_type_constant::hint_to_type_constant(
        emitter.alloc,
        emitter.options(),
        tparams,
        &BTreeMap::new(),
        &h,
        true,
        is_opaque,
    )
}
