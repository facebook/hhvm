// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use env::emitter::Emitter;
use env::Env;
use error::Error;
use error::Result;
use hhbc::Attribute;
use hhbc::TypedValue;
use naming_special_names::user_attributes as ua;
use naming_special_names_rust as naming_special_names;
use oxidized::aast::Expr;
use oxidized::aast::Expr_;
use oxidized::ast as a;

use crate::emit_expression;

pub fn from_asts(e: &mut Emitter<'_, '_>, attrs: &[a::UserAttribute]) -> Result<Vec<Attribute>> {
    attrs.iter().map(|attr| from_ast(e, attr)).collect()
}

pub fn from_ast(e: &mut Emitter<'_, '_>, attr: &a::UserAttribute) -> Result<Attribute> {
    let mut arguments: Vec<Expr<_, _>> = attr
        .params
        .iter()
        .map(|param| {
            // Treat enum class label syntax Foo#Bar as "Bar" in attribute arguments.
            if let Expr_::EnumClassLabel(ecl) = &param.2 {
                let label = &ecl.1;
                Expr(
                    param.0,
                    param.1.clone(),
                    Expr_::String(label.clone().into()),
                )
            } else {
                param.clone()
            }
        })
        .collect();

    let arguments = constant_folder::literals_from_exprs(
        &mut arguments,
        // T88847409 likely need to track scope for folding closure param attribute args
        &ast_scope::Scope::default(),
        e,
    )
    .map_err(|err| {
        assert_eq!(
            err,
            constant_folder::Error::UserDefinedConstant,
            "literals_from_expr should have panicked for an error other than UserDefinedConstant"
        );
        Error::fatal_parse(&attr.name.0, "Attribute arguments must be literals")
    })?;
    let fully_qualified_id = if attr.name.1.starts_with("__") {
        // don't do anything to builtin attributes
        &attr.name.1
    } else {
        hhbc::ClassName::from_ast_name_and_mangle(&attr.name.1).as_str()
    };
    Ok(Attribute::new(fully_qualified_id, arguments))
}

/// Adds an __Reified attribute for functions and classes with reified type
/// parameters. The arguments to __Reified are number of type parameters
/// followed by the indicies of these reified type parameters and whether they
/// are soft reified or not
pub fn add_reified_attribute(tparams: &[a::Tparam]) -> Option<Attribute> {
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

    let name = "__Reified";
    let bool2i64 = |b| b as i64;
    let mut arguments = Vec::with_capacity(reified_data.len() * 3 + 1);
    arguments.push(TypedValue::Int(tparams.len() as i64));
    for (i, soft, warn) in reified_data.into_iter() {
        arguments.push(TypedValue::Int(i as i64));
        arguments.push(TypedValue::Int(bool2i64(soft)));
        arguments.push(TypedValue::Int(bool2i64(warn)));
    }
    Some(Attribute::new(name, arguments))
}

pub fn add_reified_parent_attribute(env: &Env<'_>, extends: &[a::Hint]) -> Option<Attribute> {
    if let Some((_, hl)) = extends.first().and_then(|h| h.1.as_happly()) {
        if emit_expression::has_non_tparam_generics(env, hl) {
            return Some(Attribute::new("__HasReifiedParent", vec![]));
        }
    }
    None
}
