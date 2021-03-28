// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use indexmap::IndexMap;
use std::{collections::hash_map::RandomState, fmt, iter::FromIterator};

use ast_class_expr_rust as ast_class_expr;
use ast_scope_rust as ast_scope;
use env::emitter::Emitter;
use hhbc_id_rust::Id;
use hhbc_string_utils_rust as string_utils;
use naming_special_names_rust::{math, members, special_functions, typehints};
use options::HhvmFlags;
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
    Unrecoverable(String),
}

impl Error {
    fn unrecoverable(s: impl Into<String>) -> Self {
        Self::Unrecoverable(s.into())
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::NotLiteral => write!(f, "NotLiteral"),
            Self::UserDefinedConstant => write!(f, "UserDefinedConstant"),
            Self::Unrecoverable(msg) => write!(f, "{}", msg),
        }
    }
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
            if emitter.options().emit_class_pointers() == 2 {
                let cname = hhbc_id_rust::class::Type::from_ast_name_and_mangle(&cname).into();
                return Ok(TypedValue::LazyClass(cname));
            } else {
                let cname = hhbc_id_rust::class::Type::from_ast_name(&cname).into();
                return Ok(TypedValue::String(cname));
            }
        }
    }
    Err(Error::UserDefinedConstant)
}

fn varray_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    fields: &Vec<tast::Expr>,
) -> Result<TypedValue, Error> {
    let tv_fields: Vec<TypedValue> = fields
        .iter()
        .map(|x| expr_to_typed_value(emitter, ns, x))
        .collect::<Result<_, Error>>()?;
    Ok(TypedValue::Vec(tv_fields))
}

fn darray_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    fields: &Vec<(tast::Expr, tast::Expr)>,
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
    Ok(TypedValue::Dict(update_duplicates_in_map(tv_fields)))
}

fn set_afield_to_typed_value_pair(
    e: &Emitter,
    ns: &Namespace,
    afield: &tast::Afield,
) -> Result<(TypedValue, TypedValue), Error> {
    match afield {
        tast::Afield::AFvalue(v) => set_afield_value_to_typed_value_pair(e, ns, v),
        _ => Err(Error::unrecoverable(
            "set_afield_to_typed_value_pair: unexpected key=>value",
        )),
    }
}

fn set_afield_value_to_typed_value_pair(
    e: &Emitter,
    ns: &Namespace,
    v: &tast::Expr,
) -> Result<(TypedValue, TypedValue), Error> {
    let tv = key_expr_to_typed_value(e, ns, v)?;
    Ok((tv.clone(), tv))
}

fn afield_to_typed_value_pair(
    emitter: &Emitter,
    ns: &Namespace,
    afield: &tast::Afield,
) -> Result<(TypedValue, TypedValue), Error> {
    match afield {
        tast::Afield::AFvalue(_) => Err(Error::unrecoverable(
            "afield_to_typed_value_pair: unexpected value",
        )),
        tast::Afield::AFkvalue(key, value) => kv_to_typed_value_pair(emitter, ns, key, value),
    }
}

fn kv_to_typed_value_pair(
    emitter: &Emitter,
    ns: &Namespace,
    key: &tast::Expr,
    value: &tast::Expr,
) -> Result<(TypedValue, TypedValue), Error> {
    Ok((
        key_expr_to_typed_value(emitter, ns, key)?,
        expr_to_typed_value(emitter, ns, value)?,
    ))
}

fn value_afield_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    afield: &tast::Afield,
) -> Result<TypedValue, Error> {
    match afield {
        tast::Afield::AFvalue(e) => expr_to_typed_value(emitter, ns, e),
        tast::Afield::AFkvalue(_, _) => Err(Error::unrecoverable(
            "value_afield_to_typed_value: unexpected key=>value",
        )),
    }
}

