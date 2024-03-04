// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::marker::PhantomData;

use ast_scope::Scope;
use emit_type_hint::hint_to_type_info;
use emit_type_hint::Kind;
use env::emitter::Emitter;
use env::Env;
use error::Error;
use error::Result;
use ffi::Maybe;
use ffi::Nothing;
use hhbc::Label;
use hhbc::Local;
use hhbc::Param;
use hhbc::StringIdMap;
use hhbc::StringIdSet;
use hhbc::TypeInfo;
use hhbc_string_utils::locals::strip_dollar;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::aast_visitor;
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::Node;
use oxidized::ast as a;
use oxidized::ast_defs::Id;
use oxidized::ast_defs::ReadonlyKind;
use oxidized::pos::Pos;

use crate::emit_attribute;
use crate::emit_expression;

pub fn has_variadic(params: &[Param]) -> bool {
    params.iter().any(|v| v.is_variadic)
}

pub fn from_asts<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    tparams: &mut Vec<&str>,
    generate_defaults: bool,
    scope: &Scope<'a>,
    ast_params: &[a::FunParam],
) -> Result<Vec<(Param, Option<(Label, a::Expr)>)>> {
    ast_params
        .iter()
        .map(|param| from_ast(emitter, tparams, generate_defaults, scope, param))
        .collect::<Result<Vec<_>>>()
        .map(|params| {
            params
                .iter()
                .filter_map(|p| p.to_owned())
                .collect::<Vec<_>>()
        })
        .map(rename_params)
}

fn rename_params<'arena>(
    mut params: Vec<(Param, Option<(Label, a::Expr)>)>,
) -> Vec<(Param, Option<(Label, a::Expr)>)> {
    let mut param_counts = StringIdMap::default();
    let names: StringIdSet = params.iter().map(|(p, _)| p.name).collect();
    for (param, _) in params.iter_mut() {
        use std::collections::hash_map::Entry;
        'inner: loop {
            match param_counts.entry(param.name) {
                Entry::Vacant(e) => {
                    e.insert(0);
                }
                Entry::Occupied(mut e) => {
                    let newname = format!("{}{}", param.name, e.get());
                    let newname = hhbc::intern(newname);
                    *e.get_mut() += 1;
                    if names.contains(&newname) {
                        // collision - try again
                        continue 'inner;
                    } else {
                        param.name = newname;
                    }
                }
            }
            break 'inner;
        }
    }
    params.into_iter().collect()
}

fn from_ast<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    tparams: &mut Vec<&str>,
    generate_defaults: bool,
    scope: &Scope<'a>,
    param: &a::FunParam,
) -> Result<Option<(Param, Option<(Label, a::Expr)>)>> {
    if param.is_variadic && param.name == "..." {
        return Ok(None);
    };
    if param.is_variadic {
        tparams.push("array");
    };
    let type_info = {
        let param_type_hint = if param.is_variadic {
            Some(Hint(
                Pos::NONE,
                Box::new(Hint_::mk_happly(
                    Id(Pos::NONE, "array".to_string()),
                    param
                        .type_hint
                        .get_hint()
                        .as_ref()
                        .map_or(vec![], |h| vec![h.clone()]),
                )),
            ))
        } else {
            param.type_hint.get_hint().clone()
        };
        if let Some(h) = param_type_hint {
            Some(hint_to_type_info(
                emitter.alloc,
                &Kind::Param,
                false,
                false, /* meaning only set nullable based on given hint */
                &tparams[..],
                &h,
            )?)
        } else {
            None
        }
    };
    // Do the type check for default value type and hint type
    if let Some(err_msg) = default_type_check(&param.name, type_info.as_ref(), param.expr.as_ref())
    {
        return Err(Error::fatal_parse(&param.pos, err_msg));
    }
    aast_visitor::visit(
        &mut ResolverVisitor {
            phantom_a: PhantomData,
            phantom_b: PhantomData,
            phantom_c: PhantomData,
        },
        &mut Ctx { emitter, scope },
        &param.expr,
    )
    .unwrap();
    let default_value = if generate_defaults {
        param
            .expr
            .as_ref()
            .map(|expr| (emitter.label_gen_mut().next_regular(), expr.clone()))
    } else {
        None
    };
    let is_readonly = match param.readonly {
        Some(ReadonlyKind::Readonly) => true,
        _ => false,
    };
    let attrs = emit_attribute::from_asts(emitter, &param.user_attributes)?;
    Ok(Some((
        Param {
            name: hhbc::intern(&param.name),
            is_variadic: param.is_variadic,
            is_inout: param.callconv.is_pinout(),
            is_readonly,
            user_attributes: attrs.into(),
            type_info: Maybe::from(type_info),
            // - Write hhas_param.default_value as `Nothing` while keeping `default_value` around
            //   for emitting decl vars and default value setters
            // - emit_body::make_body will rewrite hhas_param.default_value using `default_value`
            default_value: Nothing,
        },
        default_value,
    )))
}

