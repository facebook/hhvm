// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_ast_constant_folder as ast_constant_folder;
use hhbc_by_ref_emit_expression as emit_expression;
use hhbc_by_ref_emit_fatal as emit_fatal;
use hhbc_by_ref_env::{emitter::Emitter, Env};
use hhbc_by_ref_hhas_attribute::HhasAttribute;
use hhbc_by_ref_hhbc_id::{self as hhbc_id, Id};
use hhbc_by_ref_instruction_sequence::Result;
use hhbc_by_ref_runtime::TypedValue;
use naming_special_names::user_attributes as ua;
use naming_special_names_rust as naming_special_names;
use oxidized::ast as a;

pub fn from_asts<'arena>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena>,
    attrs: &[a::UserAttribute],
) -> Result<Vec<HhasAttribute<'arena>>> {
    attrs.iter().map(|attr| from_ast(alloc, e, attr)).collect()
}

pub fn from_ast<'arena>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena>,
    attr: &a::UserAttribute,
) -> Result<HhasAttribute<'arena>> {
    let arguments = ast_constant_folder::literals_from_exprs(
        &mut attr.params.clone(),
        alloc,
        e,
    )
    .map_err(|err| {
        assert_eq!(
            err,
            ast_constant_folder::Error::UserDefinedConstant,
            "literals_from_expr should have panicked for an error other than UserDefinedConstant"
        );
        emit_fatal::raise_fatal_parse(&attr.name.0, "Attribute arguments must be literals")
    })?;
    let fully_qualified_id = if attr.name.1.starts_with("__") {
        // don't do anything to builtin attributes
        attr.name.1.to_owned()
    } else {
        escaper::escape(hhbc_id::class::Type::from_ast_name(alloc, &attr.name.1).to_raw_string())
            .into()
    };
    Ok(HhasAttribute {
        name: fully_qualified_id,
        arguments,
    })
}

/// Adds an __Reified attribute for functions and classes with reified type
/// parameters. The arguments to __Reified are number of type parameters
/// followed by the indicies of these reified type parameters and whether they
/// are soft reified or not
pub fn add_reified_attribute<'arena>(tparams: &[a::Tparam]) -> Option<HhasAttribute<'arena>> {
    let reified_data: Vec<(usize, bool, bool)> = tparams
        .iter()
        .enumerate()
        .filter_map(|(i, tparam)| {
            if tparam.reified == a::ReifyKind::Erased {
                None
            } else {
                let soft = tparam.user_attributes.iter().any(|a| a.name.1 == ua::SOFT);
                let warn = tparam.user_attributes.iter().any(|a| a.name.1 == ua::WARN);
                Some((i, soft, warn))
            }
        })
        .collect();
    if reified_data.is_empty() {
        return None;
    }

    let name = "__Reified".to_owned();
    let bool2i64 = |b| b as i64;
    // NOTE(hrust) hopefully faster than .into_iter().flat_map(...).collect()
    let mut arguments = Vec::with_capacity(reified_data.len() * 3 + 1);
    arguments.push(TypedValue::Int(tparams.len() as i64));
    for (i, soft, warn) in reified_data.into_iter() {
        arguments.push(TypedValue::Int(i as i64));
        arguments.push(TypedValue::Int(bool2i64(soft)));
        arguments.push(TypedValue::Int(bool2i64(warn)));
    }
    Some(HhasAttribute { name, arguments })
}

pub fn add_reified_parent_attribute<'a, 'arena>(
    env: &Env<'a, 'arena>,
    extends: &[a::Hint],
) -> Option<HhasAttribute<'arena>> {
    if let Some((_, hl)) = extends.first().and_then(|h| h.1.as_happly()) {
        if emit_expression::has_non_tparam_generics(env, hl) {
            return Some(HhasAttribute {
                name: "__HasReifiedParent".into(),
                arguments: vec![],
            });
        }
    }
    None
}
