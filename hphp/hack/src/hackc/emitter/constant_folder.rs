// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::hash_map::RandomState;
use std::fmt;

use ast_scope::Scope;
use ast_scope::ScopeItem;
use env::emitter::Emitter;
use env::ClassExpr;
use ffi::Str;
use hhbc::DictEntry;
use hhbc::TypedValue;
use hhbc_string_utils as string_utils;
use indexmap::IndexMap;
use itertools::Itertools;
use naming_special_names_rust::math;
use naming_special_names_rust::members;
use naming_special_names_rust::typehints;
use oxidized::aast_visitor::visit_mut;
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::NodeMut;
use oxidized::aast_visitor::VisitorMut;
use oxidized::ast;
use oxidized::ast_defs;
use oxidized::pos::Pos;

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

fn class_const_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    cid: &ast::ClassId,
    id: &ast::Pstring,
) -> Result<TypedValue<'arena>, Error> {
    if id.1 == members::M_CLASS {
        let cexpr = ClassExpr::class_id_to_class_expr(emitter, scope, false, true, cid);
        if let ClassExpr::Id(ast_defs::Id(_, cname)) = cexpr {
            let classid =
                hhbc::ClassName::from_ast_name_and_mangle(emitter.alloc, cname).as_ffi_str();
            return Ok(TypedValue::LazyClass(classid));
        }
    }
    Err(Error::UserDefinedConstant)
}

fn varray_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    fields: &[ast::Expr],
) -> Result<TypedValue<'arena>, Error> {
    let tv_fields = emitter.alloc.alloc_slice_fill_iter(
        fields
            .iter()
            .map(|x| expr_to_typed_value(emitter, scope, x))
            .collect::<Result<Vec<_>, _>>()?
            .into_iter(),
    );
    Ok(TypedValue::vec(tv_fields))
}

fn darray_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    fields: &[(ast::Expr, ast::Expr)],
) -> Result<TypedValue<'arena>, Error> {
    //TODO: Improve. It's a bit silly having to use a std::vector::Vec
    // here.
    let tv_fields: Vec<(TypedValue<'arena>, TypedValue<'arena>)> = fields
        .iter()
        .map(|(k, v)| {
            Ok((
                key_expr_to_typed_value(emitter, scope, k)?,
                expr_to_typed_value(emitter, scope, v)?,
            ))
        })
        .collect::<Result<_, Error>>()?;
    Ok(TypedValue::dict(emitter.alloc.alloc_slice_fill_iter(
        update_duplicates_in_map(tv_fields),
    )))
}

fn set_afield_to_typed_value_pair<'arena, 'decl>(
    e: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    afield: &ast::Afield,
) -> Result<(TypedValue<'arena>, TypedValue<'arena>), Error> {
    match afield {
        ast::Afield::AFvalue(v) => set_afield_value_to_typed_value_pair(e, scope, v),
        _ => Err(Error::unrecoverable(
            "set_afield_to_typed_value_pair: unexpected key=>value",
        )),
    }
}

fn set_afield_value_to_typed_value_pair<'arena, 'decl>(
    e: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    v: &ast::Expr,
) -> Result<(TypedValue<'arena>, TypedValue<'arena>), Error> {
    let tv = key_expr_to_typed_value(e, scope, v)?;
    Ok((tv.clone(), tv))
}

fn afield_to_typed_value_pair<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    afield: &ast::Afield,
) -> Result<(TypedValue<'arena>, TypedValue<'arena>), Error> {
    match afield {
        ast::Afield::AFvalue(_) => Err(Error::unrecoverable(
            "afield_to_typed_value_pair: unexpected value",
        )),
        ast::Afield::AFkvalue(key, value) => kv_to_typed_value_pair(emitter, scope, key, value),
    }
}

fn kv_to_typed_value_pair<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    key: &ast::Expr,
    value: &ast::Expr,
) -> Result<(TypedValue<'arena>, TypedValue<'arena>), Error> {
    Ok((
        key_expr_to_typed_value(emitter, scope, key)?,
        expr_to_typed_value(emitter, scope, value)?,
    ))
}

fn value_afield_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    afield: &ast::Afield,
) -> Result<TypedValue<'arena>, Error> {
    match afield {
        ast::Afield::AFvalue(e) => expr_to_typed_value(emitter, scope, e),
        ast::Afield::AFkvalue(_, _) => Err(Error::unrecoverable(
            "value_afield_to_typed_value: unexpected key=>value",
        )),
    }
}

