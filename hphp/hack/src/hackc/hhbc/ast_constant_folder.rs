// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use indexmap::IndexMap;
use std::{collections::hash_map::RandomState, fmt};

use env::emitter::Emitter;
use ffi::Pair;
use hhbc_string_utils as string_utils;
use naming_special_names_rust::{math, members, special_functions, typehints};
use options::HhvmFlags;
use oxidized::{
    aast_visitor::{visit_mut, AstParams, NodeMut, VisitorMut},
    ast, ast_defs,
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

fn class_const_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    cid: &ast::ClassId,
    id: &ast::Pstring,
) -> Result<TypedValue<'arena>, Error> {
    if id.1 == members::M_CLASS {
        let cexpr = ast_class_expr::ClassExpr::class_id_to_class_expr(
            emitter,
            false,
            true,
            &ast_scope::Scope::toplevel(),
            cid,
        );
        if let ast_class_expr::ClassExpr::Id(ast_defs::Id(_, cname)) = cexpr {
            let classid = hhbc_id::class::ClassType::from_ast_name_and_mangle(emitter.alloc, cname)
                .as_ffi_str();
            if emitter.options().emit_class_pointers() == 2 {
                return Ok(TypedValue::LazyClass(classid));
            } else {
                return Ok(TypedValue::String(classid));
            }
        }
    }
    Err(Error::UserDefinedConstant)
}

fn varray_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    fields: &[ast::Expr],
) -> Result<TypedValue<'arena>, Error> {
    let tv_fields = emitter.alloc.alloc_slice_fill_iter(
        fields
            .iter()
            .map(|x| expr_to_typed_value(emitter, x))
            .collect::<Result<Vec<_>, _>>()?
            .into_iter(),
    );
    Ok(TypedValue::mk_vec(tv_fields))
}

fn darray_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    fields: &[(ast::Expr, ast::Expr)],
) -> Result<TypedValue<'arena>, Error> {
    //TODO: Improve. It's a bit silly having to use a std::vector::Vec
    // here.
    let tv_fields: Vec<(TypedValue<'arena>, TypedValue<'arena>)> = fields
        .iter()
        .map(|(k, v)| {
            Ok((
                key_expr_to_typed_value(emitter, k)?,
                expr_to_typed_value(emitter, v)?,
            ))
        })
        .collect::<Result<_, Error>>()?;
    Ok(TypedValue::mk_dict(emitter.alloc.alloc_slice_fill_iter(
        update_duplicates_in_map(tv_fields),
    )))
}

fn set_afield_to_typed_value_pair<'arena, 'decl>(
    e: &Emitter<'arena, 'decl>,
    afield: &ast::Afield,
) -> Result<(TypedValue<'arena>, TypedValue<'arena>), Error> {
    match afield {
        ast::Afield::AFvalue(v) => set_afield_value_to_typed_value_pair(e, v),
        _ => Err(Error::unrecoverable(
            "set_afield_to_typed_value_pair: unexpected key=>value",
        )),
    }
}

fn set_afield_value_to_typed_value_pair<'arena, 'decl>(
    e: &Emitter<'arena, 'decl>,
    v: &ast::Expr,
) -> Result<(TypedValue<'arena>, TypedValue<'arena>), Error> {
    let tv = key_expr_to_typed_value(e, v)?;
    Ok((tv.clone(), tv))
}

fn afield_to_typed_value_pair<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    afield: &ast::Afield,
) -> Result<(TypedValue<'arena>, TypedValue<'arena>), Error> {
    match afield {
        ast::Afield::AFvalue(_) => Err(Error::unrecoverable(
            "afield_to_typed_value_pair: unexpected value",
        )),
        ast::Afield::AFkvalue(key, value) => kv_to_typed_value_pair(emitter, key, value),
    }
}

fn kv_to_typed_value_pair<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    key: &ast::Expr,
    value: &ast::Expr,
) -> Result<(TypedValue<'arena>, TypedValue<'arena>), Error> {
    Ok((
        key_expr_to_typed_value(emitter, key)?,
        expr_to_typed_value(emitter, value)?,
    ))
}

