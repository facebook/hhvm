// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ast_scope_rust::Scope;
use emit_attribute_rust as emit_attribute;
use emit_expression_rust as emit_expression;
use emit_fatal_rust as emit_fatal;
use emit_pos_rust as emit_pos;
use emit_type_hint_rust::{hint_to_type_info, Kind};
use env::{emitter::Emitter, Env};
use hhas_param_rust::HhasParam;
use hhas_type::Info;
use hhbc_string_utils_rust::locals::strip_dollar;
use instruction_sequence::{instr, InstrSeq, Result};
use options::LangFlags;
use oxidized::{
    aast_defs::{Hint, Hint_},
    aast_visitor::{self, AstParams, Node},
    ast as a,
    ast_defs::{Id, ParamKind},
    namespace_env::Env as Namespace,
    pos::Pos,
};

use std::collections::{BTreeMap, BTreeSet};
use std::marker::PhantomData;

pub fn from_asts(
    emitter: &mut Emitter,
    tparams: &mut Vec<&str>,
    namespace: &Namespace,
    generate_defaults: bool,
    scope: &Scope,
    ast_params: &[a::FunParam],
) -> Result<Vec<HhasParam>> {
    ast_params
        .iter()
        .map(|param| from_ast(emitter, tparams, namespace, generate_defaults, scope, param))
        .collect::<Result<Vec<_>>>()
        .map(|params| {
            params
                .iter()
                .filter_map(|p| p.to_owned())
                .collect::<Vec<_>>()
        })
        .map(rename_params)
}

fn rename_params(mut params: Vec<HhasParam>) -> Vec<HhasParam> {
    fn rename(
        names: &BTreeSet<String>,
        param_counts: &mut BTreeMap<String, usize>,
        param: &mut HhasParam,
    ) {
        match param_counts.get_mut(&param.name) {
            None => {
                param_counts.insert(param.name.clone(), 0);
            }
            Some(count) => {
                let newname = format!("{}{}", param.name, count);
                *count += 1;
                if names.contains(&newname) {
                    rename(names, param_counts, param);
                } else {
                    param.name = newname;
                }
            }
        }
    }
    let mut param_counts = BTreeMap::new();
    let names = params
        .iter()
        .map(|p| p.name.clone())
        .collect::<BTreeSet<_>>();
    params
        .iter_mut()
        .rev()
        .for_each(|p| rename(&names, &mut param_counts, p));
    params.into_iter().collect()
}

fn from_ast(
    emitter: &mut Emitter,
    tparams: &mut Vec<&str>,
    namespace: &Namespace,
    generate_defaults: bool,
    scope: &Scope,
    param: &a::FunParam,
) -> Result<Option<HhasParam>> {
    if param.is_variadic && param.name == "..." {
        return Ok(None);
    };
    if param.is_variadic {
        tparams.push("array");
    };
    let nullable = param
        .expr
        .as_ref()
        .map_or(false, |a::Expr(_, e)| e.is_null());
    let type_info = {
        let param_type_hint = if param.is_variadic {
            Some(Hint(
                Pos::make_none(),
                Box::new(Hint_::mk_happly(
                    Id(Pos::make_none(), "array".to_string()),
                    param
                        .type_hint
                        .get_hint()
                        .as_ref()
                        .map_or(vec![], |h| vec![h.clone()]),
                )),
            ))
        } else if emitter
            .options()
            .hhvm
            .hack_lang
            .flags
            .contains(LangFlags::ENABLE_ENUM_CLASSES)
            && param.user_attributes.iter().any(|a| match &a.name {
                Id(_, s) => s == "__Atom",
            })
        {
            Some(Hint(
                Pos::make_none(),
                Box::new(Hint_::mk_happly(
                    Id(Pos::make_none(), "HH\\string".to_string()),
                    vec![],
                )),
            ))
        } else {
            param.type_hint.get_hint().clone()
        };
        if let Some(h) = param_type_hint {
            Some(hint_to_type_info(
                &Kind::Param,
                false,
                nullable,
                &tparams[..],
                &h,
            )?)
        } else {
            None
        }
    };
    // Do the type check for default value type and hint type
    if !nullable {
        if let Some(err_msg) = default_type_check(&param.name, &type_info, &param.expr) {
            return Err(emit_fatal::raise_fatal_parse(&param.pos, err_msg));
        }
    };
    aast_visitor::visit(
        &mut ResolverVisitor(PhantomData),
        &mut Ctx { emitter, scope },
        &param.expr,
    )
    .unwrap();
    let default_value = if generate_defaults {
        param
            .expr
            .as_ref()
            .map(|expr| (emitter.label_gen_mut().next_default_arg(), expr.clone()))
    } else {
        None
    };
    let is_inout = match param.callconv {
        Some(ParamKind::Pinout) => true,
        _ => false,
    };
    Ok(Some(HhasParam {
        name: param.name.clone(),
        is_variadic: param.is_variadic,
        is_inout,
        user_attributes: emit_attribute::from_asts(emitter, namespace, &param.user_attributes)?,
        type_info,
        default_value,
    }))
}