fn key_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    expr: &ast::Expr,
) -> Result<TypedValue<'arena>, Error> {
    let tv = expr_to_typed_value(emitter, scope, expr)?;
    let fold_lc = emitter.options().hhbc.fold_lazy_class_keys;
    match tv {
        TypedValue::Int(_) | TypedValue::String(_) => Ok(tv),
        TypedValue::LazyClass(_) if fold_lc => Ok(tv),
        _ => Err(Error::NotLiteral),
    }
}

fn keyset_value_afield_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    afield: &ast::Afield,
) -> Result<TypedValue<'arena>, Error> {
    let tv = value_afield_to_typed_value(emitter, scope, afield)?;
    let fold_lc = emitter.options().hhbc.fold_lazy_class_keys;
    match tv {
        TypedValue::Int(_) | TypedValue::String(_) => Ok(tv),
        TypedValue::LazyClass(_) if fold_lc => Ok(tv),
        _ => Err(Error::NotLiteral),
    }
}

fn shape_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    fields: &[(ast::ShapeFieldName, ast::Expr)],
) -> Result<TypedValue<'arena>, Error> {
    let a = emitter.alloc.alloc_slice_fill_iter(
        fields
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
                        TypedValue::string(
                            emitter
                                .alloc
                                .alloc_str(unsafe { std::str::from_utf8_unchecked(&id.1) }),
                        )
                    }
                    ast_defs::ShapeFieldName::SFclassConst(class_id, id) => {
                        class_const_to_typed_value(
                            emitter,
                            scope,
                            &ast::ClassId((), Pos::NONE, ast::ClassId_::CI(class_id.clone())),
                            id,
                        )?
                    }
                };
                let value = expr_to_typed_value(emitter, scope, expr)?;
                Ok(DictEntry { key, value })
            })
            .collect::<Result<Vec<_>, _>>()?
            .into_iter(),
    );
    Ok(TypedValue::dict(a))
}

pub fn vec_to_typed_value<'arena, 'decl>(
    e: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    fields: &[ast::Afield],
) -> Result<TypedValue<'arena>, Error> {
    //TODO: Improve. It's a bit silly having to use a std::vector::Vec
    // here.
    let tv_fields: Result<Vec<TypedValue<'arena>>, Error> = fields
        .iter()
        .map(|f| value_afield_to_typed_value(e, scope, f))
        .collect();
    let fields = e.alloc.alloc_slice_fill_iter(tv_fields?.into_iter());
    Ok(TypedValue::vec(fields))
}

pub fn expr_to_typed_value<'arena, 'decl>(
    e: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    expr: &ast::Expr,
) -> Result<TypedValue<'arena>, Error> {
    expr_to_typed_value_(e, scope, expr, false /*allow_maps*/)
}

