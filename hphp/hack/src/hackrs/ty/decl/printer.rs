// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::fmt;
use std::fmt::Display;
use std::fmt::Formatter;

use super::ty::*;
use crate::reason::Reason;

// This module provides a `Display` impl for `Ty` which uses Hack-like
// syntax. The `Debug` impl for `Ty` uses this `Display` impl to make debug
// output easier to read. If you need the full `Debug` information for a
// `Ty`, use the `Debug` impl on `Ty_`, which is the standard derived
// impl.

impl<R: Reason> Display for Ty<R> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.node())
    }
}

impl<R: Reason> fmt::Debug for Ty<R> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.node())
    }
}

impl<R: Reason> Display for Ty_<R> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        use Ty_::*;
        match self {
            Tany => write!(f, "_"),
            Tthis => write!(f, "this"),
            Tmixed => write!(f, "mixed"),
            Twildcard => write!(f, "_"),
            Tnonnull => write!(f, "nonnull"),
            Tdynamic => write!(f, "dynamic"),
            Tprim(prim) => match prim {
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
            Tlike(ty) => write!(f, "~{}", ty),
            Toption(ty) => write!(f, "?{}", ty),
            Taccess(ta) => write!(f, "{}::{}", ta.ty, ta.type_const.id()),
            Tfun(ft) => write!(f, "{}", ft),
            Tshape(params) => tshape(f, &params.0, &params.1),
            Ttuple(tys) => list(f, "(", ", ", ")", tys.iter()),
            Trefinement(r) => {
                write!(f, "({} with ", r.ty)?;
                trefinements(f, &r.refinement.consts)?;
                write!(f, ")")
            }
            Tunion(tys) => match tys.len() {
                0 => write!(f, "nothing"),
                1 => write!(f, "(|{})", tys[0]),
                _ => list(f, "(", " | ", ")", tys.iter()),
            },
            Tintersection(tys) => match tys.len() {
                0 => write!(f, "mixed"),
                1 => write!(f, "(&{})", tys[0]),
                _ => list(f, "(", " & ", ")", tys.iter()),
            },
            Tapply(params) => {
                let (name, args) = &**params;
                write!(f, "{}", strip_ns(name.id_ref()))?;
                if !args.is_empty() {
                    list(f, "<", ", ", ">", args.iter())?;
                }
                Ok(())
            }
            Tgeneric(params) => {
                let (name, ref args) = **params;
                write!(f, "{}", name)?;
                if !args.is_empty() {
                    list(f, "<", ", ", ">", args.iter())?;
                }
                Ok(())
            }
            TvecOrDict(params) => {
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

fn trefinements<TY: Display>(
    f: &mut Formatter<'_>,
    consts: &BTreeMap<pos::TypeConstName, RefinedConst<TY>>,
) -> fmt::Result {
    write!(f, "{{")?;
    let mut is_first = true;
    for (name, rc) in consts {
        if !is_first {
            write!(f, "; ")?;
        }
        is_first = false;
        let kind = if rc.is_ctx { "ctx" } else { "type" };
        write!(f, "{} {}", kind, name)?;
        match &rc.bound {
            RefinedConstBound::Exact(ref ty) => write!(f, " = {}", ty)?,
            RefinedConstBound::Loose(bounds) => {
                for ty in bounds.lower.iter() {
                    write!(f, " super {}", ty)?;
                }
                for ty in bounds.upper.iter() {
                    write!(f, " as {}", ty)?;
                }
            }
        }
    }
    write!(f, "}}")
}

fn tshape<R: Reason>(
    f: &mut Formatter<'_>,
    kind: &Ty<R>,
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
    match &**kind.node() {
        // Closed shape is represented by unknown fields having type nothing
        Ty_::Tunion(tys) if tys.is_empty() => {}
        _ => {
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
