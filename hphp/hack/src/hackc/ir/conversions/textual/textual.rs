// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! This is a structural version of the Textual language - it should have
//! basically no business logic and just provides a type-safe way to write
//! Textual.

use std::borrow::Cow;
use std::fmt;

use anyhow::Error;
use ascii::AsciiString;
use ir::func::SrcLoc;
use ir::BlockId;
use ir::LocalId;
use ir::StringInterner;
use itertools::Itertools;
use newtype::newtype_int;

pub(crate) const INDENT: &str = "  ";

type Result<T = (), E = Error> = std::result::Result<T, E>;

newtype_int!(Sid, u32, SidMap, SidSet);

struct FmtBid(BlockId);

impl fmt::Display for FmtBid {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "b{}", self.0.as_usize())
    }
}

struct FmtLid<'a>(&'a StringInterner, LocalId);

impl fmt::Display for FmtLid<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let FmtLid(strings, lid) = *self;
        match lid {
            LocalId::Named(id) => {
                write!(f, "{}", strings.lookup_bstr(id))
            }
            LocalId::Unnamed(id) => {
                write!(f, "${}", id.as_usize())
            }
        }
    }
}

pub(crate) struct FmtSid(pub Sid);

impl fmt::Display for FmtSid {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let FmtSid(sid) = *self;
        write!(f, "n{}", sid.as_usize())
    }
}

pub(crate) type Ty = ir::BaseType;

struct FmtTy<'a>(&'a Ty);

impl fmt::Display for FmtTy<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.0 {
            Ty::Bool => write!(f, "bool"),
            Ty::Int => write!(f, "int"),
            Ty::String => write!(f, "string"),
            Ty::RawType(s) => write!(f, "{s}"),
            Ty::RawPtr(sub) => write!(f, "*{}", FmtTy(sub)),
            Ty::Mixed => f.write_str("*Mixed"),
            Ty::Void => f.write_str("void"),

            Ty::AnyArray
            | Ty::Arraykey
            | Ty::Class(_)
            | Ty::Classname
            | Ty::Darray
            | Ty::Dict
            | Ty::Float
            | Ty::Keyset
            | Ty::None
            | Ty::Nonnull
            | Ty::Noreturn
            | Ty::Nothing
            | Ty::Null
            | Ty::Num
            | Ty::Resource
            | Ty::This
            | Ty::Typename
            | Ty::Varray
            | Ty::VarrayOrDarray
            | Ty::Vec
            | Ty::VecOrDict => todo!("unhandled type: {:?}", self.0),
        }
    }
}

#[derive(Clone, Debug)]
pub(crate) enum Var {
    Named(String),
    Hack(LocalId),
}

impl Var {
    pub(crate) fn named(s: impl Into<String>) -> Self {
        Var::Named(s.into())
    }
}

impl From<LocalId> for Var {
    fn from(lid: LocalId) -> Var {
        Var::Hack(lid)
    }
}

struct FmtVar<'a>(&'a StringInterner, &'a Var);

impl fmt::Display for FmtVar<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let FmtVar(strings, var) = *self;
        match *var {
            Var::Named(ref s) => s.fmt(f),
            Var::Hack(lid) => FmtLid(strings, lid).fmt(f),
        }
    }
}

#[derive(Clone, Debug)]
pub(crate) enum Const {
    False,
    // Float(f64),
    HackInt(i64),
    HackString(Vec<u8>),
    Int(i64),
    Null,
    String(AsciiString),
    True,
}

struct FmtConst<'a>(&'a Const);

impl fmt::Display for FmtConst<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let FmtConst(const_) = *self;
        match const_ {
            Const::False => f.write_str("false"),
            // Const::Float(v) => v.fmt(f),
            Const::HackInt(i) => write!(f, "hack_int({})", i),
            Const::HackString(s) => {
                write!(f, "hack_string(\"{}\")", crate::util::escaped_string(s))
            }
            Const::Int(i) => i.fmt(f),
            Const::Null => f.write_str("null"),
            Const::String(ref s) => {
                // String should already be escaped...
                write!(f, "\"{}\"", s)
            }
            Const::True => f.write_str("true"),
        }
    }
}

