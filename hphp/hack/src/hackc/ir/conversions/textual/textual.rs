// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! This is a structural version of the Textual language - it should have
//! basically no business logic and just provides a type-safe way to write
//! Textual.

use std::fmt;
use std::sync::Arc;

use anyhow::Error;
use ascii::AsciiString;
use hash::HashMap;
use hash::HashSet;
use ir::func::SrcLoc;
use ir::BlockId;
use ir::LocalId;
use ir::StringInterner;
use itertools::Itertools;
use newtype::newtype_int;

pub(crate) const INDENT: &str = "  ";

type Result<T = (), E = Error> = std::result::Result<T, E>;

pub(crate) struct TextualFile<'a> {
    w: &'a mut dyn std::io::Write,
    strings: Arc<StringInterner>,
    pub(crate) internal_functions: HashSet<String>,
    pub(crate) called_functions: HashSet<String>,
    pub(crate) globals: HashMap<String, Ty>,
}

impl<'a> TextualFile<'a> {
    pub(crate) fn new(w: &'a mut dyn std::io::Write, strings: Arc<StringInterner>) -> Self {
        TextualFile {
            w,
            strings,
            internal_functions: Default::default(),
            called_functions: Default::default(),
            globals: Default::default(),
        }
    }

    pub(crate) fn debug_separator(&mut self) -> Result {
        writeln!(self.w)?;
        Ok(())
    }

    pub(crate) fn declare_function(&mut self, name: &str, tys: &[Ty], ret_ty: &Ty) -> Result {
        write!(self.w, "declare {name}(")?;

        let mut sep = "";
        for ty in tys {
            write!(self.w, "{sep}{}", FmtTy(ty))?;
            sep = ", ";
        }

        writeln!(self.w, "): {}", FmtTy(&ret_ty))?;
        Ok(())
    }

    pub(crate) fn declare_global(&mut self, name: String, ty: Ty) {
        self.globals.insert(name, ty);
    }

    fn declare_unknown_function(&mut self, name: &str) -> Result {
        writeln!(self.w, "declare {name}(...): *HackMixed")?;
        Ok(())
    }

    pub(crate) fn define_function<R>(
        &mut self,
        name: &str,
        loc: &SrcLoc,
        params: &[(&str, &Ty)],
        ret_ty: &Ty,
        locals: &[(LocalId, &Ty)],
        body: impl FnOnce(&mut FuncBuilder<'_, '_>) -> Result<R>,
    ) -> Result<R> {
        if !self.internal_functions.contains(name) {
            self.internal_functions.insert(name.to_owned());
        }

        self.write_full_loc(loc)?;

        write!(self.w, "define {name}(")?;
        let mut sep = "";
        for (name, ty) in params {
            write!(self.w, "{sep}{name}: {ty}", ty = FmtTy(ty))?;
            sep = ", ";
        }
        writeln!(self.w, ") : {} {{", FmtTy(&ret_ty))?;

        if !locals.is_empty() {
            let mut sep = "";
            write!(self.w, "local ")?;
            for (lid, ty) in locals {
                write!(
                    self.w,
                    "{sep}{name}: {ty}",
                    name = FmtLid(&self.strings, *lid),
                    ty = FmtTy(ty)
                )?;
                sep = ", ";
            }
            writeln!(self.w)?;
        }

        let mut writer = FuncBuilder {
            cur_loc: loc.clone(),
            next_id: Sid::from_usize(0),
            txf: self,
        };

        writer.write_label(BlockId::from_usize(0), &[])?;
        let result = body(&mut writer)?;

        writeln!(self.w, "}}")?;
        writeln!(self.w)?;
        Ok(result)
    }