pub fn emit_param_default_value_setter<'a, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    env: &Env<'a>,
    pos: &Pos,
    params: &[(Param, Option<(Label, a::Expr)>)],
) -> Result<(InstrSeq, InstrSeq)> {
    let setters = params
        .iter()
        .enumerate()
        .filter_map(|(i, (_, dv))| {
            // LocalIds for params are numbered from 0.
            dv.as_ref().map(|(lbl, expr)| {
                let param_local = Local::new(i);
                let instrs = InstrSeq::gather(vec![
                    emit_expression::emit_expr(emitter, env, expr)?,
                    emit_pos::emit_pos(pos),
                    instr::verify_param_type(param_local),
                    instr::set_l(param_local),
                    instr::pop_c(),
                ]);
                Ok(InstrSeq::gather(vec![instr::label(lbl.to_owned()), instrs]))
            })
        })
        .collect::<Result<Vec<_>>>()?;
    if setters.is_empty() {
        Ok((instr::empty(), instr::empty()))
    } else {
        let l = emitter.label_gen_mut().next_regular();
        Ok((
            instr::label(l),
            InstrSeq::gather(vec![InstrSeq::gather(setters), instr::enter(l)]),
        ))
    }
}

struct ResolverVisitor<'a, 'arena: 'a, 'decl: 'a> {
    phantom_a: PhantomData<&'a ()>,
    phantom_b: PhantomData<&'arena ()>,
    phantom_c: PhantomData<&'decl ()>,
}

#[allow(dead_code)]
struct Ctx<'a, 'arena, 'decl> {
    emitter: &'a mut Emitter<'arena, 'decl>,
    scope: &'a Scope<'a>,
}

impl<'ast, 'a, 'arena, 'decl> aast_visitor::Visitor<'ast> for ResolverVisitor<'a, 'arena, 'decl> {
    type Params = AstParams<Ctx<'a, 'arena, 'decl>, ()>;

    fn object(&mut self) -> &mut dyn aast_visitor::Visitor<'ast, Params = Self::Params> {
        self
    }

    fn visit_expr(&mut self, c: &mut Ctx<'a, 'arena, 'decl>, p: &a::Expr) -> Result<(), ()> {
        p.recurse(c, self.object())
        // TODO(hrust) implement on_CIexpr & remove dead_code on struct Ctx
    }
}

// Return None if it passes type check, otherwise return error msg
fn default_type_check<'arena>(
    param_name: &str,
    param_type_info: Option<&TypeInfo>,
    param_expr: Option<&a::Expr>,
) -> Option<String> {
    let hint_type = get_hint_display_name(
        param_type_info
            .as_ref()
            .and_then(|ti| ti.user_type.as_ref().map(|s| s.as_str()).into()),
    );
    // If matches, return None, otherwise return default_type
    let default_type = hint_type.and_then(|ht| match_default_and_hint(ht, param_expr));
    let param_true_name = strip_dollar(param_name);
    default_type.and_then(|dt| hint_type.map(|ht| match ht {
        "class" => format!(
            "Default value for parameter {} with a class type hint can only be NULL",
            param_true_name),
        _ => format!(
            "Default value for parameter {} with type {} needs to have the same type as the type hint {}",
            param_true_name,
            dt,
            ht)
    }))
}

fn get_hint_display_name(hint: Option<&str>) -> Option<&'static str> {
    hint.map(|h| match h {
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
fn match_default_and_hint(hint_type: &str, param_expr: Option<&a::Expr>) -> Option<&'static str> {
    if hint_type == "class" {
        return None;
    }
    match &param_expr.as_ref() {
        None => None,
        Some(e) => match e.2 {
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