pub fn expr_to_typed_value_<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    expr: &ast::Expr,
    allow_maps: bool,
) -> Result<TypedValue<'arena>, Error> {
    stack_limit::maybe_grow(|| {
        // TODO: ML equivalent has this as an implicit parameter that defaults to false.
        use ast::Expr_;
        match &expr.2 {
            Expr_::Int(s) => int_expr_to_typed_value(s),
            Expr_::True => Ok(TypedValue::Bool(true)),
            Expr_::False => Ok(TypedValue::Bool(false)),
            Expr_::Null => Ok(TypedValue::Null),
            Expr_::String(s) => string_expr_to_typed_value(emitter, s),
            Expr_::Float(s) => float_expr_to_typed_value(emitter, s),

            Expr_::Varray(fields) => varray_to_typed_value(emitter, scope, &fields.1),
            Expr_::Darray(fields) => darray_to_typed_value(emitter, scope, &fields.1),

            Expr_::Id(id) if id.1 == math::NAN => Ok(TypedValue::float(std::f64::NAN)),
            Expr_::Id(id) if id.1 == math::INF => Ok(TypedValue::float(std::f64::INFINITY)),
            Expr_::Id(_) => Err(Error::UserDefinedConstant),

            Expr_::Collection(x) if x.0.name().eq("keyset") => {
                keyset_expr_to_typed_value(emitter, scope, x)
            }
            Expr_::Collection(x)
                if x.0.name().eq("dict")
                    || allow_maps
                        && (string_utils::cmp(&(x.0).1, "Map", false, true)
                            || string_utils::cmp(&(x.0).1, "ImmMap", false, true)) =>
            {
                dict_expr_to_typed_value(emitter, scope, x)
            }
            Expr_::Collection(x)
                if allow_maps
                    && (string_utils::cmp(&(x.0).1, "Set", false, true)
                        || string_utils::cmp(&(x.0).1, "ImmSet", false, true)) =>
            {
                set_expr_to_typed_value(emitter, scope, x)
            }
            Expr_::Tuple(x) => tuple_expr_to_typed_value(emitter, scope, x),
            Expr_::ValCollection(x)
                if x.0.1 == ast::VcKind::Vec || x.0.1 == ast::VcKind::Vector =>
            {
                valcollection_vec_expr_to_typed_value(emitter, scope, x)
            }
            Expr_::ValCollection(x) if x.0.1 == ast::VcKind::Keyset => {
                valcollection_keyset_expr_to_typed_value(emitter, scope, x)
            }
            Expr_::ValCollection(x)
                if x.0.1 == ast::VcKind::Set || x.0.1 == ast::VcKind::ImmSet =>
            {
                valcollection_set_expr_to_typed_value(emitter, scope, x)
            }
            Expr_::KeyValCollection(x) => keyvalcollection_expr_to_typed_value(emitter, scope, x),
            Expr_::Shape(fields) => shape_to_typed_value(emitter, scope, fields),
            Expr_::ClassConst(x) => class_const_to_typed_value(emitter, scope, &x.0, &x.1),

            Expr_::ClassGet(_) => Err(Error::UserDefinedConstant),
            ast::Expr_::As(x) if (x.1).1.is_hlike() => {
                expr_to_typed_value_(emitter, scope, &x.0, allow_maps)
            }
            Expr_::Upcast(e) => expr_to_typed_value(emitter, scope, &e.0),
            _ => Err(Error::NotLiteral),
        }
    })
}

fn valcollection_keyset_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    x: &((Pos, ast::VcKind), Option<ast::Targ>, Vec<ast::Expr>),
) -> Result<TypedValue<'arena>, Error> {
    let keys = emitter.alloc.alloc_slice_fill_iter(
        x.2.iter()
            .map(|e| {
                expr_to_typed_value(emitter, scope, e).and_then(|tv| match tv {
                    TypedValue::Int(_) | TypedValue::String(_) => Ok(tv),
                    TypedValue::LazyClass(_) if emitter.options().hhbc.fold_lazy_class_keys => {
                        Ok(tv)
                    }
                    _ => Err(Error::NotLiteral),
                })
            })
            .collect::<Result<Vec<_>, _>>()?
            .into_iter()
            .unique()
            .collect::<Vec<_>>()
            .into_iter(),
    );
    Ok(TypedValue::keyset(keys))
}

fn keyvalcollection_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    x: &(
        (Pos, ast::KvcKind),
        Option<(ast::Targ, ast::Targ)>,
        Vec<ast::Field>,
    ),
) -> Result<TypedValue<'arena>, Error> {
    let values = emitter
        .alloc
        .alloc_slice_fill_iter(update_duplicates_in_map(
            x.2.iter()
                .map(|e| kv_to_typed_value_pair(emitter, scope, &e.0, &e.1))
                .collect::<Result<Vec<_>, _>>()?,
        ));
    Ok(TypedValue::dict(values))
}

fn valcollection_set_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    x: &((Pos, ast::VcKind), Option<ast::Targ>, Vec<ast::Expr>),
) -> Result<TypedValue<'arena>, Error> {
    let values = emitter
        .alloc
        .alloc_slice_fill_iter(update_duplicates_in_map(
            x.2.iter()
                .map(|e| set_afield_value_to_typed_value_pair(emitter, scope, e))
                .collect::<Result<Vec<_>, _>>()?,
        ));
    Ok(TypedValue::dict(values))
}

fn valcollection_vec_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    x: &((Pos, ast::VcKind), Option<ast::Targ>, Vec<ast::Expr>),
) -> Result<TypedValue<'arena>, Error> {
    let v: Vec<_> =
        x.2.iter()
            .map(|e| expr_to_typed_value(emitter, scope, e))
            .collect::<Result<_, _>>()?;
    Ok(TypedValue::vec(
        emitter.alloc.alloc_slice_fill_iter(v.into_iter()),
    ))
}

