// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use indexmap::IndexMap;
use std::{cell::Cell, collections::hash_map::RandomState, iter::FromIterator};

use ast_class_expr_rust as ast_class_expr;
use ast_scope_rust as ast_scope;
use env::emitter::Emitter;
use hhbc_id_rust::Id;
use hhbc_string_utils_rust as string_utils;
use naming_special_names_rust::{math, members, special_functions, typehints};
use oxidized::{
    aast,
    aast_visitor::{visit_mut, AstParams, NodeMut, VisitorMut},
    ast as tast, ast_defs,
    namespace_env::Env as Namespace,
    pos::Pos,
};
use runtime::TypedValue;

use itertools::Itertools;

#[derive(Debug, PartialEq, Eq)]
pub enum Error {
    NotLiteral,
    UserDefinedConstant,
}

enum Radix {
    Oct,
    Hex,
    Dec,
    Bin,
}

fn radix(s: &str) -> Radix {
    let s = s.as_bytes();
    if s.len() > 1 && (s[0] as char) == '0' {
        match s[1] as char {
            'b' | 'B' => Radix::Bin,
            'x' | 'X' => Radix::Hex,
            _ => Radix::Oct,
        }
    } else {
        Radix::Dec
    }
}

fn try_type_intlike(s: &str) -> Option<i64> {
    match radix(s) {
        Radix::Dec => s.parse().ok(),
        Radix::Bin => i64::from_str_radix(&s[2..], 2).ok(),
        Radix::Oct => {
            let mut i = 1;
            let sb = s.as_bytes();
            // Ocaml's version truncate if any digit is greater then 7.
            while i < sb.len() {
                if sb[i] >= b'0' && sb[i] <= b'7' {
                    i += 1;
                } else {
                    break;
                }
            }
            if i > 1 {
                let sb = &sb[1..i];
                i64::from_str_radix(std::str::from_utf8(sb).unwrap(), 8).ok()
            } else {
                Some(0)
            }
        }
        Radix::Hex => i64::from_str_radix(&s[2..], 16).ok(),
    }
}

fn class_const_to_typed_value(
    emitter: &Emitter,
    cid: &tast::ClassId,
    id: &tast::Pstring,
) -> Result<TypedValue, Error> {
    if id.1 == members::M_CLASS {
        let cexpr = ast_class_expr::ClassExpr::class_id_to_class_expr(
            emitter,
            false,
            true,
            &ast_scope::Scope::toplevel(),
            cid,
        );
        if let ast_class_expr::ClassExpr::Id(ast_defs::Id(_, cname)) = cexpr {
            let cname = hhbc_id_rust::class::Type::from_ast_name(&cname).into();
            return Ok(TypedValue::String(cname));
        }
    }
    Err(Error::UserDefinedConstant)
}

fn array_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    fields: &Vec<tast::Afield>,
) -> Result<TypedValue, Error> {
    Ok(TypedValue::Array(if fields.is_empty() {
        vec![]
    } else {
        let mut res = vec![];
        let maxindex = Cell::new(0);

        let update_max_index = |mut newindex| {
            if newindex >= maxindex.get() {
                newindex += 1;
                maxindex.set(newindex);
            }
        };
        let default = |key, value| {
            let k_tv = key_expr_to_typed_value(emitter, ns, key)?;
            if let TypedValue::Int(newindex) = k_tv {
                update_max_index(newindex)
            }
            Ok((k_tv, expr_to_typed_value(emitter, ns, value)?))
        };
        for field in fields {
            res.push(match field {
                tast::Afield::AFkvalue(key, value) => default(key, value)?,
                tast::Afield::AFvalue(value) => {
                    let index = maxindex.get();
                    maxindex.set(index + 1);
                    (
                        TypedValue::Int(index),
                        expr_to_typed_value(emitter, ns, value)?,
                    )
                }
            })
        }
        res
    }))
}

fn varray_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    fields: &Vec<tast::Expr>,
    pos: &ast_defs::Pos,
) -> Result<TypedValue, Error> {
    let tv_fields: Result<Vec<TypedValue>, Error> = fields
        .iter()
        .map(|x| expr_to_typed_value(emitter, ns, x))
        .collect();
    Ok(TypedValue::VArray((tv_fields?, Some(pos.clone()))))
}

