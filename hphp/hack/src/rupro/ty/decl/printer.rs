// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::ty::*;
use crate::reason::Reason;
use std::collections::BTreeMap;
use std::fmt::{self, Display, Formatter};

// This module provides a `Display` impl for `DeclTy` which uses Hack-like
// syntax. The `Debug` impl for `DeclTy` uses this `Display` impl to make debug
// output easier to read. If you need the full `Debug` information for a
// `DeclTy`, use the `Debug` impl on `DeclTy_`, which is the standard derived
// impl.

impl<R: Reason> Display for DeclTy<R> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.node())
    }
}

impl<R: Reason> fmt::Debug for DeclTy<R> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.node())
    }
}

impl<R: Reason> Display for DeclTy_<R> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        use DeclTy_::*;
        match self {
            DTany => write!(f, "_"),
            DTerr => write!(f, "err"),
            DTthis => write!(f, "this"),
            DTmixed => write!(f, "mixed"),
            DTnonnull => write!(f, "nonnull"),
            DTdynamic => write!(f, "dynamic"),
            DTprim(prim) => match prim {
                Prim::Tnull => write!(f, "null"),
                Prim::Tvoid => write!(f, "void"),
                Prim::Tint => write!(f, "int"),
                Prim::Tbool => write!(f, "bool"),
                Prim::Tfloat => write!(f, "float"),
                Prim::Tstring => write!(f, "string"),
                Prim::Tresource => write!(f, "resource"),
                Prim::Tnum => write!(f, "num"),
                Prim::Tarraykey => write!(f, "arraykey"),
                Prim::Tnoreturn => write!(f, "noreturn"),
            },
            DTvar(id) => write!(f, "{}", id),
            DTlike(ty) => write!(f, "~{}", ty),
            DToption(ty) => write!(f, "?{}", ty),
            DTaccess(ta) => write!(f, "{}::{}", ta.ty, ta.type_const.id()),
            DTfun(ft) => write!(f, "{}", ft),
            DTshape(params) => tshape(f, params.0, &params.1),
            DTtuple(tys) => list(f, "(", ", ", ")", tys.iter()),
            DTunion(tys) => {
                if tys.len() <= 1 {
                    write!(f, "|")?;
                }
                list(f, "(", " | ", ")", tys.iter())
            }
            DTintersection(tys) => {
                if tys.len() <= 1 {
                    write!(f, "&")?;
                }
                list(f, "(", " & ", ")", tys.iter())
            }
            DTapply(params) => {
                let (name, args) = &**params;
                write!(f, "{}", strip_ns(name.id_ref()))?;
                if !args.is_empty() {
                    list(f, "<", ", ", ">", args.iter())?;
                }
                Ok(())
            }
            DTgeneric(params) => {
                let (name, ref args) = **params;
                write!(f, "{}", name)?;
                if !args.is_empty() {
                    list(f, "<", ", ", ">", args.iter())?;
                }
                Ok(())
            }
            DTvecOrDict(params) => {
                let (kty, vty) = &**params;
                write!(f, "vec_or_dict<{}, {}>", kty, vty)
            }
        }
    }
}

impl<R: Reason, TY: Display> Display for FunType<R, TY> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "(")?;
        if self.flags.contains(FunTypeFlags::READONLY_THIS) {
            write!(f, "readonly ")?;
        }
        write!(f, "function")?;
        if !self.tparams.is_empty() {
            list(f, "<", ", ", ">", self.tparams.iter())?;
        }
        list(f, "(", ", ", "): ", self.params.iter())?;
        if self.flags.contains(FunTypeFlags::RETURNS_READONLY) {
            write!(f, "readonly ")?;
        }
        write!(f, "{})", self.ret)
    }
}

impl<R: Reason, TY: Display> Display for Tparam<R, TY> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        match self.reified {
            ReifyKind::Erased => {}
            ReifyKind::SoftReified => write!(f, "<<__Soft>> reify ")?,
            ReifyKind::Reified => write!(f, "reify ")?,
        }
        write!(f, "{}", self.name.id())?;
        let mut is_first = true;
        for constraint in self.constraints.iter() {
            if !is_first {
                write!(f, " ")?;
            }
            is_first = false;
            let (kind, ref ty) = *constraint;
            match kind {
                ConstraintKind::ConstraintAs => write!(f, " as {}", ty)?,
                ConstraintKind::ConstraintSuper => write!(f, " super {}", ty)?,
                ConstraintKind::ConstraintEq => write!(f, " = {}", ty)?,
            }
        }
        Ok(())
    }
}

impl<R: Reason, TY: Display> Display for FunParam<R, TY> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        if self.flags.contains(FunParamFlags::INOUT) {
            write!(f, "inout ")?;
        }
        if self.flags.contains(FunParamFlags::READONLY) {
            write!(f, "readonly ")?;
        }
        match self.name {
            None => write!(f, "{}", self.ty)?,
            Some(name) => {
                let ty_str = format!("{}", self.ty);
                match ty_str.as_str() {
                    "_" | "err" => write!(f, "{}", name)?,
                    _ => write!(f, "{} {}", self.ty, name)?,
                }
            }
        }
        if self.flags.contains(FunParamFlags::HAS_DEFAULT) {
            write!(f, "=_")?;
        }
        Ok(())
    }
}

impl<TY: Display> Display for PossiblyEnforcedTy<TY> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        if self.enforced == Enforcement::Enforced {
            write!(f, "enforced ")?;
        }
        write!(f, "{}", self.ty)
    }
}

fn tshape<R: Reason>(
    f: &mut Formatter<'_>,
    kind: ShapeKind,
    fields: &BTreeMap<TshapeFieldName, ShapeFieldType<R>>,
) -> fmt::Result {
    write!(f, "shape(")?;
    let mut is_first = true;
    for (name, sft) in fields {
        if !is_first {
            write!(f, ", ")?;
        }
        is_first = false;
        if sft.optional {
            write!(f, "?")?
        }
        match name {
            TshapeFieldName::TSFlitInt(i) => write!(f, "{}", i)?,
            TshapeFieldName::TSFlitStr(s) => write!(f, "'{}'", String::from_utf8_lossy(s))?,
            TshapeFieldName::TSFclassConst(class_name, const_name) => {
                write!(f, "{}::{}", strip_ns(class_name), const_name)?;
            }
        }
        write!(f, " => {}", sft.ty)?;
    }
    match kind {
        ShapeKind::ClosedShape => {}
        ShapeKind::OpenShape => {
            if !is_first {
                write!(f, ", ")?;
            }
            write!(f, "...")?;
        }
    }
    write!(f, ")")
}

fn strip_ns<'a>(s: &'a impl AsRef<str>) -> &'a str {
    let s = s.as_ref();
    s.strip_prefix('\\').unwrap_or(s)
}

fn list<T: Display>(
    f: &mut Formatter<'_>,
    left_delim: &str,
    separator: &str,
    right_delim: &str,
    list: impl Iterator<Item = T>,
) -> fmt::Result {
    write!(f, "{}", left_delim)?;
    let mut is_first = true;
    for item in list {
        if !is_first {
            write!(f, "{}", separator)?;
        }
        is_first = false;
        write!(f, "{}", item)?;
    }
    write!(f, "{}", right_delim)
}