#[derive(Clone, Debug)]
pub(crate) enum Expr {
    Sid(Sid),
    /// *Variable
    Deref(Var),
    // Field(String),
    /// a[b]
    Index(Box<Expr>, Box<Expr>),
    /// 0, null, etc
    Const(Const),
    /// foo(1, 2, 3)
    Call(String, Box<[Expr]>),
    // TyAscription(Box<Expr>, Ty),
}

impl Expr {
    pub(crate) fn call(target: impl ToString, params: impl VarArgs) -> Expr {
        Expr::Call(target.to_string(), params.into_exprs().into_boxed_slice())
    }

    pub(crate) fn deref(v: impl Into<Var>) -> Expr {
        Expr::Deref(v.into())
    }

    pub(crate) fn false_() -> Expr {
        Expr::Const(Const::False)
    }

    pub(crate) fn hack_int(i: i64) -> Expr {
        Expr::Const(Const::HackInt(i))
    }

    pub(crate) fn hack_string<'a>(s: impl Into<Cow<'a, [u8]>>) -> Expr {
        Expr::Const(Const::HackString(s.into().into_owned()))
    }

    pub(crate) fn index(base: impl Into<Expr>, offset: impl Into<Expr>) -> Expr {
        let base = base.into();
        let offset = offset.into();
        Expr::Index(Box::new(base), Box::new(offset))
    }

    pub(crate) fn int(i: i64) -> Expr {
        Expr::Const(Const::Int(i))
    }

    pub(crate) fn null() -> Expr {
        Expr::Const(Const::Null)
    }

    pub(crate) fn string(s: AsciiString) -> Expr {
        Expr::Const(Const::String(s))
    }

    pub(crate) fn true_() -> Expr {
        Expr::Const(Const::True)
    }
}

impl From<&'static str> for Expr {
    fn from(s: &'static str) -> Self {
        Expr::Const(Const::String(AsciiString::from_ascii(s).unwrap()))
    }
}

impl From<i64> for Expr {
    fn from(i: i64) -> Self {
        Expr::Const(Const::Int(i))
    }
}

impl From<Sid> for Expr {
    fn from(sid: Sid) -> Self {
        Expr::Sid(sid)
    }
}

struct FmtExpr<'a>(&'a StringInterner, &'a Expr);

impl fmt::Display for FmtExpr<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let FmtExpr(strings, expr) = *self;
        match *expr {
            Expr::Call(ref target, ref params) => {
                write!(f, "{}(", target)?;
                let mut sep = "";
                for param in params.iter() {
                    write!(f, "{sep}{}", FmtExpr(strings, param))?;
                    sep = ", ";
                }
                write!(f, ")")
            }
            Expr::Const(ref c) => FmtConst(c).fmt(f),
            Expr::Deref(ref var) => write!(f, "&{}", FmtVar(strings, var)),
            Expr::Index(ref base, ref offset) => {
                write!(
                    f,
                    "{}[{}]",
                    FmtExpr(strings, base),
                    FmtExpr(strings, offset)
                )
            }
            Expr::Sid(sid) => FmtSid(sid).fmt(f),
        }
    }
}

pub(crate) trait VarArgs {
    fn into_exprs(self) -> Vec<Expr>;
}

impl<T> VarArgs for &[T]
where
    T: Into<Expr> + Copy,
{
    fn into_exprs(self) -> Vec<Expr> {
        self.iter().copied().map_into().collect_vec()
    }
}

impl VarArgs for () {
    fn into_exprs(self) -> Vec<Expr> {
        Vec::new()
    }
}

impl<T> VarArgs for [T; 1]
where
    T: Into<Expr>,
{
    fn into_exprs(self) -> Vec<Expr> {
        let [a] = self;
        vec![a.into()]
    }
}

impl<T> VarArgs for [T; 2]
where
    T: Into<Expr>,
{
    fn into_exprs(self) -> Vec<Expr> {
        let [a, b] = self;
        vec![a.into(), b.into()]
    }
}

impl<T> VarArgs for Vec<T>
where
    T: Into<Expr>,
{
    fn into_exprs(self) -> Vec<Expr> {
        self.into_iter().map_into().collect_vec()
    }
}

impl<P1, P2> VarArgs for (P1, P2)
where
    P1: Into<Expr>,
    P2: Into<Expr>,
{
    fn into_exprs(self) -> Vec<Expr> {
        vec![self.0.into(), self.1.into()]
    }
}