fn tuple_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    x: &[ast::Expr],
) -> Result<TypedValue<'arena>, Error> {
    let v: Vec<_> = x
        .iter()
        .map(|e| expr_to_typed_value(emitter, scope, e))
        .collect::<Result<_, _>>()?;
    Ok(TypedValue::vec(
        emitter.alloc.alloc_slice_fill_iter(v.into_iter()),
    ))
}

fn set_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    x: &(
        ast::ClassName,
        Option<ast::CollectionTarg>,
        Vec<ast::Afield>,
    ),
) -> Result<TypedValue<'arena>, Error> {
    let values = emitter
        .alloc
        .alloc_slice_fill_iter(update_duplicates_in_map(
            x.2.iter()
                .map(|x| set_afield_to_typed_value_pair(emitter, scope, x))
                .collect::<Result<_, _>>()?,
        ));
    Ok(TypedValue::dict(values))
}

fn dict_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    x: &(
        ast::ClassName,
        Option<ast::CollectionTarg>,
        Vec<ast::Afield>,
    ),
) -> Result<TypedValue<'arena>, Error> {
    let values = emitter
        .alloc
        .alloc_slice_fill_iter(update_duplicates_in_map(
            x.2.iter()
                .map(|x| afield_to_typed_value_pair(emitter, scope, x))
                .collect::<Result<_, _>>()?,
        ));
    Ok(TypedValue::dict(values))
}

fn keyset_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    scope: &Scope<'_, 'arena>,
    x: &(
        ast::ClassName,
        Option<ast::CollectionTarg>,
        Vec<ast::Afield>,
    ),
) -> Result<TypedValue<'arena>, Error> {
    let keys = emitter.alloc.alloc_slice_fill_iter(
        x.2.iter()
            .map(|x| keyset_value_afield_to_typed_value(emitter, scope, x))
            .collect::<Result<Vec<_>, _>>()?
            .into_iter()
            .unique()
            .collect::<Vec<_>>()
            .into_iter(),
    );
    Ok(TypedValue::keyset(keys))
}

fn float_expr_to_typed_value<'arena, 'decl>(
    _emitter: &Emitter<'arena, 'decl>,
    s: &str,
) -> Result<TypedValue<'arena>, Error> {
    if s == math::INF {
        Ok(TypedValue::float(std::f64::INFINITY))
    } else if s == math::NEG_INF {
        Ok(TypedValue::float(std::f64::NEG_INFINITY))
    } else if s == math::NAN {
        Ok(TypedValue::float(std::f64::NAN))
    } else {
        s.parse()
            .map(TypedValue::float)
            .map_err(|_| Error::NotLiteral)
    }
}

fn string_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    s: &[u8],
) -> Result<TypedValue<'arena>, Error> {
    // FIXME: This is not safe--string literals are binary strings.
    // There's no guarantee that they're valid UTF-8.
    Ok(TypedValue::string(
        emitter
            .alloc
            .alloc_str(unsafe { std::str::from_utf8_unchecked(s) }),
    ))
}

fn int_expr_to_typed_value<'arena>(s: &str) -> Result<TypedValue<'arena>, Error> {
    Ok(TypedValue::Int(
        try_type_intlike(s).unwrap_or(std::i64::MAX),
    ))
}

fn update_duplicates_in_map<'arena>(
    kvs: Vec<(TypedValue<'arena>, TypedValue<'arena>)>,
) -> impl IntoIterator<
    Item = DictEntry<'arena>,
    IntoIter = impl Iterator<Item = DictEntry<'arena>> + ExactSizeIterator + 'arena,
> + 'arena {
    kvs.into_iter()
        .collect::<IndexMap<_, _, RandomState>>()
        .into_iter()
        .map(|(key, value)| DictEntry { key, value })
}

fn cast_value<'arena>(
    alloc: &'arena bumpalo::Bump,
    hint: &ast::Hint_,
    v: TypedValue<'arena>,
) -> Result<TypedValue<'arena>, Error> {
    match hint {
        ast::Hint_::Happly(ast_defs::Id(_, id), args) if args.is_empty() => {
            let id = string_utils::strip_hh_ns(id);
            if id == typehints::BOOL {
                Some(TypedValue::Bool(cast_to_bool(v)))
            } else if id == typehints::STRING {
                cast_to_arena_str(v, alloc).map(TypedValue::string)
            } else if id == typehints::FLOAT {
                cast_to_float(v).map(TypedValue::float)
            } else {
                None
            }
        }
        _ => None,
    }
    .ok_or(Error::NotLiteral)
}