fn key_expr_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    expr: &tast::Expr,
) -> Result<TypedValue, Error> {
    let tv = expr_to_typed_value(emitter, ns, expr)?;
    let fold_lc = emitter
        .options()
        .hhvm
        .flags
        .contains(HhvmFlags::FOLD_LAZY_CLASS_KEYS);
    match tv {
        TypedValue::Int(_) | TypedValue::String(_) => Ok(tv),
        TypedValue::LazyClass(_) if fold_lc => Ok(tv),
        _ => Err(Error::NotLiteral),
    }
}

fn keyset_value_afield_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    afield: &tast::Afield,
) -> Result<TypedValue, Error> {
    let tv = value_afield_to_typed_value(emitter, ns, afield)?;
    let fold_lc = emitter
        .options()
        .hhvm
        .flags
        .contains(HhvmFlags::FOLD_LAZY_CLASS_KEYS);
    match tv {
        TypedValue::Int(_) | TypedValue::String(_) => Ok(tv),
        TypedValue::LazyClass(_) if fold_lc => Ok(tv),
        _ => Err(Error::NotLiteral),
    }
}

fn shape_to_typed_value(
    emitter: &Emitter,
    ns: &Namespace,
    fields: &Vec<(tast::ShapeFieldName, tast::Expr)>,
) -> Result<TypedValue, Error> {
    let a = fields
        .iter()
        .map(|(sf, expr)| {
            let key = match sf {
                ast_defs::ShapeFieldName::SFlitInt((_, s)) => {
                    let tv = int_expr_to_typed_value(s)?;
                    match tv {
                        TypedValue::Int(_) => tv,
                        _ => {
                            return Err(Error::unrecoverable(format!(
                                "{} is not a valid integer index",
                                s
                            )));
                        }
                    }
                }
                ast_defs::ShapeFieldName::SFlitStr(id) => {
                    // FIXME: This is not safe--string literals are binary
                    // strings. There's no guarantee that they're valid UTF-8.
                    TypedValue::String(unsafe { String::from_utf8_unchecked(id.1.clone().into()) })
                }
                ast_defs::ShapeFieldName::SFclassConst(class_id, id) => class_const_to_typed_value(
                    emitter,
                    &tast::ClassId(Pos::make_none(), tast::ClassId_::CI(class_id.clone())),
                    id,
                )?,
            };
            Ok((key, expr_to_typed_value(emitter, ns, expr)?))
        })
        .collect::<Result<_, _>>()?;
    Ok(TypedValue::Dict(a))
}

pub fn vec_to_typed_value(
    e: &Emitter,
    ns: &Namespace,
    fields: &[tast::Afield],
) -> Result<TypedValue, Error> {
    Ok(TypedValue::Vec(
        fields
            .iter()
            .map(|f| value_afield_to_typed_value(e, ns, f))
            .collect::<Result<_, _>>()?,
    ))
}

pub fn expr_to_typed_value(
    e: &Emitter,
    ns: &Namespace,
    expr: &tast::Expr,
) -> Result<TypedValue, Error> {
    expr_to_typed_value_(
        e, ns, expr, false, /*allow_maps*/
        false, /*force_class_const*/
    )
}

