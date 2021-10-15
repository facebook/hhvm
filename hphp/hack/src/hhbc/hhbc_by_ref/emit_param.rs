// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::{Maybe, Nothing, Slice, Str};
use hhbc_by_ref_ast_scope::Scope;
use hhbc_by_ref_emit_attribute as emit_attribute;
use hhbc_by_ref_emit_expression as emit_expression;
use hhbc_by_ref_emit_fatal as emit_fatal;
use hhbc_by_ref_emit_pos as emit_pos;
use hhbc_by_ref_emit_type_hint::{hint_to_type_info, Kind};
use hhbc_by_ref_env::{emitter::Emitter, Env};
use hhbc_by_ref_hhas_param::HhasParam;
use hhbc_by_ref_hhas_type::HhasTypeInfo;
use hhbc_by_ref_hhbc_string_utils::locals::strip_dollar;
use hhbc_by_ref_instruction_sequence::{instr, InstrSeq, Result};
use hhbc_by_ref_label::Label;
use hhbc_by_ref_local::Local;
use hhbc_by_ref_options::LangFlags;
use oxidized::{
    aast_defs::{Hint, Hint_},
    aast_visitor::{self, AstParams, Node},
    ast as a,
    ast_defs::{Id, ReadonlyKind},
    pos::Pos,
};

use naming_special_names_rust::user_attributes as ua;

use std::collections::{BTreeMap, BTreeSet};
use std::marker::PhantomData;

pub fn from_asts<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    tparams: &mut Vec<&str>,
    generate_defaults: bool,
    scope: &Scope<'a, 'arena>,
    ast_params: &[a::FunParam],
) -> Result<Vec<(HhasParam<'arena>, Option<(Label, a::Expr)>)>> {
    ast_params
        .iter()
        .map(|param| from_ast(alloc, emitter, tparams, generate_defaults, scope, param))
        .collect::<Result<Vec<_>>>()
        .map(|params| {
            params
                .iter()
                .filter_map(|p| p.to_owned())
                .collect::<Vec<_>>()
        })
        .map(|params| rename_params(alloc, params))
}

fn rename_params<'arena>(
    alloc: &'arena bumpalo::Bump,
    mut params: Vec<(HhasParam<'arena>, Option<(Label, a::Expr)>)>,
) -> Vec<(HhasParam<'arena>, Option<(Label, a::Expr)>)> {
    fn rename<'arena>(
        alloc: &'arena bumpalo::Bump,
        names: &BTreeSet<Str<'arena>>,
        param_counts: &mut BTreeMap<Str<'arena>, usize>,
        param: &mut HhasParam<'arena>,
    ) {
        match param_counts.get_mut(&param.name) {
            None => {
                param_counts.insert(param.name, 0);
            }
            Some(count) => {
                let newname = Str::new_str(alloc, format!("{}{}", param.name.as_str(), count));
                *count += 1;
                if names.contains(&newname) {
                    rename(alloc, names, param_counts, param);
                } else {
                    param.name = newname;
                }
            }
        }
    }
    let mut param_counts = BTreeMap::new();
    let names = params
        .iter()
        .map(|(p, _)| p.name.clone())
        .collect::<BTreeSet<_>>();
    params
        .iter_mut()
        .rev()
        .for_each(|(p, _)| rename(alloc, &names, &mut param_counts, p));
    params.into_iter().collect()
}

