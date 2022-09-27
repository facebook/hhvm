// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! This module rewrites a Func's parameters to match the "standard" parameters
//! for a Infer Hack function.  A "standard" Infer Hack function should take in
//! a HackParams and should return a HackMixed.

use ir::instr::Hhbc;
use ir::Constant;
use ir::EnforceableType;
use ir::Func;
use ir::FuncBuilder;
use ir::Instr;
use ir::LocalId;
use ir::Param;
use ir::StringInterner;
use ir::TypeConstraintFlags;
use ir::UserType;
use itertools::Itertools;

use super::func_builder::FuncBuilderEx as _;
use crate::hack;

/// Steal the parameters of the Func contained in `builder` and replace them
/// with a single `*HackParams`. Then insert code at the beginning of the Func
/// to extract the parameters from the passed HackParams.
pub(crate) fn rewrite_entry<'a, 'b>(
    builder: &'b mut FuncBuilder<'a>,
    strings: &'b mut StringInterner,
) {
    // Replace the parameters with a single parameter of type *HackParams.
    let orig_params = std::mem::replace(
        &mut builder.func.params,
        vec![Param {
            name: strings.intern_str("params"),
            is_variadic: false,
            is_inout: false,
            is_readonly: false,
            user_attributes: vec![],
            ty: UserType {
                user_type: None,
                enforced: EnforceableType {
                    ty: tx_ty!(*HackParams),
                    modifiers: TypeConstraintFlags::NoFlags,
                },
            },
            default_value: None,
        }],
    );

    // If any block jumps back to the ENTRY_BID then we'll have to do some
    // rewriting...
    assert!(
        !builder
            .func
            .block_ids()
            .flat_map(|bid| builder.func.edges(bid))
            .contains(&Func::ENTRY_BID)
    );

    let loc = builder.func.span;

    let old_entry = builder.alloc_bid();
    builder.func.blocks.swap(Func::ENTRY_BID, old_entry);

    builder.start_block(Func::ENTRY_BID);

    // Check the parameter count.
    let params_name = strings.intern_str("params");
    let params_var = LocalId::Named(params_name);
    let params = builder.emit(Instr::Hhbc(Hhbc::CGetL(params_var, loc)));
    // TODO: need to handle default parameters.
    let min = builder.emit_constant(Constant::Int(orig_params.len() as i64));
    let max = builder.emit_constant(Constant::Int(orig_params.len() as i64));
    builder.emit_hack_builtin(hack::Builtin::VerifyParamCount, &[params, min, max], loc);

    // Extract the parameters from the HackParam into locals.
    for (idx, param) in orig_params.iter().enumerate() {
        let idx_vid = builder.emit_constant(Constant::Int(idx as i64));
        let vid = builder.emit_hack_builtin(hack::Builtin::GetParam, &[params, idx_vid], loc);
        let lid = LocalId::Named(param.name);
        builder.emit(Instr::Hhbc(Hhbc::SetL(vid, lid, loc)));
    }

    builder.emit(Instr::jmp(old_entry, builder.func.span));
}