pub fn emit_param_default_value_setter(
    emitter: &mut Emitter,
    env: &Env,
    pos: &Pos,
    params: &[HhasParam],
) -> Result<(InstrSeq, InstrSeq)> {
    let param_to_setter = |param: &HhasParam| {
        param.default_value.as_ref().map(|(lbl, expr)| {
            let instrs = InstrSeq::gather(vec![
                emit_expression::emit_expr(emitter, env, &expr)?,
                emit_pos::emit_pos(pos),
                instr::setl(local::Type::Named(param.name.clone())),
                instr::popc(),
            ]);
            Ok(InstrSeq::gather(vec![instr::label(lbl.to_owned()), instrs]))
        })
    };
    let setters = params
        .iter()
        .filter_map(param_to_setter)
        .collect::<Result<Vec<_>>>()?;
    if setters.is_empty() {
        Ok((InstrSeq::Empty, InstrSeq::Empty))
    } else {
        let l = emitter.label_gen_mut().next_regular();
        Ok((
            instr::label(l.clone()),
            InstrSeq::gather(vec![InstrSeq::gather(setters), instr::jmpns(l)]),
        ))
    }
}

struct ResolverVisitor<'a>(PhantomData<&'a ()>);

#[allow(dead_code)]
struct Ctx<'a> {
    emitter: &'a mut Emitter,
    scope: &'a Scope<'a>,
}

impl<'ast, 'a> aast_visitor::Visitor<'ast> for ResolverVisitor<'a> {
    type P = AstParams<Ctx<'a>, ()>;

    fn object(&mut self) -> &mut dyn aast_visitor::Visitor<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, c: &mut Ctx<'a>, p: &a::Expr) -> std::result::Result<(), ()> {
        p.recurse(c, self.object())
        // TODO(hrust) implement on_CIexpr & remove dead_code on struct Ctx
    }
}

// Return None if it passes type check, otherwise return error msg
fn default_type_check(
    param_name: &str,
    param_type_info: &Option<Info>,
    param_expr: &Option<a::Expr>,
) -> Option<String> {
    let hint_type = get_hint_display_name(
        param_type_info
            .as_ref()
            .and_then(|ti| ti.user_type.as_ref()),
    );
    // If matches, return None, otherwise return default_type
    let default_type = hint_type.and_then(|ht| match_default_and_hint(ht, param_expr));
    let param_true_name = strip_dollar(param_name);
    default_type.and_then(|dt| hint_type.and_then(|ht| match ht {
        "class" => Some(format!(
            "Default value for parameter {} with a class type hint can only be NULL",
            param_true_name)),
        _ => Some(format!(
            "Default value for parameter {} with type {} needs to have the same type as the type hint {}",
            param_true_name,
            dt,
            ht))
    }))
}

fn get_hint_display_name(hint: Option<&String>) -> Option<&str> {
    hint.map(|h| match h.as_str() {
        "HH\\bool" => "bool",
        "HH\\varray" => "HH\\varray",
        "HH\\darray" => "HH\\darray",
        "HH\\varray_or_darray" => "HH\\varray_or_darray",
        "HH\\vec_or_dict" => "HH\\vec_or_dict",
        "HH\\AnyArray" => "HH\\AnyArray",
        "HH\\int" => "int",
        "HH\\num" => "num",
        "HH\\arraykey" => "arraykey",
        "HH\\float" => "float",
        "HH\\string" => "string",
        _ => "class",
    })
}

// By now only check default type for bool, array, int, float and string.
//    Return None when hint_type and default_value matches (in hh mode,
//    "class" type matches anything). If not, return default_value type string
//    for printing fatal parse error
fn match_default_and_hint(hint_type: &str, param_expr: &Option<a::Expr>) -> Option<&'static str> {
    if hint_type == "class" {
        return None;
    }
    match &param_expr.as_ref() {
        None => None,
        Some(e) => match e.1 {
            a::Expr_::True | a::Expr_::False => match hint_type {
                "bool" => None,
                _ => Some("Boolean"),
            },
            a::Expr_::Int(_) => match hint_type {
                "int" | "num" | "arraykey" | "float" => None,
                _ => Some("Int64"),
            },
            a::Expr_::Float(_) => match hint_type {
                "float" | "num" => None,
                _ => Some("Double"),
            },
            a::Expr_::String(_) => match hint_type {
                "string" | "arraykey" => None,
                _ => Some("String"),
            },
            _ => None,
        },
    }
}