fn value_afield_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    afield: &ast::Afield,
) -> Result<TypedValue<'arena>, Error> {
    match afield {
        ast::Afield::AFvalue(e) => expr_to_typed_value(emitter, e),
        ast::Afield::AFkvalue(_, _) => Err(Error::unrecoverable(
            "value_afield_to_typed_value: unexpected key=>value",
        )),
    }
}

fn key_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    expr: &ast::Expr,
) -> Result<TypedValue<'arena>, Error> {
    let tv = expr_to_typed_value(emitter, expr)?;
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

fn keyset_value_afield_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    afield: &ast::Afield,
) -> Result<TypedValue<'arena>, Error> {
    let tv = value_afield_to_typed_value(emitter, afield)?;
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

fn shape_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
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
                        TypedValue::mk_string(
                            emitter
                                .alloc
                                .alloc_str(unsafe { std::str::from_utf8_unchecked(&id.1) }),
                        )
                    }
                    ast_defs::ShapeFieldName::SFclassConst(class_id, id) => {
                        class_const_to_typed_value(
                            emitter,
                            &ast::ClassId(
                                (),
                                Pos::make_none(),
                                ast::ClassId_::CI(class_id.clone()),
                            ),
                            id,
                        )?
                    }
                };
                Ok((key, expr_to_typed_value(emitter, expr)?).into())
            })
            .collect::<Result<Vec<Pair<_, _>>, _>>()?
            .into_iter(),
    );
    Ok(TypedValue::mk_dict(a))
}

pub fn vec_to_typed_value<'arena, 'decl>(
    e: &Emitter<'arena, 'decl>,
    fields: &[ast::Afield],
) -> Result<TypedValue<'arena>, Error> {
    //TODO: Improve. It's a bit silly having to use a std::vector::Vec
    // here.
    let tv_fields: Result<Vec<TypedValue<'arena>>, Error> = fields
        .iter()
        .map(|f| value_afield_to_typed_value(e, f))
        .collect();
    let fields = e.alloc.alloc_slice_fill_iter(tv_fields?.into_iter());
    Ok(TypedValue::mk_vec(fields))
}

pub fn expr_to_typed_value<'arena, 'decl>(
    e: &Emitter<'arena, 'decl>,
    expr: &ast::Expr,
) -> Result<TypedValue<'arena>, Error> {
    expr_to_typed_value_(
        e, expr, false, /*allow_maps*/
        false, /*force_class_const*/
    )
}