impl<P1, P2, P3> VarArgs for (P1, P2, P3)
where
    P1: Into<Expr>,
    P2: Into<Expr>,
    P3: Into<Expr>,
{
    fn into_exprs(self) -> Vec<Expr> {
        vec![self.0.into(), self.1.into(), self.2.into()]
    }
}

pub(crate) fn write_full_loc(
    w: &mut dyn std::io::Write,
    src_loc: &SrcLoc,
    strings: &StringInterner,
) -> Result {
    let filename = strings.lookup_bstr(src_loc.filename.0);
    writeln!(w, "// .file \"{filename}\"")?;
    write_line_loc(w, src_loc)?;
    Ok(())
}

pub(crate) fn write_line_loc(w: &mut dyn std::io::Write, src_loc: &SrcLoc) -> Result {
    writeln!(w, "// .line {}", src_loc.line_begin)?;
    Ok(())
}

pub(crate) enum Attribute {
    SourceLanguage(String),
}

pub(crate) fn write_attribute(w: &mut dyn std::io::Write, attr: Attribute) -> Result {
    match attr {
        Attribute::SourceLanguage(lang) => {
            writeln!(w, "attribute source_language = \"{lang}\"")?;
        }
    }
    Ok(())
}

pub(crate) fn write_function(
    w: &mut dyn std::io::Write,
    strings: &StringInterner,
    name: &str,
    loc: &SrcLoc,
    params: &[(&str, Ty)],
    ret_ty: Ty,
    body: impl FnOnce(&mut FuncWriter<'_>) -> Result,
) -> Result {
    write_full_loc(w, loc, strings)?;

    write!(w, "define {name}(")?;
    let mut sep = "";
    for (name, ty) in params {
        write!(w, "{sep}{name}: {ty}", ty = FmtTy(ty))?;
        sep = ", ";
    }
    writeln!(w, ") : {} {{", FmtTy(&ret_ty))?;

    let mut writer = FuncWriter {
        cur_loc: loc.clone(),
        next_id: Sid::from_usize(0),
        strings,
        w,
    };

    writer.write_label(BlockId::from_usize(0), &[])?;
    body(&mut writer)?;

    writeln!(w, "}}")?;
    writeln!(w)?;
    Ok(())
}

pub(crate) struct FuncWriter<'a> {
    cur_loc: SrcLoc,
    next_id: Sid,
    strings: &'a StringInterner,
    w: &'a mut dyn std::io::Write,
}

impl<'a> FuncWriter<'a> {
    pub fn alloc_sid(&mut self) -> Sid {
        let id = self.next_id;
        self.next_id = Sid::from_usize(id.as_usize() + 1);
        id
    }
}

impl<'a> FuncWriter<'a> {
    pub(crate) fn jmp(&mut self, targets: &[BlockId], params: impl VarArgs) -> Result {
        let params = params.into_exprs();
        write!(self.w, "{INDENT}jmp ")?;

        let mut sep1 = "";
        for &target in targets {
            write!(self.w, "{sep1}{}", FmtBid(target))?;
            sep1 = ", ";

            if !params.is_empty() {
                write!(self.w, "(")?;
                let mut sep2 = "";
                for param in params.iter() {
                    write!(self.w, "{sep2}{}", FmtExpr(self.strings, param))?;
                    sep2 = ", ";
                }
                write!(self.w, ")")?;
            }
        }

        writeln!(self.w)?;

        Ok(())
    }

    pub(crate) fn write_expr(&mut self, expr: impl Into<Expr>) -> Result<Sid> {
        let expr = expr.into();
        match expr {
            Expr::Sid(sid) => Ok(sid),
            Expr::Call(_, _) | Expr::Const(_) | Expr::Deref(_) | Expr::Index(_, _) => {
                todo!("EXPR: {expr:?}")
            }
        }
    }

    pub(crate) fn write_label(&mut self, bid: BlockId, params: &[Sid]) -> Result {
        if params.is_empty() {
            writeln!(self.w, "#{}:", FmtBid(bid))?;
        } else {
            write!(self.w, "#{}(", FmtBid(bid))?;
            let mut sep = "";
            for sid in params {
                write!(self.w, "{sep}{}", FmtSid(*sid))?;
                sep = ", ";
            }
            writeln!(self.w, "):")?;
        }
        Ok(())
    }