fn unop_on_value<'arena>(
    unop: &ast_defs::Uop,
    v: TypedValue<'arena>,
) -> Result<TypedValue<'arena>, Error> {
    match unop {
        ast_defs::Uop::Unot => fold_logical_not(v),
        ast_defs::Uop::Uplus => fold_add(v, TypedValue::Int(0)),
        ast_defs::Uop::Uminus => match v {
            TypedValue::Int(i) => Some(TypedValue::Int((-std::num::Wrapping(i)).0)),
            TypedValue::Float(i) => Some(TypedValue::float(0.0 - i.to_f64())),
            _ => None,
        },
        ast_defs::Uop::Utild => fold_bitwise_not(v),
        ast_defs::Uop::Usilence => Some(v.clone()),
        _ => None,
    }
    .ok_or(Error::NotLiteral)
}

fn binop_on_values<'arena>(
    alloc: &'arena bumpalo::Bump,
    binop: &ast_defs::Bop,
    v1: TypedValue<'arena>,
    v2: TypedValue<'arena>,
) -> Result<TypedValue<'arena>, Error> {
    use ast_defs::Bop;
    match binop {
        Bop::Dot => fold_concat(v1, v2, alloc),
        Bop::Plus => fold_add(v1, v2),
        Bop::Minus => fold_sub(v1, v2),
        Bop::Star => fold_mul(v1, v2),
        Bop::Ltlt => fold_shift_left(v1, v2),
        Bop::Slash => fold_div(v1, v2),
        Bop::Bar => fold_bitwise_or(v1, v2),
        _ => None,
    }
    .ok_or(Error::NotLiteral)
}

fn value_to_expr_<'arena>(v: TypedValue<'arena>) -> Result<ast::Expr_, Error> {
    use ast::Expr_;
    match v {
        TypedValue::Int(i) => Ok(Expr_::Int(i.to_string())),
        TypedValue::Float(f) => Ok(Expr_::Float(hhbc_string_utils::float::to_string(
            f.to_f64(),
        ))),
        TypedValue::Bool(false) => Ok(Expr_::False),
        TypedValue::Bool(true) => Ok(Expr_::True),
        TypedValue::String(s) => Ok(Expr_::String(s.unsafe_as_str().into())),
        TypedValue::LazyClass(_) => Err(Error::unrecoverable("value_to_expr: lazyclass NYI")),
        TypedValue::Null => Ok(Expr_::Null),
        TypedValue::Uninit => Err(Error::unrecoverable("value_to_expr: uninit value")),
        TypedValue::Vec(_) => Err(Error::unrecoverable("value_to_expr: vec NYI")),
        TypedValue::Keyset(_) => Err(Error::unrecoverable("value_to_expr: keyset NYI")),
        TypedValue::Dict(_) => Err(Error::unrecoverable("value_to_expr: dict NYI")),
    }
}

struct FolderVisitorMut<'a, 'arena, 'decl> {
    emitter: &'a Emitter<'arena, 'decl>,
    scope: &'a mut Scope<'a, 'arena>,
}

impl<'a, 'arena, 'decl> FolderVisitorMut<'a, 'arena, 'decl> {
    fn new(emitter: &'a Emitter<'arena, 'decl>, scope: &'a mut Scope<'a, 'arena>) -> Self {
        Self { emitter, scope }
    }
}

struct FolderVisitor<'a, 'arena, 'decl> {
    emitter: &'a Emitter<'arena, 'decl>,
    scope: &'a Scope<'a, 'arena>,
}

impl<'a, 'arena, 'decl> FolderVisitor<'a, 'arena, 'decl> {
    fn new(emitter: &'a Emitter<'arena, 'decl>, scope: &'a Scope<'a, 'arena>) -> Self {
        Self { emitter, scope }
    }
}

impl<'ast, 'decl> VisitorMut<'ast> for FolderVisitor<'_, '_, 'decl> {
    type Params = AstParams<(), Error>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, Params = Self::Params> {
        self
    }

