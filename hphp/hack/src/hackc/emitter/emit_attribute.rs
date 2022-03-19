// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::{emitter::Emitter, Env};
use hhas_attribute::HhasAttribute;
use instruction_sequence::{Error, Result};
use naming_special_names::user_attributes as ua;
use naming_special_names_rust as naming_special_names;
use oxidized::ast as a;
use runtime::TypedValue;

pub fn from_asts<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    attrs: &[a::UserAttribute],
) -> Result<Vec<HhasAttribute<'arena>>> {
    attrs.iter().map(|attr| from_ast(e, attr)).collect()
}

pub fn from_ast<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    attr: &a::UserAttribute,
) -> Result<HhasAttribute<'arena>> {
    let arguments = ast_constant_folder::literals_from_exprs(
        &mut attr.params.clone(),
        e,
    )
    .map_err(|err| {
        assert_eq!(
            err,
            ast_constant_folder::Error::UserDefinedConstant,
            "literals_from_expr should have panicked for an error other than UserDefinedConstant"
        );
        Error::fatal_parse(&attr.name.0, "Attribute arguments must be literals")
    })?;
    let fully_qualified_id = if attr.name.1.starts_with("__") {
        // don't do anything to builtin attributes
        &attr.name.1
    } else {
        hhbc_id::class::ClassType::from_ast_name_and_mangle(e.alloc, &attr.name.1).unsafe_as_str()
    };
    Ok(HhasAttribute {
        name: e.alloc.alloc_str(fully_qualified_id).into(),
        arguments: e.alloc.alloc_slice_fill_iter(arguments.into_iter()).into(),
    })
}

/// Adds an __Reified attribute for functions and classes with reified type
/// parameters. The arguments to __Reified are number of type parameters
/// followed by the indicies of these reified type parameters and whether they
/// are soft reified or not
pub fn add_reified_attribute<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[a::Tparam],
) -> Option<HhasAttribute<'arena>> {
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

    let name = "__Reified".into();
    let bool2i64 = |b| b as i64;
    // NOTE(hrust) hopefully faster than .into_iter().flat_map(...).collect()
    let mut arguments =
        bumpalo::collections::vec::Vec::with_capacity_in(reified_data.len() * 3 + 1, alloc);
    arguments.push(TypedValue::Int(tparams.len() as i64));
    for (i, soft, warn) in reified_data.into_iter() {
        arguments.push(TypedValue::Int(i as i64));
        arguments.push(TypedValue::Int(bool2i64(soft)));
        arguments.push(TypedValue::Int(bool2i64(warn)));
    }
    Some(HhasAttribute {
        name,
        arguments: arguments.into_bump_slice().into(),
    })
}

pub fn add_reified_parent_attribute<'a, 'arena>(
    env: &Env<'a, 'arena>,
    extends: &[a::Hint],
) -> Option<HhasAttribute<'arena>> {
    if let Some((_, hl)) = extends.first().and_then(|h| h.1.as_happly()) {
        if emit_expression::has_non_tparam_generics(env, hl) {
            return Some(HhasAttribute {
                name: "__HasReifiedParent".into(),
                arguments: ffi::Slice::empty(),
            });
        }
    }
    None
}