pub fn expr_to_typed_value_(
    emitter: &Emitter,
    ns: &Namespace,
    expr: &tast::Expr,
    allow_maps: bool,
    force_class_const: bool,
) -> Result<TypedValue, Error> {
    use aast::Expr_::*;
    // TODO: ML equivalent has this as an implicit parameter that defaults to false.
    match &expr.1 {
        Int(s) => int_expr_to_typed_value(s),
        tast::Expr_::True => Ok(TypedValue::Bool(true)),
        False => Ok(TypedValue::Bool(false)),
        Null => Ok(TypedValue::Null),
        String(s) => {
            // FIXME: This is not safe--string literals are binary strings.
            // There's no guarantee that they're valid UTF-8.
            Ok(TypedValue::String(unsafe {
                std::string::String::from_utf8_unchecked(s.clone().into())
            }))
        }
        EnumAtom(s) => Ok(TypedValue::String(s.clone())),
        Float(s) => {
            if s == math::INF {
                Ok(TypedValue::float(std::f64::INFINITY))
            } else if s == math::NEG_INF {
                Ok(TypedValue::float(std::f64::NEG_INFINITY))
            } else if s == math::NAN {
                Ok(TypedValue::float(std::f64::NAN))
            } else {
                s.parse()
                    .map(|x| TypedValue::float(x))
                    .map_err(|_| Error::NotLiteral)
            }
        }

        Call(id)
            if id
                .0
                .as_id()
                .map(|x| x.1 == special_functions::HHAS_ADATA)
                .unwrap_or(false) =>
        {
            match &id.2[..] {
                &[tast::Expr(_, tast::Expr_::String(ref data))] => {
                    // FIXME: This is not safe--string literals are binary strings.
                    // There's no guarantee that they're valid UTF-8.
                    Ok(TypedValue::HhasAdata(unsafe {
                        std::string::String::from_utf8_unchecked(data.clone().into())
                    }))
                }
                _ => Err(Error::NotLiteral),
            }
        }

        Varray(fields) => varray_to_typed_value(emitter, ns, &fields.1),
        Darray(fields) => darray_to_typed_value(emitter, ns, &fields.1),

        Id(id) if id.1 == math::NAN => Ok(TypedValue::float(std::f64::NAN)),
        Id(id) if id.1 == math::INF => Ok(TypedValue::float(std::f64::INFINITY)),
        Id(_) => Err(Error::UserDefinedConstant),

        Collection(x) if x.0.name().eq("vec") => vec_to_typed_value(emitter, ns, &x.2),
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
            Ok(TypedValue::Dict(update_duplicates_in_map(values)))
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
            Ok(TypedValue::Dict(update_duplicates_in_map(values)))
        }
        ValCollection(x) if x.0 == tast::VcKind::Vec || x.0 == tast::VcKind::Vector => {
            Ok(TypedValue::Vec(
                x.2.iter()
                    .map(|e| expr_to_typed_value(emitter, ns, e))
                    .collect::<Result<_, _>>()?,
            ))
        }
        ValCollection(x) if x.0 == tast::VcKind::Keyset => Ok(TypedValue::Keyset(
            x.2.iter()
                .map(|e| {
                    expr_to_typed_value(emitter, ns, e).and_then(|tv| match tv {
                        TypedValue::Int(_) | TypedValue::String(_) => Ok(tv),
                        TypedValue::LazyClass(_)
                            if emitter
                                .options()
                                .hhvm
                                .flags
                                .contains(HhvmFlags::FOLD_LAZY_CLASS_KEYS) =>
                        {
                            Ok(tv)
                        }
                        _ => Err(Error::NotLiteral),
                    })
                })
                .collect::<Result<Vec<_>, _>>()?
                .into_iter()
                .unique()
                .collect(),
        )),
        ValCollection(x) if x.0 == tast::VcKind::Set || x.0 == tast::VcKind::ImmSet => {
            let values =
                x.2.iter()
                    .map(|e| set_afield_value_to_typed_value_pair(emitter, ns, e))
                    .collect::<Result<_, _>>()?;
            Ok(TypedValue::Dict(update_duplicates_in_map(values)))
        }

        KeyValCollection(x) => {
            let values =
                x.2.iter()
                    .map(|e| kv_to_typed_value_pair(emitter, ns, &e.0, &e.1))
                    .collect::<Result<_, _>>()?;
            Ok(TypedValue::Dict(update_duplicates_in_map(values)))
        }

        Shape(fields) => shape_to_typed_value(emitter, ns, fields),
        ClassConst(x) => {
            if emitter.options().emit_class_pointers() == 1 && !force_class_const {
                Err(Error::NotLiteral)
            } else {
                class_const_to_typed_value(emitter, &x.0, &x.1)
            }
        }
        ClassGet(_) => Err(Error::UserDefinedConstant),
        As(x) if (x.1).1.is_hlike() => expr_to_typed_value_(emitter, ns, &x.0, allow_maps, false),
        _ => Err(Error::NotLiteral),
    }
}

