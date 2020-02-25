// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Interestingly, HHAS does not represent the declared types of constants,
// unlike formal parameters and return types. We might consider fixing this.
// Also interestingly, abstract constants are not emitted at all.

use ast_constant_folder_rust as ast_constant_folder;
use emit_expression_rust as emit_expr;
use env::{emitter::Emitter, Env};
use hhbc_id_rust as hhbc_id;
use instruction_sequence_rust::{InstrSeq, Result};
use oxidized::ast as tast;
use runtime::TypedValue;

#[derive(Debug)]
pub struct HhasConstant<'id> {
    pub name: hhbc_id::r#const::Type<'id>,
    pub value: Option<TypedValue>,
    pub initializer_instrs: Option<InstrSeq>,
}

pub fn from_ast<'a>(
    emitter: &mut Emitter,
    env: &Env,
    id: &'a tast::Id,
    expr: Option<&tast::Expr>,
) -> Result<HhasConstant<'a>> {
    let (value, initializer_instrs) = match expr {
        None => (None, None),
        Some(init) => {
            match ast_constant_folder::expr_to_typed_value(emitter, &env.namespace, init) {
                Ok(v) => (Some(v), None),
                Err(_) => (
                    Some(TypedValue::Uninit),
                    Some(emit_expr::emit_expr(emitter, env, init)?),
                ),
            }
        }
    };
    Ok(HhasConstant {
        name: id.name().into(),
        value,
        initializer_instrs,
    })
}