    pub(crate) fn call(&mut self, target: &str, params: impl VarArgs) -> Result<Sid> {
        let dst = self.alloc_sid();
        write!(self.w, "{INDENT}{dst} = {target}(", dst = FmtSid(dst))?;
        let mut sep = "";
        let params = params.into_exprs();
        for param in params {
            write!(self.w, "{sep}{}", FmtExpr(self.strings, &param))?;
            sep = ", ";
        }
        writeln!(self.w, ")")?;
        Ok(dst)
    }

    pub(crate) fn copy(&mut self, src: impl Into<Expr>) -> Result<Sid> {
        self.call("copy", vec![src])
    }

    pub(crate) fn load(&mut self, ty: Ty, src: impl Into<Expr>) -> Result<Sid> {
        let src = src.into();
        let dst = self.alloc_sid();
        writeln!(
            self.w,
            "{INDENT}{dst}: {ty} = load {src}",
            dst = FmtSid(dst),
            ty = FmtTy(&ty),
            src = FmtExpr(self.strings, &src)
        )?;
        Ok(dst)
    }

    pub(crate) fn prune(&mut self, expr: impl Into<Expr>) -> Result {
        let expr = expr.into();
        writeln!(self.w, "{INDENT}prune {}", FmtExpr(self.strings, &expr))?;
        Ok(())
    }

    pub(crate) fn prune_not(&mut self, expr: impl Into<Expr>) -> Result {
        let expr = expr.into();
        writeln!(self.w, "{INDENT}prune ! {}", FmtExpr(self.strings, &expr))?;
        Ok(())
    }

    pub(crate) fn ret(&mut self, expr: impl Into<Expr>) -> Result {
        let expr = expr.into();
        writeln!(self.w, "{INDENT}ret {}", FmtExpr(self.strings, &expr))?;
        Ok(())
    }

    pub(crate) fn store(
        &mut self,
        dst: impl Into<Expr>,
        src: impl Into<Expr>,
        src_ty: Ty,
    ) -> Result {
        let dst = dst.into();
        let src = src.into();
        writeln!(
            self.w,
            "{INDENT}store {dst} <- {src}: {ty}",
            dst = FmtExpr(self.strings, &dst),
            src = FmtExpr(self.strings, &src),
            ty = FmtTy(&src_ty),
        )?;
        Ok(())
    }

    pub(crate) fn unreachable(&mut self) -> Result {
        writeln!(self.w, "{INDENT}ret 0 // unreachable")?;
        Ok(())
    }

    pub(crate) fn write_loc(&mut self, src_loc: &SrcLoc) -> Result {
        if src_loc.filename != self.cur_loc.filename {
            write_full_loc(self.w, src_loc, self.strings)?;
            self.cur_loc = src_loc.clone();
        } else if src_loc.line_begin != self.cur_loc.line_begin {
            write_line_loc(self.w, src_loc)?;
            self.cur_loc = src_loc.clone();
        }
        Ok(())
    }
}

pub(crate) fn write_type(
    w: &mut dyn std::io::Write,
    name: &str,
    src_loc: &SrcLoc,
    fields: &[(&str, Ty)],
    strings: &StringInterner,
) -> Result {
    write_full_loc(w, src_loc, strings)?;

    write!(w, "type {name} = {{")?;

    let mut sep = "\n";
    for (name, ty) in fields {
        write!(w, "{sep}{INDENT}{name}: {ty}", ty = FmtTy(ty))?;
        sep = ";\n";
    }

    writeln!(w, "\n}}")?;
    writeln!(w)?;

    Ok(())
}

pub(crate) fn declare_global(w: &mut dyn std::io::Write, name: &str, ty: Ty) -> Result {
    writeln!(w, "global {name} // {}", FmtTy(&ty))?;
    writeln!(w)?;
    Ok(())
}

pub(crate) fn declare_function(
    w: &mut dyn std::io::Write,
    name: &str,
    tys: &[Ty],
    ret_ty: Ty,
) -> Result {
    write!(w, "declare {name}(")?;

    let mut sep = "";
    for ty in tys {
        write!(w, "{sep}{}", FmtTy(ty))?;
        sep = ", ";
    }

    writeln!(w, "): {}", FmtTy(&ret_ty))?;
    Ok(())
}