    pub(crate) fn define_type<'s>(
        &mut self,
        name: &str,
        src_loc: &SrcLoc,
        fields: impl Iterator<Item = Field<'s>>,
    ) -> Result {
        self.write_full_loc(src_loc)?;

        write!(self.w, "type {name} = {{")?;

        let mut sep = "\n";
        for f in fields {
            write!(
                self.w,
                "{sep}{INDENT}{name}: {vis} {ty}",
                name = f.name,
                ty = FmtTy(f.ty),
                vis = f.visibility.decl()
            )?;
            sep = ";\n";
        }

        writeln!(self.w, "\n}}")?;
        writeln!(self.w)?;

        Ok(())
    }

    pub(crate) fn set_attribute(&mut self, attr: Attribute) -> Result {
        match attr {
            Attribute::SourceLanguage(lang) => {
                writeln!(self.w, ".source_language = \"{lang}\"")?;
            }
        }
        Ok(())
    }

    pub(crate) fn write_comment(&mut self, msg: &str) -> Result {
        writeln!(self.w, "// {msg}")?;
        Ok(())
    }

    pub(crate) fn write_epilogue<T: Eq + std::hash::Hash + Copy>(
        &mut self,
        builtins: &HashMap<&str, T>,
    ) -> Result<HashSet<T>> {
        if !self.globals.is_empty() {
            self.write_comment("----- GLOBALS -----")?;

            for (name, ty) in self.globals.iter().sorted_by(|(n1, _), (n2, _)| n1.cmp(n2)) {
                writeln!(self.w, "global {name} : {}", FmtTy(ty))?;
            }
            self.debug_separator()?;
        }

        let (builtins, mut non_builtins): (HashSet<T>, Vec<String>) = (&self.called_functions
            - &self.internal_functions)
            .into_iter()
            .partition_map(|f| match builtins.get(&f as &str) {
                Some(b) => itertools::Either::Left(b),
                None => itertools::Either::Right(f),
            });
        non_builtins.sort();
        if !non_builtins.is_empty() {
            self.write_comment("----- EXTERNALS -----")?;
            for name in non_builtins {
                self.declare_unknown_function(&name)?;
            }
            self.debug_separator()?;
        }

        Ok(builtins)
    }

    fn write_full_loc(&mut self, src_loc: &SrcLoc) -> Result {
        let filename = self.strings.lookup_bstr(src_loc.filename.0);
        writeln!(self.w, "// .file \"{filename}\"")?;
        drop(filename);
        self.write_line_loc(src_loc)?;
        Ok(())
    }

    fn write_line_loc(&mut self, src_loc: &SrcLoc) -> Result {
        writeln!(self.w, "// .line {}", src_loc.line_begin)?;
        Ok(())
    }
}

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

struct FmtSid(Sid);

impl fmt::Display for FmtSid {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let FmtSid(sid) = *self;
        write!(f, "n{}", sid.as_usize())
    }
}

#[derive(Debug, Clone, Hash, Eq, PartialEq)]
pub(crate) enum Ty {
    Ellipsis,
    Float,
    HackMixedPtr,
    Int,
    Noreturn,
    Ptr(Box<Ty>),
    Type(String),
    String,
    Void,
    VoidPtr,
}

impl From<Ty> for std::borrow::Cow<'_, Ty> {
    fn from(ty: Ty) -> Self {
        Self::Owned(ty)
    }
}

impl<'a> From<&'a Ty> for std::borrow::Cow<'a, Ty> {
    fn from(ty: &'a Ty) -> Self {
        Self::Borrowed(ty)
    }
}

struct FmtTy<'a>(&'a Ty);

impl fmt::Display for FmtTy<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.0 {
            Ty::Ellipsis => write!(f, "..."),
            Ty::Float => write!(f, "float"),
            Ty::HackMixedPtr => f.write_str("*HackMixed"),
            Ty::Int => write!(f, "int"),
            Ty::Noreturn => f.write_str("noreturn"),
            Ty::Ptr(sub) => write!(f, "*{}", FmtTy(sub)),
            Ty::String => write!(f, "*string"),
            Ty::Type(s) => write!(f, "{s}"),
            Ty::Void => f.write_str("void"),
            Ty::VoidPtr => f.write_str("*void"),
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
    Float(f64),
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
            Const::Float(d) => write!(f, "{d:?}"),
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
    /// foo(1, 2, 3)
    Call(String, Box<[Expr]>),
    /// 0, null, etc
    Const(Const),
    /// *Variable
    Deref(Box<Expr>),
    /// a.b
    Field(Box<Expr>, String),
    /// a[b]
    Index(Box<Expr>, Box<Expr>),
    Sid(Sid),
    // TyAscription(Box<Expr>, Ty),
    Var(Var),
}