    fn visit_expr_(&mut self, c: &mut (), p: &mut ast::Expr_) -> Result<(), Error> {
        p.recurse(c, self.object())?;
        let new_p = match p {
            ast::Expr_::Cast(e) => expr_to_typed_value(self.emitter, self.scope, &e.1)
                .and_then(|v| cast_value(self.emitter.alloc, &(e.0).1, v))
                .map(value_to_expr_)
                .ok(),
            ast::Expr_::Unop(e) => expr_to_typed_value(self.emitter, self.scope, &e.1)
                .and_then(|v| unop_on_value(&e.0, v))
                .map(value_to_expr_)
                .ok(),
            ast::Expr_::Binop(binop) => expr_to_typed_value(self.emitter, self.scope, &binop.lhs)
                .and_then(|v1| {
                    expr_to_typed_value(self.emitter, self.scope, &binop.rhs).and_then(|v2| {
                        binop_on_values(self.emitter.alloc, &binop.bop, v1, v2).map(value_to_expr_)
                    })
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

impl<'ast, 'decl> VisitorMut<'ast> for FolderVisitorMut<'_, '_, 'decl> {
    type Params = AstParams<(), Error>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, Params = Self::Params> {
        self
    }
    fn visit_expr(&mut self, _c: &mut (), expr: &mut ast::Expr) -> Result<(), Error> {
        visit_mut(
            &mut FolderVisitor::new(self.emitter, self.scope),
            &mut (),
            expr,
        )
    }
    fn visit_fun_def(&mut self, c: &mut (), fd: &mut ast::FunDef) -> Result<(), Error> {
        self.scope
            .push_item(ScopeItem::Function(ast_scope::Fun::new_rc(fd)));
        fd.recurse(c, self.object())?;
        self.scope.pop_item();
        Ok(())
    }
    fn visit_method_(&mut self, c: &mut (), m: &mut ast::Method_) -> Result<(), Error> {
        self.scope
            .push_item(ScopeItem::Method(ast_scope::Method::new_rc(m)));
        // TODO(T88847409) May need different handling for closures
        m.recurse(c, self.object())?;
        self.scope.pop_item();
        Ok(())
    }
    fn visit_class_(&mut self, c: &mut (), cls: &mut ast::Class_) -> Result<(), Error> {
        self.scope
            .push_item(ScopeItem::Class(ast_scope::Class::new_rc(cls)));
        cls.recurse(c, self.object())?;
        self.scope.pop_item();
        Ok(())
    }
}

pub fn fold_expr<'a, 'arena: 'a, 'decl: 'a>(
    expr: &mut ast::Expr,
    scope: &'a Scope<'a, 'arena>,
    e: &'a mut Emitter<'arena, 'decl>,
) -> Result<(), Error> {
    visit_mut(&mut FolderVisitor::new(e, scope), &mut (), expr)
}

pub fn fold_program<'arena, 'decl>(
    p: &mut ast::Program,
    e: &mut Emitter<'arena, 'decl>,
) -> Result<(), Error> {
    visit_mut(
        &mut FolderVisitorMut::new(e, &mut ast_scope::Scope::default()),
        &mut (),
        p,
    )
}

pub fn literals_from_exprs<'arena, 'decl>(
    exprs: &mut [ast::Expr],
    scope: &Scope<'_, 'arena>,
    e: &mut Emitter<'arena, 'decl>,
) -> Result<Vec<TypedValue<'arena>>, Error> {
    for expr in exprs.iter_mut() {
        fold_expr(expr, scope, e)?;
    }
    let ret = exprs
        .iter()
        .map(|expr| expr_to_typed_value_(e, scope, expr, false))
        .collect();
    if let Err(Error::NotLiteral) = ret {
        Err(Error::unrecoverable("literals_from_exprs: not literal"))
    } else {
        ret
    }
}

fn cast_to_arena_str<'a>(x: TypedValue<'a>, alloc: &'a bumpalo::Bump) -> Option<Str<'a>> {
    match x {
        TypedValue::Uninit => None, // Should not happen
        TypedValue::Bool(false) => Some("".into()),
        TypedValue::Bool(true) => Some("1".into()),
        TypedValue::Null => Some("".into()),
        TypedValue::Int(i) => Some(alloc.alloc_str(i.to_string().as_str()).into()),
        TypedValue::String(s) => Some(s),
        TypedValue::LazyClass(s) => Some(s),
        _ => None,
    }
}

// Arithmetic. Only on pure integer or float operands
// and don't attempt to implement overflow-to-float semantics.
fn fold_add<'a>(x: TypedValue<'a>, y: TypedValue<'a>) -> Option<TypedValue<'a>> {
    match (x, y) {
        (TypedValue::Float(i1), TypedValue::Float(i2)) => {
            Some(TypedValue::float(i1.to_f64() + i2.to_f64()))
        }
        (TypedValue::Int(i1), TypedValue::Int(i2)) => Some(TypedValue::Int(
            (std::num::Wrapping(i1) + std::num::Wrapping(i2)).0,
        )),
        (TypedValue::Int(i1), TypedValue::Float(i2)) => {
            Some(TypedValue::float(i1 as f64 + i2.to_f64()))
        }
        (TypedValue::Float(i1), TypedValue::Int(i2)) => {
            Some(TypedValue::float(i1.to_f64() + i2 as f64))
        }
        _ => None,
    }
}

// Arithmetic. Only on pure integer or float operands,
// and don't attempt to implement overflow-to-float semantics.
fn fold_sub<'a>(x: TypedValue<'a>, y: TypedValue<'a>) -> Option<TypedValue<'a>> {
    match (x, y) {
        (TypedValue::Int(i1), TypedValue::Int(i2)) => Some(TypedValue::Int(
            (std::num::Wrapping(i1) - std::num::Wrapping(i2)).0,
        )),
        (TypedValue::Float(f1), TypedValue::Float(f2)) => {
            Some(TypedValue::float(f1.to_f64() - f2.to_f64()))
        }
        _ => None,
    }
}

// Arithmetic. Only on pure integer or float operands
// and don't attempt to implement overflow-to-float semantics.
fn fold_mul<'a>(x: TypedValue<'a>, y: TypedValue<'a>) -> Option<TypedValue<'a>> {
    match (x, y) {
        (TypedValue::Int(i1), TypedValue::Int(i2)) => Some(TypedValue::Int(
            (std::num::Wrapping(i1) * std::num::Wrapping(i2)).0,
        )),
        (TypedValue::Float(i1), TypedValue::Float(i2)) => {
            Some(TypedValue::float(i1.to_f64() * i2.to_f64()))
        }
        (TypedValue::Int(i1), TypedValue::Float(i2)) => {
            Some(TypedValue::float(i1 as f64 * i2.to_f64()))
        }
        (TypedValue::Float(i1), TypedValue::Int(i2)) => {
            Some(TypedValue::float(i1.to_f64() * i2 as f64))
        }
        _ => None,
    }
}

// Arithmetic. Only on pure integer or float operands
// and don't attempt to implement overflow-to-float semantics.
fn fold_div<'a>(x: TypedValue<'a>, y: TypedValue<'a>) -> Option<TypedValue<'a>> {
    match (x, y) {
        (TypedValue::Int(i1), TypedValue::Int(i2)) if i2 != 0 && i1 % i2 == 0 => {
            Some(TypedValue::Int(i1 / i2))
        }
        (TypedValue::Int(i1), TypedValue::Int(i2)) if i2 != 0 => {
            Some(TypedValue::float(i1 as f64 / i2 as f64))
        }
        (TypedValue::Float(f1), TypedValue::Float(f2)) if f2.to_f64() != 0.0 => {
            Some(TypedValue::float(f1.to_f64() / f2.to_f64()))
        }
        (TypedValue::Int(i1), TypedValue::Float(f2)) if f2.to_f64() != 0.0 => {
            Some(TypedValue::float(i1 as f64 / f2.to_f64()))
        }
        (TypedValue::Float(f1), TypedValue::Int(i2)) if i2 != 0 => {
            Some(TypedValue::float(f1.to_f64() / i2 as f64))
        }
        _ => None,
    }
}

fn fold_shift_left<'a>(x: TypedValue<'a>, y: TypedValue<'a>) -> Option<TypedValue<'a>> {
    match (x, y) {
        (TypedValue::Int(_), TypedValue::Int(i2)) if i2 < 0 => None,
        (TypedValue::Int(i1), TypedValue::Int(i2)) => i32::try_from(i2)
            .ok()
            .map(|i2| TypedValue::Int(i1 << (i2 % 64) as u32)),
        _ => None,
    }
}

// Arithmetic, only on pure integer operands.
fn fold_bitwise_or<'a>(x: TypedValue<'a>, y: TypedValue<'a>) -> Option<TypedValue<'a>> {
    match (x, y) {
        (TypedValue::Int(i1), TypedValue::Int(i2)) => Some(TypedValue::Int(i1 | i2)),
        _ => None,
    }
}

// String concatenation
fn fold_concat<'a>(
    x: TypedValue<'a>,
    y: TypedValue<'a>,
    alloc: &'a bumpalo::Bump,
) -> Option<TypedValue<'a>> {
    fn safe_to_cast(t: &TypedValue<'_>) -> bool {
        matches!(
            t,
            TypedValue::Int(_) | TypedValue::String(_) | TypedValue::LazyClass(_)
        )
    }
    if !safe_to_cast(&x) || !safe_to_cast(&y) {
        return None;
    }

    let l = cast_to_string(x)?;
    let r = cast_to_string(y)?;
    Some(TypedValue::alloc_string(l + &r, alloc))
}

// Bitwise operations.
fn fold_bitwise_not<'a>(x: TypedValue<'a>) -> Option<TypedValue<'a>> {
    match x {
        TypedValue::Int(i) => Some(TypedValue::Int(!i)),
        _ => None,
    }
}