fn from_ast<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena, 'decl>,
    tparams: &mut Vec<&str>,
    generate_defaults: bool,
    scope: &Scope<'a, 'arena>,
    param: &a::FunParam,
) -> Result<Option<(HhasParam<'arena>, Option<(Label, a::Expr)>)>> {
    if param.is_variadic && param.name == "..." {
        return Ok(None);
    };
    if param.is_variadic {
        tparams.push("array");
    };
    let nullable = param
        .expr
        .as_ref()
        .map_or(false, |a::Expr(_, _, e)| e.is_null());
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
                Id(_, s) => s == ua::VIA_LABEL,
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
                alloc,
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
            .map(|expr| (emitter.label_gen_mut().next_default_arg(), expr.clone()))
    } else {
        None
    };
    let is_readonly = match param.readonly {
        Some(ReadonlyKind::Readonly) => true,
        _ => false,
    };
    let attrs = emit_attribute::from_asts(alloc, emitter, &param.user_attributes)?;
    Ok(Some((
        HhasParam {
            name: Str::new_str(alloc, &param.name),
            is_variadic: param.is_variadic,
            is_inout: param.callconv.is_pinout(),
            is_readonly,
            user_attributes: Slice::new(alloc.alloc_slice_fill_iter(attrs.into_iter())).into(),
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
    env: &Env<'a, 'arena>,
    pos: &Pos,
    params: &[(HhasParam<'arena>, Option<(Label, a::Expr)>)],
) -> Result<(InstrSeq<'arena>, InstrSeq<'arena>)> {
    let alloc = env.arena;
    let mut to_setter = |(param, default_value): &(HhasParam<'arena>, Option<(Label, a::Expr)>)| {
        default_value.as_ref().map(|(lbl, expr)| {
            let instrs = InstrSeq::gather(
                alloc,
                vec![
                    emit_expression::emit_expr(emitter, env, &expr)?,
                    emit_pos::emit_pos(alloc, pos),
                    instr::setl(
                        alloc,
                        Local::Named(Str::new_str(alloc, param.name.as_str())),
                    ),
                    instr::popc(alloc),
                ],
            );
            Ok(InstrSeq::gather(
                alloc,
                vec![instr::label(alloc, lbl.to_owned()), instrs],
            ))
        })
    };
    let setters = params
        .iter()
        .filter_map(|p| Option::from(to_setter(p)))
        .collect::<Result<Vec<_>>>()?;
    if setters.is_empty() {
        Ok((instr::empty(alloc), instr::empty(alloc)))
    } else {
        let l = emitter.label_gen_mut().next_regular();
        Ok((
            instr::label(alloc, l),
            InstrSeq::gather(
                alloc,
                vec![InstrSeq::gather(alloc, setters), instr::jmpns(alloc, l)],
            ),
        ))
    }
}

struct ResolverVisitor<'a, 'arena: 'a, 'decl: 'a> {
    phantom_a: PhantomData<&'a ()>,
    phantom_b: PhantomData<&'arena ()>,
    phantom_c: PhantomData<&'decl ()>,
}

#[allow(dead_code)]
struct Ctx<'a, 'arena: 'a, 'decl: 'a> {
    emitter: &'a mut Emitter<'arena, 'decl>,
    scope: &'a Scope<'a, 'arena>,
}

impl<'ast, 'a, 'arena, 'decl> aast_visitor::Visitor<'ast> for ResolverVisitor<'a, 'arena, 'decl> {
    type P = AstParams<Ctx<'a, 'arena, 'decl>, ()>;

    fn object(&mut self) -> &mut dyn aast_visitor::Visitor<'ast, P = Self::P> {
        self
    }

    fn visit_expr(
        &mut self,
        c: &mut Ctx<'a, 'arena, 'decl>,
        p: &a::Expr,
    ) -> std::result::Result<(), ()> {
        p.recurse(c, self.object())
        // TODO(hrust) implement on_CIexpr & remove dead_code on struct Ctx
    }
}

// Return None if it passes type check, otherwise return error msg
fn default_type_check<'arena>(
    param_name: &str,
    param_type_info: &Option<HhasTypeInfo<'arena>>,
    param_expr: &Option<a::Expr>,
) -> Option<String> {
    let hint_type = get_hint_display_name(
        param_type_info
            .as_ref()
            .and_then(|ti| ti.user_type.as_ref().into()),
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

fn get_hint_display_name<'arena>(hint: Option<&Str<'arena>>) -> Option<&'static str> {
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