impl Expr {
    pub(crate) fn call(target: impl ToString, params: impl VarArgs) -> Expr {
        Expr::Call(target.to_string(), params.into_exprs().into_boxed_slice())
    }

    pub(crate) fn deref(v: impl Into<Expr>) -> Expr {
        Expr::Deref(Box::new(v.into()))
    }

    #[allow(dead_code)]
    pub(crate) fn field(base: impl Into<Expr>, name: impl Into<String>) -> Expr {
        let base = base.into();
        let name = name.into();
        Expr::Field(Box::new(base), name)
    }

    #[allow(dead_code)]
    pub(crate) fn index(base: impl Into<Expr>, offset: impl Into<Expr>) -> Expr {
        let base = base.into();
        let offset = offset.into();
        Expr::Index(Box::new(base), Box::new(offset))
    }

    pub(crate) fn null() -> Expr {
        Expr::Const(Const::Null)
    }
}

impl From<&'static str> for Expr {
    fn from(s: &'static str) -> Self {
        Expr::Const(Const::String(AsciiString::from_ascii(s).unwrap()))
    }
}

impl From<AsciiString> for Expr {
    fn from(s: AsciiString) -> Self {
        Expr::Const(Const::String(s))
    }
}

impl From<bool> for Expr {
    fn from(v: bool) -> Self {
        if v {
            Expr::Const(Const::True)
        } else {
            Expr::Const(Const::False)
        }
    }
}