fn fold_logical_not<'a>(x: TypedValue<'a>) -> Option<TypedValue<'a>> {
    Some(TypedValue::Bool(!cast_to_bool(x)))
}

/// Cast to a boolean: the (bool) operator in PHP
pub fn cast_to_bool(x: TypedValue<'_>) -> bool {
    match x {
        TypedValue::Uninit => false, // Should not happen
        TypedValue::Bool(b) => b,
        TypedValue::Null => false,
        TypedValue::String(s) => !s.is_empty() && s.unsafe_as_str() != "0",
        TypedValue::LazyClass(_) => true,
        TypedValue::Int(i) => i != 0,
        TypedValue::Float(f) => f.to_f64() != 0.0,
        // Empty collections cast to false if empty, otherwise true
        TypedValue::Vec(v) => !v.is_empty(),
        TypedValue::Keyset(v) => !v.is_empty(),
        TypedValue::Dict(v) => !v.is_empty(),
    }
}

/// Cast to an integer: the (int) operator in PHP. Return None if we can't
/// or won't produce the correct value
pub fn cast_to_int(x: TypedValue<'_>) -> Option<i64> {
    match x {
        TypedValue::Uninit => None, // Should not happen
        // Unreachable - the only calliste of to_int is cast_to_arraykey, which never
        // calls it with String
        TypedValue::String(_) => None,    // not worth it
        TypedValue::LazyClass(_) => None, // not worth it
        TypedValue::Int(i) => Some(i),
        TypedValue::Float(f) => match f.to_f64().classify() {
            std::num::FpCategory::Nan | std::num::FpCategory::Infinite => {
                Some(if f.to_f64() == f64::INFINITY {
                    0
                } else {
                    std::i64::MIN
                })
            }
            _ => todo!(),
        },
        v => Some(if cast_to_bool(v) { 1 } else { 0 }),
    }
}

