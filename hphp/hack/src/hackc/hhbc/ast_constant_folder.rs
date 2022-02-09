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
    aast,
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
    use aast::Expr_::*;
    // TODO: ML equivalent has this as an implicit parameter that defaults to false.
    match &expr.2 {
        Int(s) => int_expr_to_typed_value(s),
        ast::Expr_::True => Ok(TypedValue::Bool(true)),
        False => Ok(TypedValue::Bool(false)),
        Null => Ok(TypedValue::Null),
        String(s) => {
            // FIXME: This is not safe--string literals are binary strings.
            // There's no guarantee that they're valid UTF-8.
            Ok(TypedValue::mk_string(
                emitter
                    .alloc
                    .alloc_str(unsafe { std::str::from_utf8_unchecked(s) }),
            ))
        }
        Float(s) => {
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
        Call(id)
            if id
                .0
                .as_id()
                .map_or(false, |x| x.1 == special_functions::HHAS_ADATA) =>
        {
            match id.2[..] {
                [(ast_defs::ParamKind::Pnormal, ast::Expr(_, _, ast::Expr_::String(ref data)))] => {
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

        Varray(fields) => varray_to_typed_value(emitter, &fields.1),
        Darray(fields) => darray_to_typed_value(emitter, &fields.1),

        Id(id) if id.1 == math::NAN => Ok(TypedValue::float(std::f64::NAN)),
        Id(id) if id.1 == math::INF => Ok(TypedValue::float(std::f64::INFINITY)),
        Id(_) => Err(Error::UserDefinedConstant),

        Collection(x) if x.0.name().eq("vec") => vec_to_typed_value(emitter, &x.2),
        Collection(x) if x.0.name().eq("keyset") => {
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
        Collection(x)
            if x.0.name().eq("dict")
                || allow_maps
                    && (string_utils::cmp(&(x.0).1, "Map", false, true)
                        || string_utils::cmp(&(x.0).1, "ImmMap", false, true)) =>
        {
            let values = emitter
                .alloc
                .alloc_slice_fill_iter(update_duplicates_in_map(
                    x.2.iter()
                        .map(|x| afield_to_typed_value_pair(emitter, x))
                        .collect::<Result<_, _>>()?,
                ));
            Ok(TypedValue::mk_dict(values))
        }
        Collection(x)
            if allow_maps
                && (string_utils::cmp(&(x.0).1, "Set", false, true)
                    || string_utils::cmp(&(x.0).1, "ImmSet", false, true)) =>
        {
            let values = emitter
                .alloc
                .alloc_slice_fill_iter(update_duplicates_in_map(
                    x.2.iter()
                        .map(|x| set_afield_to_typed_value_pair(emitter, x))
                        .collect::<Result<_, _>>()?,
                ));
            Ok(TypedValue::mk_dict(values))
        }
        Tuple(x) => {
            let v: Vec<_> = x
                .iter()
                .map(|e| expr_to_typed_value(emitter, e))
                .collect::<Result<_, _>>()?;
            Ok(TypedValue::mk_vec(
                emitter.alloc.alloc_slice_fill_iter(v.into_iter()),
            ))
        }
        ValCollection(x) if x.0 == ast::VcKind::Vec || x.0 == ast::VcKind::Vector => {
            let v: Vec<_> =
                x.2.iter()
                    .map(|e| expr_to_typed_value(emitter, e))
                    .collect::<Result<_, _>>()?;
            Ok(TypedValue::mk_vec(
                emitter.alloc.alloc_slice_fill_iter(v.into_iter()),
            ))
        }
        ValCollection(x) if x.0 == ast::VcKind::Keyset => {
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
        ValCollection(x) if x.0 == ast::VcKind::Set || x.0 == ast::VcKind::ImmSet => {
            let values = emitter
                .alloc
                .alloc_slice_fill_iter(update_duplicates_in_map(
                    x.2.iter()
                        .map(|e| set_afield_value_to_typed_value_pair(emitter, e))
                        .collect::<Result<Vec<_>, _>>()?,
                ));
            Ok(TypedValue::mk_dict(values))
        }
        KeyValCollection(x) => {
            let values = emitter
                .alloc
                .alloc_slice_fill_iter(update_duplicates_in_map(
                    x.2.iter()
                        .map(|e| kv_to_typed_value_pair(emitter, &e.0, &e.1))
                        .collect::<Result<Vec<_>, _>>()?,
                ));
            Ok(TypedValue::mk_dict(values))
        }
        Shape(fields) => shape_to_typed_value(emitter, fields),
        ClassConst(x) => {
            if emitter.options().emit_class_pointers() == 1 && !force_class_const {
                Err(Error::NotLiteral)
            } else {
                class_const_to_typed_value(emitter, &x.0, &x.1)
            }
        }
        ClassGet(_) => Err(Error::UserDefinedConstant),
        As(x) if (x.1).1.is_hlike() => expr_to_typed_value_(emitter, &x.0, allow_maps, false),
        _ => Err(Error::NotLiteral),
    }
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
                v.cast_to_float()
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
    use ast_defs::Bop::*;
    match binop {
        Dot => v1.concat(alloc, v2),
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

fn value_to_expr_<'arena>(v: TypedValue<'arena>) -> Result<ast::Expr_, Error> {
    use ast::*;
    use TypedValue::*;
    Ok(match v {
        Int(i) => Expr_::Int(i.to_string()),
        Float(i) => Expr_::Float(hhbc_string_utils::float::to_string(i)),
        Bool(false) => Expr_::False,
        Bool(true) => Expr_::True,
        String(s) => Expr_::String(s.unsafe_as_str().into()),
        LazyClass(_) => return Err(Error::unrecoverable("value_to_expr: lazyclass NYI")),
        Null => Expr_::Null,
        Uninit => return Err(Error::unrecoverable("value_to_expr: uninit value")),
        Vec(_) => return Err(Error::unrecoverable("value_to_expr: vec NYI")),
        Keyset(_) => return Err(Error::unrecoverable("value_to_expr: keyset NYI")),
        HhasAdata(_) => return Err(Error::unrecoverable("value_to_expr: HhasAdata NYI")),
        Dict(_) => return Err(Error::unrecoverable("value_to_expr: dict NYI")),
    })
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