impl From<f64> for Expr {
    fn from(f: f64) -> Self {
        Expr::Const(Const::Float(f))
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

impl From<Var> for Expr {
    fn from(var: Var) -> Self {
        Expr::Var(var)
    }
}

impl From<LocalId> for Expr {
    fn from(lid: LocalId) -> Expr {
        Expr::Var(lid.into())
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

impl<T> VarArgs for Box<[T]>
where
    T: Into<Expr>,
{
    fn into_exprs(self) -> Vec<Expr> {
        Vec::from(self).into_exprs()
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

impl<T, I, O, F> VarArgs for std::iter::Map<I, F>
where
    T: Into<Expr>,
    I: Iterator<Item = O>,
    F: FnMut(O) -> T,
{
    fn into_exprs(self) -> Vec<Expr> {
        self.map_into().collect_vec()
    }
}

pub(crate) enum Attribute {
    SourceLanguage(String),
}

pub(crate) struct FuncBuilder<'a, 'b> {
    cur_loc: SrcLoc,
    next_id: Sid,
    pub(crate) txf: &'a mut TextualFile<'b>,
}

impl FuncBuilder<'_, '_> {
    pub fn alloc_sid(&mut self) -> Sid {
        let id = self.next_id;
        self.next_id = Sid::from_usize(id.as_usize() + 1);
        id
    }
}

impl FuncBuilder<'_, '_> {
    pub(crate) fn jmp(&mut self, targets: &[BlockId], params: impl VarArgs) -> Result {
        assert!(!targets.is_empty());

        let params = params.into_exprs();
        write!(self.txf.w, "{INDENT}jmp ")?;

        let mut sep1 = "";
        for &target in targets {
            write!(self.txf.w, "{sep1}{}", FmtBid(target))?;
            sep1 = ", ";

            if !params.is_empty() {
                write!(self.txf.w, "(")?;
                let mut sep2 = "";
                for param in params.iter() {
                    self.txf.w.write_all(sep2.as_bytes())?;
                    self.write_expr(param)?;
                    sep2 = ", ";
                }
                write!(self.txf.w, ")")?;
            }
        }

        writeln!(self.txf.w)?;

        Ok(())
    }

    pub(crate) fn write_exception_handler(&mut self, target: BlockId) -> Result {
        writeln!(self.txf.w, "{INDENT}.handlers {}", FmtBid(target))?;
        Ok(())
    }

    fn write_expr(&mut self, expr: &Expr) -> Result {
        match *expr {
            Expr::Call(ref target, ref params) => {
                if !self.txf.called_functions.contains(target) {
                    self.txf.called_functions.insert(target.to_owned());
                }
                write!(self.txf.w, "{}(", target)?;
                let mut sep = "";
                for param in params.iter() {
                    self.txf.w.write_all(sep.as_bytes())?;
                    self.write_expr(param)?;
                    sep = ", ";
                }
                write!(self.txf.w, ")")?;
            }
            Expr::Const(ref c) => write!(self.txf.w, "{}", FmtConst(c))?,
            Expr::Deref(ref var) => {
                self.txf.w.write_all(b"&")?;
                self.write_expr(var)?;
            }
            Expr::Field(ref base, ref name) => {
                self.write_expr(base)?;
                write!(self.txf.w, ".{name}")?;
            }
            Expr::Index(ref base, ref offset) => {
                self.write_expr(base)?;
                self.txf.w.write_all(b"[")?;
                self.write_expr(offset)?;
                self.txf.w.write_all(b"]")?;
            }
            Expr::Sid(sid) => write!(self.txf.w, "{}", FmtSid(sid))?,
            Expr::Var(ref var) => write!(self.txf.w, "{}", FmtVar(&self.txf.strings, var))?,
        }
        Ok(())
    }

    pub(crate) fn write_expr_stmt(&mut self, expr: impl Into<Expr>) -> Result<Sid> {
        let expr = expr.into();
        match expr {
            Expr::Sid(sid) => Ok(sid),
            Expr::Call(target, params) => self.call(&target, params),
            Expr::Const(c) => {
                let sid = self.alloc_sid();
                writeln!(
                    self.txf.w,
                    "{INDENT}{dst} = {src}",
                    dst = FmtSid(sid),
                    src = FmtConst(&c)
                )?;
                Ok(sid)
            }
            Expr::Deref(_) | Expr::Field(_, _) | Expr::Index(_, _) | Expr::Var(_) => {
                todo!("EXPR: {expr:?}")
            }
        }
    }

    pub(crate) fn write_label(&mut self, bid: BlockId, params: &[Sid]) -> Result {
        if params.is_empty() {
            writeln!(self.txf.w, "#{}:", FmtBid(bid))?;
        } else {
            write!(self.txf.w, "#{}(", FmtBid(bid))?;
            let mut sep = "";
            for sid in params {
                write!(self.txf.w, "{sep}{}: *HackMixed", FmtSid(*sid))?;
                sep = ", ";
            }
            writeln!(self.txf.w, "):")?;
        }
        Ok(())
    }

    /// Call the target as a static call (without virtual dispatch).
    pub(crate) fn call_static(
        &mut self,
        target: &str,
        this: Expr,
        params: impl VarArgs,
    ) -> Result<Sid> {
        if !self.txf.called_functions.contains(target) {
            self.txf.called_functions.insert(target.to_owned());
        }
        let dst = self.alloc_sid();
        write!(self.txf.w, "{INDENT}{dst} = {target}(", dst = FmtSid(dst),)?;
        self.write_expr(&this)?;
        let params = params.into_exprs();
        for param in params {
            self.txf.w.write_all(b", ")?;
            self.write_expr(&param)?;
        }
        writeln!(self.txf.w, ")")?;
        Ok(dst)
    }

    /// Call the target as a virtual call.
    pub(crate) fn call_virtual(
        &mut self,
        target: &str,
        this: Expr,
        params: impl VarArgs,
    ) -> Result<Sid> {
        if !self.txf.called_functions.contains(target) {
            self.txf.called_functions.insert(target.to_owned());
        }
        let dst = self.alloc_sid();
        write!(self.txf.w, "{INDENT}{dst} = ", dst = FmtSid(dst),)?;
        self.write_expr(&this)?;
        write!(self.txf.w, ".{target}(")?;
        let params = params.into_exprs();
        let mut sep = "";
        for param in params {
            self.txf.w.write_all(sep.as_bytes())?;
            self.write_expr(&param)?;
            sep = ", ";
        }
        writeln!(self.txf.w, ")")?;
        Ok(dst)
    }

    pub(crate) fn call(&mut self, target: &str, params: impl VarArgs) -> Result<Sid> {
        if !self.txf.called_functions.contains(target) {
            self.txf.called_functions.insert(target.to_owned());
        }
        let dst = self.alloc_sid();
        write!(self.txf.w, "{INDENT}{dst} = {target}(", dst = FmtSid(dst))?;
        let mut sep = "";
        let params = params.into_exprs();
        for param in params {
            self.txf.w.write_all(sep.as_bytes())?;
            self.write_expr(&param)?;
            sep = ", ";
        }
        writeln!(self.txf.w, ")")?;
        Ok(dst)
    }

    pub(crate) fn comment(&mut self, msg: &str) -> Result<()> {
        writeln!(self.txf.w, "// {msg}")?;
        Ok(())
    }

    pub(crate) fn copy(&mut self, src: impl Into<Expr>) -> Result<Sid> {
        let src = src.into();
        let dst = self.alloc_sid();
        write!(self.txf.w, "{INDENT}{dst} = ", dst = FmtSid(dst),)?;
        self.write_expr(&src)?;
        writeln!(self.txf.w)?;
        Ok(dst)
    }

    pub(crate) fn load(&mut self, ty: &Ty, src: impl Into<Expr>) -> Result<Sid> {
        let src = src.into();
        let dst = self.alloc_sid();
        write!(
            self.txf.w,
            "{INDENT}{dst}: {ty} = load ",
            dst = FmtSid(dst),
            ty = FmtTy(&ty),
        )?;
        self.write_expr(&src)?;
        writeln!(self.txf.w)?;
        Ok(dst)
    }

    // Terminate this branch if `expr` is false.
    pub(crate) fn prune(&mut self, expr: impl Into<Expr>) -> Result {
        let expr = expr.into();
        write!(self.txf.w, "{INDENT}prune ")?;
        self.write_expr(&expr)?;
        writeln!(self.txf.w)?;
        Ok(())
    }

    // Terminate this branch if `expr` is true.
    pub(crate) fn prune_not(&mut self, expr: impl Into<Expr>) -> Result {
        let expr = expr.into();
        write!(self.txf.w, "{INDENT}prune ! ")?;
        self.write_expr(&expr)?;
        writeln!(self.txf.w)?;
        Ok(())
    }

    pub(crate) fn ret(&mut self, expr: impl Into<Expr>) -> Result {
        let expr = expr.into();
        write!(self.txf.w, "{INDENT}ret ",)?;
        self.write_expr(&expr)?;
        writeln!(self.txf.w)?;
        Ok(())
    }

    pub(crate) fn store(
        &mut self,
        dst: impl Into<Expr>,
        src: impl Into<Expr>,
        src_ty: &Ty,
    ) -> Result {
        let dst = dst.into();
        let src = src.into();
        write!(self.txf.w, "{INDENT}store ")?;
        self.write_expr(&dst)?;
        self.txf.w.write_all(b" <- ")?;
        self.write_expr(&src)?;
        writeln!(self.txf.w, ": {ty}", ty = FmtTy(&src_ty))?;
        Ok(())
    }

    pub(crate) fn unreachable(&mut self) -> Result {
        writeln!(self.txf.w, "{INDENT}unreachable")?;
        Ok(())
    }

    pub(crate) fn write_loc(&mut self, src_loc: &SrcLoc) -> Result {
        if src_loc.filename != self.cur_loc.filename {
            self.txf.write_full_loc(src_loc)?;
            self.cur_loc = src_loc.clone();
        } else if src_loc.line_begin != self.cur_loc.line_begin {
            self.txf.write_line_loc(src_loc)?;
            self.cur_loc = src_loc.clone();
        }
        Ok(())
    }
}

#[derive(Debug, Clone, Copy, Eq, PartialEq, Hash)]
pub(crate) enum Visibility {
    Public,
    Private,
    Protected,
}

impl Visibility {
    fn decl(&self) -> &str {
        match self {
            Visibility::Public => ".public",
            Visibility::Private => ".private",
            Visibility::Protected => ".protected",
        }
    }
}

pub(crate) struct Field<'a> {
    pub name: &'a str,
    pub ty: &'a Ty,
    pub visibility: Visibility,
}