/// Cast to a float: the (float) operator in PHP. Return None if we can't
/// or won't produce the correct value
pub fn cast_to_float(v: TypedValue<'_>) -> Option<f64> {
    match v {
        TypedValue::Uninit => None,       // Should not happen
        TypedValue::String(_) => None,    // not worth it
        TypedValue::LazyClass(_) => None, // not worth it
        TypedValue::Int(i) => Some(i as f64),
        TypedValue::Float(f) => Some(f.to_f64()),
        _ => Some(if cast_to_bool(v) { 1.0 } else { 0.0 }),
    }
}

/// Cast to a string: the (string) operator in PHP. Return Err if we can't
/// or won't produce the correct value *)
pub fn cast_to_string(x: TypedValue<'_>) -> Option<String> {
    match x {
        TypedValue::Uninit => None, // Should not happen
        TypedValue::Bool(false) => Some("".into()),
        TypedValue::Bool(true) => Some("1".into()),
        TypedValue::Null => Some("".into()),
        TypedValue::Int(i) => Some(i.to_string()),
        TypedValue::String(s) => Some(s.unsafe_as_str().into()),
        TypedValue::LazyClass(s) => Some(s.unsafe_as_str().into()),
        _ => None,
    }
}

#[cfg(test)]
mod cast_tests {
    use super::*;

    #[test]
    fn non_numeric_string_to_int() {
        let res = cast_to_int(TypedValue::string("foo"));
        assert!(res.is_none());
    }

    #[test]
    fn nan_to_int() {
        let res = cast_to_int(TypedValue::float(std::f64::NAN)).unwrap();
        assert_eq!(res, std::i64::MIN);
    }
}