fn darray_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    fields: &Vec<(tast::Expr, tast::Expr)>,
    pos: &ast_defs::Pos,
) -> Result<TypedValue, Error> {
    let tv_fields: Vec<(TypedValue, TypedValue)> = fields
        .iter()
        .map(|(k, v)| {
            Ok((
                key_expr_to_typed_value(emitter, ns, k)?,
                expr_to_typed_value(emitter, ns, v)?,
            ))
        })
        .collect::<Result<_, Error>>()?;
    Ok(TypedValue::DArray((
        update_duplicates_in_map(tv_fields),
        Some(pos.clone()),
    )))
}

fn set_afield_to_typed_value_pair(
    e: &Emitter,
    ns: &Namespace,
    afield: &tast::Afield,
) -> Result<(TypedValue, TypedValue), Error> {
    match afield {
        tast::Afield::AFvalue(v) => {
            let tv = key_expr_to_typed_value(e, ns, v)?;
            Ok((tv.clone(), tv))
        }
        _ => panic!("set_afield_to_typed_value_pair: unexpected key=>value"),
    }
}

fn afield_to_typed_value_pair(
    emitter: &Emitter,
    ns: &Namespace,
    afield: &tast::Afield,
) -> Result<(TypedValue, TypedValue), Error> {
    match afield {
        tast::Afield::AFvalue(_) => panic!("afield_to_typed_value_pair: unexpected value"),
        tast::Afield::AFkvalue(key, value) => Ok((
            key_expr_to_typed_value(emitter, ns, key)?,
            expr_to_typed_value(emitter, ns, value)?,
        )),
    }
}

fn value_afield_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    afield: &tast::Afield,
) -> Result<TypedValue, Error> {
    match afield {
        tast::Afield::AFvalue(e) => expr_to_typed_value(emitter, ns, e),
        tast::Afield::AFkvalue(_, _) => {
            panic!("value_afield_to_typed_value: unexpected key=>value")
        }
    }
}

fn key_expr_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    expr: &tast::Expr,
) -> Result<TypedValue, Error> {
    let tv = expr_to_typed_value(emitter, ns, expr)?;
    match tv {
        TypedValue::Int(_) | TypedValue::String(_) => Ok(tv),
        _ => Err(Error::NotLiteral),
    }
}

fn keyset_value_afield_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    afield: &tast::Afield,
) -> Result<TypedValue, Error> {
    let tv = value_afield_to_typed_value(emitter, ns, afield)?;
    match tv {
        TypedValue::Int(_) | TypedValue::String(_) => Ok(tv),
        _ => Err(Error::NotLiteral),
    }
}

fn shape_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    fields: &Vec<(tast::ShapeFieldName, tast::Expr)>,
    pos: &ast_defs::Pos,
) -> Result<TypedValue, Error> {
    let a = fields
        .iter()
        .map(|(sf, expr)| {
            let key = match sf {
                ast_defs::ShapeFieldName::SFlitInt(_) => unimplemented!(),
                ast_defs::ShapeFieldName::SFlitStr(id) => TypedValue::String(id.1.clone()),
                ast_defs::ShapeFieldName::SFclassConst(_, _) => unimplemented!(),
            };
            Ok((key, expr_to_typed_value(emitter, ns, expr)?))
        })
        .collect::<Result<_, _>>()?;
    Ok(TypedValue::DArray((a, Some(pos.clone()))))
}

pub fn vec_to_typed_value(
    e: &Emitter,
    ns: &Namespace,
    pos: &Pos,
    fields: &[tast::Afield],
) -> Result<TypedValue, Error> {
    Ok(TypedValue::Vec((
        fields
            .iter()
            .map(|f| value_afield_to_typed_value(e, ns, f))
            .collect::<Result<_, _>>()?,
        Some(pos.clone()),
    )))
}

pub fn expr_to_typed_value(
    e: &Emitter,
    ns: &Namespace,
    expr: &tast::Expr,
) -> Result<TypedValue, Error> {
    expr_to_typed_value_(e, ns, expr, false /*allow_maps*/)
}

