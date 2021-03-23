// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ast_constant_folder_rust as ast_constant_folder;
use emit_expression_rust as emit_expression;
use emit_fatal_rust as emit_fatal;
use env::{emitter::Emitter, Env};
use hhas_attribute_rust::HhasAttribute;
use hhbc_id_rust::{self as hhbc_id, Id};
use instruction_sequence::Result;
use naming_special_names::user_attributes as ua;
use naming_special_names_rust as naming_special_names;
use oxidized::{ast as a, namespace_env::Env as Namespace};
use runtime::TypedValue;

pub fn from_asts(
    e: &mut Emitter,
    namespace: &Namespace,
    attrs: &[a::UserAttribute],
) -> Result<Vec<HhasAttribute>> {
    attrs
        .iter()
        .map(|attr| from_ast(e, namespace, attr))
        .collect()
}

pub fn from_ast(
    e: &mut Emitter,
    namespace: &Namespace,
    attr: &a::UserAttribute,
) -> Result<HhasAttribute> {
    let arguments = ast_constant_folder::literals_from_exprs(
        namespace,
        &mut attr.params.clone(),
        e,
    )
    .map_err(|err| {
        assert_eq!(
            err,
            ast_constant_folder::Error::UserDefinedConstant,
            "literals_from_expr should have panicked for an error other than UserDefinedConstant"
        );
        emit_fatal::raise_fatal_parse(
            &attr.name.0,
            format!("Attribute arguments must be literals"),
        )
    })?;
    let fully_qualified_id = if attr.name.1.starts_with("__") {
        // don't do anything to builtin attributes
        attr.name.1.to_owned()
    } else {
        escaper::escape(hhbc_id::class::Type::from_ast_name(&attr.name.1).to_raw_string()).into()
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
pub fn add_reified_attribute(tparams: &[a::Tparam]) -> Option<HhasAttribute> {
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

pub fn add_reified_parent_attribute(env: &Env, extends: &Vec<a::Hint>) -> Option<HhasAttribute> {
    if let Some((_, hl)) = extends.first().and_then(|h| h.1.as_happly()) {
        if emit_expression::has_non_tparam_generics(env, hl) {
            return Some(HhasAttribute {
                name: "__HasReifiedParent".into(),
                arguments: vec![],
            });
        }
    }
    return None;
}