pub fn expr_to_typed_value_<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    expr: &ast::Expr,
    allow_maps: bool,
    force_class_const: bool,
) -> Result<TypedValue<'arena>, Error> {
    if let Some(sl) = emitter.stack_limit.as_ref() {
        sl.panic_if_exceeded();
    }
    // TODO: ML equivalent has this as an implicit parameter that defaults to false.
    use ast::Expr_;
    match &expr.2 {
        Expr_::Int(s) => int_expr_to_typed_value(s),
        Expr_::True => Ok(TypedValue::Bool(true)),
        Expr_::False => Ok(TypedValue::Bool(false)),
        Expr_::Null => Ok(TypedValue::Null),
        Expr_::String(s) => string_expr_to_typed_value(emitter, s),
        Expr_::Float(s) => float_expr_to_typed_value(emitter, s),
        Expr_::Call(id)
            if (id.0.as_id()).map_or(false, |x| x.1 == special_functions::HHAS_ADATA) =>
        {
            call_expr_to_typed_value(emitter, id)
        }

        Expr_::Varray(fields) => varray_to_typed_value(emitter, &fields.1),
        Expr_::Darray(fields) => darray_to_typed_value(emitter, &fields.1),

        Expr_::Id(id) if id.1 == math::NAN => Ok(TypedValue::double(std::f64::NAN)),
        Expr_::Id(id) if id.1 == math::INF => Ok(TypedValue::double(std::f64::INFINITY)),
        Expr_::Id(_) => Err(Error::UserDefinedConstant),

        Expr_::Collection(x) if x.0.name().eq("vec") => vec_to_typed_value(emitter, &x.2),
        Expr_::Collection(x) if x.0.name().eq("keyset") => keyset_expr_to_typed_value(emitter, x),
        Expr_::Collection(x)
            if x.0.name().eq("dict")
                || allow_maps
                    && (string_utils::cmp(&(x.0).1, "Map", false, true)
                        || string_utils::cmp(&(x.0).1, "ImmMap", false, true)) =>
        {
            dict_expr_to_typed_value(emitter, x)
        }
        Expr_::Collection(x)
            if allow_maps
                && (string_utils::cmp(&(x.0).1, "Set", false, true)
                    || string_utils::cmp(&(x.0).1, "ImmSet", false, true)) =>
        {
            set_expr_to_typed_value(emitter, x)
        }
        Expr_::Tuple(x) => tuple_expr_to_typed_value(emitter, x),
        Expr_::ValCollection(x) if x.0 == ast::VcKind::Vec || x.0 == ast::VcKind::Vector => {
            valcollection_vec_expr_to_typed_value(emitter, x)
        }
        Expr_::ValCollection(x) if x.0 == ast::VcKind::Keyset => {
            valcollection_keyset_expr_to_typed_value(emitter, x)
        }
        Expr_::ValCollection(x) if x.0 == ast::VcKind::Set || x.0 == ast::VcKind::ImmSet => {
            valcollection_set_expr_to_typed_value(emitter, x)
        }
        Expr_::KeyValCollection(x) => keyvalcollection_expr_to_typed_value(emitter, x),
        Expr_::Shape(fields) => shape_to_typed_value(emitter, fields),
        Expr_::ClassConst(x) => class_const_expr_to_typed_value(emitter, x, force_class_const),

        Expr_::ClassGet(_) => Err(Error::UserDefinedConstant),
        ast::Expr_::As(x) if (x.1).1.is_hlike() => {
            expr_to_typed_value_(emitter, &x.0, allow_maps, false)
        }
        _ => Err(Error::NotLiteral),
    }
}

fn class_const_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    x: &(ast::ClassId, ast::Pstring),
    force_class_const: bool,
) -> Result<TypedValue<'arena>, Error> {
    if emitter.options().emit_class_pointers() == 1 && !force_class_const {
        Err(Error::NotLiteral)
    } else {
        class_const_to_typed_value(emitter, &x.0, &x.1)
    }
}

fn call_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    id: &(
        ast::Expr,
        Vec<ast::Targ>,
        Vec<(ast_defs::ParamKind, ast::Expr)>,
        Option<ast::Expr>,
    ),
) -> Result<TypedValue<'arena>, Error> {
    use ast::{Expr, Expr_};
    match id.2[..] {
        [(ast_defs::ParamKind::Pnormal, Expr(_, _, Expr_::String(ref data)))] => {
            // FIXME: This is not safe--string literals are binary strings.
            // There's no guarantee that they're valid UTF-8.
            Ok(TypedValue::mk_hhas_adata(
                emitter
                    .alloc
                    .alloc_str(unsafe { std::str::from_utf8_unchecked(data) }),
            ))
        }
        _ => Err(Error::NotLiteral),
    }
}