pub fn expr_to_typed_value_(
    emitter: &Emitter,
    ns: &Namespace,
    expr: &tast::Expr,
    allow_maps: bool,
) -> Result<TypedValue, Error> {
    use aast::Expr_::*;
    // TODO: ML equivalent has this as an implicit parameter that defaults to false.
    let pos = &expr.0;
    match &expr.1 {
        Int(s) => Ok(TypedValue::Int(
            try_type_intlike(&s).unwrap_or(std::i64::MAX),
        )),
        tast::Expr_::True => Ok(TypedValue::Bool(true)),
        False => Ok(TypedValue::Bool(false)),
        Null => Ok(TypedValue::Null),
        String(s) => Ok(TypedValue::String(s.to_owned())),
        Float(s) => s
            .parse()
            .map(|x| TypedValue::float(x))
            .map_err(|_| Error::NotLiteral),

        Call(id)
            if id
                .1
                .as_id()
                .map(|x| x.1 == special_functions::HHAS_ADATA)
                .unwrap_or(false) =>
        {
            unimplemented!()
        }

        Array(fields) => array_to_typed_value(emitter, ns, &fields),
        Varray(fields) => varray_to_typed_value(emitter, ns, &fields.1, pos),
        Darray(fields) => darray_to_typed_value(emitter, ns, &fields.1, pos),

        Id(id) if id.1 == math::NAN => Ok(TypedValue::float(std::f64::NAN)),
        Id(id) if id.1 == math::INF => Ok(TypedValue::float(std::f64::INFINITY)),
        Id(_) => Err(Error::UserDefinedConstant),

        Collection(x) if x.0.name().eq("vec") => vec_to_typed_value(emitter, ns, pos, &x.2),
        Collection(x) if x.0.name().eq("keyset") => Ok(TypedValue::Keyset(
            x.2.iter()
                .map(|x| keyset_value_afield_to_typed_value(emitter, ns, x))
                .collect::<Result<Vec<_>, _>>()?
                .into_iter()
                .unique()
                .collect(),
        )),
        Collection(x)
            if x.0.name().eq("dict")
                || allow_maps
                    && (string_utils::cmp(&(x.0).1, "Map", false, true)
                        || string_utils::cmp(&(x.0).1, "ImmMap", false, true)) =>
        {
            let values =
                x.2.iter()
                    .map(|x| afield_to_typed_value_pair(emitter, ns, x))
                    .collect::<Result<_, _>>()?;
            Ok(TypedValue::Dict((
                update_duplicates_in_map(values),
                Some(pos.clone()),
            )))
        }
        Collection(x)
            if allow_maps
                && (string_utils::cmp(&(x.0).1, "Set", false, true)
                    || string_utils::cmp(&(x.0).1, "ImmSet", false, true)) =>
        {
            let values =
                x.2.iter()
                    .map(|x| set_afield_to_typed_value_pair(emitter, ns, x))
                    .collect::<Result<_, _>>()?;
            Ok(TypedValue::Dict((
                update_duplicates_in_map(values),
                Some(pos.clone()),
            )))
        }
        ValCollection(x) if x.0 == tast::VcKind::Vec || x.0 == tast::VcKind::Vector => {
            unimplemented!()
        }
        ValCollection(x) if x.0 == tast::VcKind::Keyset => unimplemented!(),
        ValCollection(x) if x.0 == tast::VcKind::Set || x.0 == tast::VcKind::ImmSet => {
            unimplemented!()
        }

        KeyValCollection(_) => unimplemented!(),

        Shape(fields) => shape_to_typed_value(emitter, ns, fields, pos),
        ClassConst(x) => class_const_to_typed_value(emitter, &x.0, &x.1),
        BracedExpr(x) => expr_to_typed_value_(emitter, ns, x, allow_maps),
        ClassGet(_) => Err(Error::UserDefinedConstant),
        As(x) if (x.1).1.is_hlike() => expr_to_typed_value_(emitter, ns, &x.0, allow_maps),
        _ => Err(Error::NotLiteral),
    }
}

fn update_duplicates_in_map(kvs: Vec<(TypedValue, TypedValue)>) -> Vec<(TypedValue, TypedValue)> {
    IndexMap::<_, _, RandomState>::from_iter(kvs.into_iter())
        .into_iter()
        .collect()
}

fn cast_value(hint: &tast::Hint_, v: TypedValue) -> Result<TypedValue, Error> {
    match hint {
        tast::Hint_::Happly(ast_defs::Id(_, id), args) if args.is_empty() => {
            let id = string_utils::strip_hh_ns(id);
            if id == typehints::BOOL {
                v.cast_to_bool()
            } else if id == typehints::STRING {
                v.cast_to_string()
            } else if id == typehints::FLOAT {
                v.cast_to_float()
            } else {
                None
            }
        }
        _ => None,
    }
    .ok_or(Error::NotLiteral)
}

