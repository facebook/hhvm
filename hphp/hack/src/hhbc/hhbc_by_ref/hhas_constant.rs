// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Interestingly, HHAS does not represent the declared types of constants,
// unlike formal parameters and return types. We might consider fixing this.
// Also interestingly, abstract constants are not emitted at all.

use decl_provider::DeclProvider;
use hhbc_by_ref_ast_constant_folder as ast_constant_folder;
use hhbc_by_ref_emit_expression as emit_expr;
use hhbc_by_ref_env::{emitter::Emitter, Env};
use hhbc_by_ref_hhbc_id::{self as hhbc_id, Id};
use hhbc_by_ref_instruction_sequence::{InstrSeq, Result};
use hhbc_by_ref_runtime::TypedValue;
use oxidized::ast as tast;

#[derive(Debug)]
pub struct HhasConstant<'arena> {
    pub name: hhbc_id::r#const::Type<'arena>,
    pub value: Option<TypedValue<'arena>>,
    pub initializer_instrs: Option<InstrSeq<'arena>>,
    pub is_abstract: bool,
}

pub fn from_ast<'a, 'arena, 'decl, D: DeclProvider<'decl>>(
    emitter: &mut Emitter<'arena, 'decl, D>,
    env: &Env<'a, 'arena>,
    id: &'a tast::Id,
    is_abstract: bool,
    expr: Option<&tast::Expr>,
) -> Result<HhasConstant<'arena>> {
    let alloc = env.arena;
    let (value, initializer_instrs) = match expr {
        None => (None, None),
        Some(init) => match ast_constant_folder::expr_to_typed_value(alloc, emitter, init) {
            Ok(v) => (Some(v), None),
            Err(_) => (
                Some(TypedValue::Uninit),
                Some(emit_expr::emit_expr(emitter, env, init)?),
            ),
        },
    };
    Ok(HhasConstant {
        name: hhbc_id::r#const::Type::from_ast_name(alloc, id.name()),
        value,
        initializer_instrs,
        is_abstract,
    })
}