fn valcollection_keyset_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    x: &(ast::VcKind, Option<ast::Targ>, Vec<ast::Expr>),
) -> Result<TypedValue<'arena>, Error> {
    let keys = emitter.alloc.alloc_slice_fill_iter(
        x.2.iter()
            .map(|e| {
                expr_to_typed_value(emitter, e).and_then(|tv| match tv {
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
            .collect::<Vec<_>>()
            .into_iter(),
    );
    Ok(TypedValue::mk_keyset(keys))
}

fn keyvalcollection_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    x: &(
        ast::KvcKind,
        Option<(ast::Targ, ast::Targ)>,
        Vec<ast::Field>,
    ),
) -> Result<TypedValue<'arena>, Error> {
    let values = emitter
        .alloc
        .alloc_slice_fill_iter(update_duplicates_in_map(
            x.2.iter()
                .map(|e| kv_to_typed_value_pair(emitter, &e.0, &e.1))
                .collect::<Result<Vec<_>, _>>()?,
        ));
    Ok(TypedValue::mk_dict(values))
}

fn valcollection_set_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    x: &(ast::VcKind, Option<ast::Targ>, Vec<ast::Expr>),
) -> Result<TypedValue<'arena>, Error> {
    let values = emitter
        .alloc
        .alloc_slice_fill_iter(update_duplicates_in_map(
            x.2.iter()
                .map(|e| set_afield_value_to_typed_value_pair(emitter, e))
                .collect::<Result<Vec<_>, _>>()?,
        ));
    Ok(TypedValue::mk_dict(values))
}

fn valcollection_vec_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    x: &(ast::VcKind, Option<ast::Targ>, Vec<ast::Expr>),
) -> Result<TypedValue<'arena>, Error> {
    let v: Vec<_> =
        x.2.iter()
            .map(|e| expr_to_typed_value(emitter, e))
            .collect::<Result<_, _>>()?;
    Ok(TypedValue::mk_vec(
        emitter.alloc.alloc_slice_fill_iter(v.into_iter()),
    ))
}

fn tuple_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    x: &[ast::Expr],
) -> Result<TypedValue<'arena>, Error> {
    let v: Vec<_> = x
        .iter()
        .map(|e| expr_to_typed_value(emitter, e))
        .collect::<Result<_, _>>()?;
    Ok(TypedValue::mk_vec(
        emitter.alloc.alloc_slice_fill_iter(v.into_iter()),
    ))
}

fn set_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
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
                .map(|x| set_afield_to_typed_value_pair(emitter, x))
                .collect::<Result<_, _>>()?,
        ));
    Ok(TypedValue::mk_dict(values))
}

fn dict_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
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
                .map(|x| afield_to_typed_value_pair(emitter, x))
                .collect::<Result<_, _>>()?,
        ));
    Ok(TypedValue::mk_dict(values))
}

fn keyset_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    x: &(
        ast::ClassName,
        Option<ast::CollectionTarg>,
        Vec<ast::Afield>,
    ),
) -> Result<TypedValue<'arena>, Error> {
    let keys = emitter.alloc.alloc_slice_fill_iter(
        x.2.iter()
            .map(|x| keyset_value_afield_to_typed_value(emitter, x))
            .collect::<Result<Vec<_>, _>>()?
            .into_iter()
            .unique()
            .collect::<Vec<_>>()
            .into_iter(),
    );
    Ok(TypedValue::mk_keyset(keys))
}

fn float_expr_to_typed_value<'arena, 'decl>(
    _emitter: &Emitter<'arena, 'decl>,
    s: &str,
) -> Result<TypedValue<'arena>, Error> {
    if s == math::INF {
        Ok(TypedValue::double(std::f64::INFINITY))
    } else if s == math::NEG_INF {
        Ok(TypedValue::double(std::f64::NEG_INFINITY))
    } else if s == math::NAN {
        Ok(TypedValue::double(std::f64::NAN))
    } else {
        s.parse()
            .map(TypedValue::double)
            .map_err(|_| Error::NotLiteral)
    }
}

fn string_expr_to_typed_value<'arena, 'decl>(
    emitter: &Emitter<'arena, 'decl>,
    s: &[u8],
) -> Result<TypedValue<'arena>, Error> {
    // FIXME: This is not safe--string literals are binary strings.
    // There's no guarantee that they're valid UTF-8.
    Ok(TypedValue::mk_string(
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
    Item = Pair<TypedValue<'arena>, TypedValue<'arena>>,
    IntoIter = impl ExactSizeIterator + 'arena,
> + 'arena {
    kvs.into_iter()
        .collect::<IndexMap<_, _, RandomState>>()
        .into_iter()
        .map(std::convert::Into::into)
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
                v.cast_to_bool()
            } else if id == typehints::STRING {
                v.cast_to_string(alloc)
            } else if id == typehints::FLOAT {
                v.cast_to_double()
            } else {
                None
            }
        }
        _ => None,
    }
    .ok_or(Error::NotLiteral)
}