fn unop_on_value(unop: &ast_defs::Uop, v: TypedValue) -> Result<TypedValue, Error> {
    match unop {
        ast_defs::Uop::Unot => v.not(),
        ast_defs::Uop::Uplus => v.add(&TypedValue::Int(0)),
        ast_defs::Uop::Uminus => v.neg(),
        ast_defs::Uop::Utild => v.bitwise_not(),
        ast_defs::Uop::Usilence => Some(v.clone()),
        _ => None,
    }
    .ok_or(Error::NotLiteral)
}

fn binop_on_values(
    binop: &ast_defs::Bop,
    v1: TypedValue,
    v2: TypedValue,
) -> Result<TypedValue, Error> {
    use ast_defs::Bop::*;
    match binop {
        Dot => v1.concat(v2),
        Plus => v1.add(&v2),
        Minus => v1.sub(&v2),
        Star => v1.mul(&v2),
        Ltlt => v1.shift_left(&v2),
        Slash => v1.div(&v2),
        Bar => v1.bitwise_or(&v2),
        _ => None,
    }
    .ok_or(Error::NotLiteral)
}

fn value_to_expr(v: TypedValue) -> tast::Expr_ {
    use tast::*;
    use TypedValue::*;
    match v {
        Int(i) => Expr_::Int(i.to_string()),
        Float(i) => Expr_::Float(hhbc_string_utils_rust::float::to_string(i)),
        Bool(false) => Expr_::False,
        Bool(true) => Expr_::True,
        String(s) => Expr_::String(s),
        _ => panic!("TODO"),
    }
}

struct FolderVisitor<'a> {
    emitter: &'a Emitter,
    empty_namespace: &'a Namespace,
}

impl<'a> FolderVisitor<'a> {
    fn new(emitter: &'a Emitter, empty_namespace: &'a Namespace) -> Self {
        Self {
            emitter,
            empty_namespace,
        }
    }
}

impl VisitorMut for FolderVisitor<'_> {
    type P = AstParams<(), ()>;

    fn object(&mut self) -> &mut dyn VisitorMut<P = Self::P> {
        self
    }

    fn visit_expr_(&mut self, c: &mut (), p: &mut tast::Expr_) -> Result<(), ()> {
        p.recurse(c, self.object())?;
        let new_p = match p {
            tast::Expr_::Cast(e) => expr_to_typed_value(self.emitter, self.empty_namespace, &e.1)
                .and_then(|v| cast_value(&(e.0).1, v))
                .map(value_to_expr)
                .ok(),
            tast::Expr_::Unop(e) => expr_to_typed_value(self.emitter, self.empty_namespace, &e.1)
                .and_then(|v| unop_on_value(&e.0, v))
                .map(value_to_expr)
                .ok(),
            tast::Expr_::Binop(e) => expr_to_typed_value(self.emitter, self.empty_namespace, &e.1)
                .and_then(|v1| {
                    expr_to_typed_value(self.emitter, self.empty_namespace, &e.2)
                        .and_then(|v2| binop_on_values(&e.0, v1, v2).map(value_to_expr))
                })
                .ok(),
            _ => None,
        };
        if let Some(new_p) = new_p {
            *p = new_p
        }
        Ok(())
    }
}

pub fn fold_expr(expr: &mut tast::Expr, e: &mut Emitter, empty_namespace: &Namespace) {
    visit_mut(&mut FolderVisitor::new(e, empty_namespace), &mut (), expr).unwrap();
}

pub fn fold_program(p: &mut tast::Program, e: &mut Emitter, empty_namespace: &Namespace) {
    visit_mut(&mut FolderVisitor::new(e, empty_namespace), &mut (), p).unwrap();
}

pub fn literals_from_exprs(
    ns: &Namespace,
    exprs: &mut [tast::Expr],
    e: &mut Emitter,
) -> Result<Vec<TypedValue>, Error> {
    for expr in exprs.iter_mut() {
        fold_expr(expr, e, ns);
    }
    let ret = exprs
        .iter()
        .map(|expr| expr_to_typed_value(e, ns, expr))
        .collect();
    if let Err(Error::NotLiteral) = ret {
        panic!("literals_from_exprs: not literal");
    }
    ret
}