fn int_expr_to_typed_value(s: &str) -> Result<TypedValue, Error> {
    Ok(TypedValue::Int(
        try_type_intlike(&s).unwrap_or(std::i64::MAX),
    ))
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

fn value_to_expr_(v: TypedValue) -> Result<tast::Expr_, Error> {
    use tast::*;
    use TypedValue::*;
    Ok(match v {
        Int(i) => Expr_::Int(i.to_string()),
        Float(i) => Expr_::Float(hhbc_string_utils_rust::float::to_string(i)),
        Bool(false) => Expr_::False,
        Bool(true) => Expr_::True,
        String(s) => Expr_::String(s.into()),
        LazyClass(_) => return Err(Error::unrecoverable("value_to_expr: lazyclass NYI")),
        Null => Expr_::Null,
        Uninit => return Err(Error::unrecoverable("value_to_expr: uninit value")),
        Vec(_) => return Err(Error::unrecoverable("value_to_expr: vec NYI")),
        Keyset(_) => return Err(Error::unrecoverable("value_to_expr: keyset NYI")),
        HhasAdata(_) => return Err(Error::unrecoverable("value_to_expr: HhasAdata NYI")),
        Dict(_) => return Err(Error::unrecoverable("value_to_expr: dict NYI")),
    })
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

impl<'ast> VisitorMut<'ast> for FolderVisitor<'_> {
    type P = AstParams<(), Error>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr_(&mut self, c: &mut (), p: &mut tast::Expr_) -> Result<(), Error> {
        p.recurse(c, self.object())?;
        let new_p = match p {
            tast::Expr_::Cast(e) => expr_to_typed_value(self.emitter, self.empty_namespace, &e.1)
                .and_then(|v| cast_value(&(e.0).1, v))
                .map(value_to_expr_)
                .ok(),
            tast::Expr_::Unop(e) => expr_to_typed_value(self.emitter, self.empty_namespace, &e.1)
                .and_then(|v| unop_on_value(&e.0, v))
                .map(value_to_expr_)
                .ok(),
            tast::Expr_::Binop(e) => expr_to_typed_value(self.emitter, self.empty_namespace, &e.1)
                .and_then(|v1| {
                    expr_to_typed_value(self.emitter, self.empty_namespace, &e.2)
                        .and_then(|v2| binop_on_values(&e.0, v1, v2).map(value_to_expr_))
                })
                .ok(),
            _ => None,
        };
        if let Some(new_p) = new_p {
            *p = new_p?
        }
        Ok(())
    }
}

pub fn fold_expr(
    expr: &mut tast::Expr,
    e: &mut Emitter,
    empty_namespace: &Namespace,
) -> Result<(), Error> {
    visit_mut(&mut FolderVisitor::new(e, empty_namespace), &mut (), expr)
}

pub fn fold_program(
    p: &mut tast::Program,
    e: &mut Emitter,
    empty_namespace: &Namespace,
) -> Result<(), Error> {
    visit_mut(&mut FolderVisitor::new(e, empty_namespace), &mut (), p)
}

pub fn literals_from_exprs(
    ns: &Namespace,
    exprs: &mut [tast::Expr],
    e: &mut Emitter,
) -> Result<Vec<TypedValue>, Error> {
    for expr in exprs.iter_mut() {
        fold_expr(expr, e, ns)?;
    }
    let ret = exprs
        .iter()
        .map(|expr| expr_to_typed_value_(e, ns, expr, false, true))
        .collect();
    if let Err(Error::NotLiteral) = ret {
        Err(Error::unrecoverable("literals_from_exprs: not literal"))
    } else {
        ret
    }
}