fn unop_on_value<'arena>(
    alloc: &'arena bumpalo::Bump,
    unop: &ast_defs::Uop,
    v: TypedValue<'arena>,
) -> Result<TypedValue<'arena>, Error> {
    match unop {
        ast_defs::Uop::Unot => v.not(),
        ast_defs::Uop::Uplus => v.add(&TypedValue::Int(0)),
        ast_defs::Uop::Uminus => v.neg(),
        ast_defs::Uop::Utild => v.bitwise_not(alloc),
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
        Bop::Dot => v1.concat(alloc, v2),
        Bop::Plus => v1.add(&v2),
        Bop::Minus => v1.sub(&v2),
        Bop::Star => v1.mul(&v2),
        Bop::Ltlt => v1.shift_left(&v2),
        Bop::Slash => v1.div(&v2),
        Bop::Bar => v1.bitwise_or(&v2),
        _ => None,
    }
    .ok_or(Error::NotLiteral)
}

fn value_to_expr_<'arena>(v: TypedValue<'arena>) -> Result<ast::Expr_, Error> {
    use ast::Expr_;
    match v {
        TypedValue::Int(i) => Ok(Expr_::Int(i.to_string())),
        TypedValue::Double(f) => Ok(Expr_::Float(hhbc_string_utils::float::to_string(
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
        TypedValue::HhasAdata(_) => Err(Error::unrecoverable("value_to_expr: HhasAdata NYI")),
        TypedValue::Dict(_) => Err(Error::unrecoverable("value_to_expr: dict NYI")),
    }
}

struct FolderVisitor<'a, 'arena, 'decl> {
    emitter: &'a Emitter<'arena, 'decl>,
}

impl<'a, 'arena, 'decl> FolderVisitor<'a, 'arena, 'decl> {
    fn new(emitter: &'a Emitter<'arena, 'decl>) -> Self {
        Self { emitter }
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
            ast::Expr_::Cast(e) => expr_to_typed_value(self.emitter, &e.1)
                .and_then(|v| cast_value(self.emitter.alloc, &(e.0).1, v))
                .map(value_to_expr_)
                .ok(),
            ast::Expr_::Unop(e) => expr_to_typed_value(self.emitter, &e.1)
                .and_then(|v| unop_on_value(self.emitter.alloc, &e.0, v))
                .map(value_to_expr_)
                .ok(),
            ast::Expr_::Binop(e) => expr_to_typed_value(self.emitter, &e.1)
                .and_then(|v1| {
                    expr_to_typed_value(self.emitter, &e.2).and_then(|v2| {
                        binop_on_values(self.emitter.alloc, &e.0, v1, v2).map(value_to_expr_)
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

pub fn fold_expr<'arena, 'decl>(
    expr: &mut ast::Expr,
    e: &mut Emitter<'arena, 'decl>,
) -> Result<(), Error> {
    visit_mut(&mut FolderVisitor::new(e), &mut (), expr)
}

pub fn fold_program<'arena, 'decl>(
    p: &mut ast::Program,
    e: &mut Emitter<'arena, 'decl>,
) -> Result<(), Error> {
    visit_mut(&mut FolderVisitor::new(e), &mut (), p)
}

pub fn literals_from_exprs<'arena, 'decl>(
    exprs: &mut [ast::Expr],
    e: &mut Emitter<'arena, 'decl>,
) -> Result<Vec<TypedValue<'arena>>, Error> {
    for expr in exprs.iter_mut() {
        fold_expr(expr, e)?;
    }
    let ret = exprs
        .iter()
        .map(|expr| expr_to_typed_value_(e, expr, false, true))
        .collect();
    if let Err(Error::NotLiteral) = ret {
        Err(Error::unrecoverable("literals_from_exprs: not literal"))
    } else {
        ret
    }
}
