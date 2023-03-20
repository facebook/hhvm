// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use std::fmt;

use ir::StringInterner;
use naming_special_names_rust::members;

use crate::class::IsStatic;

pub(crate) const TOP_LEVELS_CLASS: &str = "$root";

/// Used for things that can mangle themselves directly.
pub(crate) trait Mangle {
    fn mangle(&self, strings: &StringInterner) -> String;
}

impl Mangle for [u8] {
    fn mangle(&self, _strings: &StringInterner) -> String {
        // Handle some reserved tokens.
        match self {
            b"declare" => "declare_".to_owned(),
            b"define" => "define_".to_owned(),
            b"extends" => "extends_".to_owned(),
            b"false" => "false_".to_owned(),
            b"float" => "float_".to_owned(),
            b"global" => "global_".to_owned(),
            b"int" => "int_".to_owned(),
            b"jmp" => "jmp_".to_owned(),
            b"load" => "load_".to_owned(),
            b"local" => "local_".to_owned(),
            b"null" => "null_".to_owned(),
            b"prune" => "prune_".to_owned(),
            b"ret" => "ret_".to_owned(),
            b"store" => "store_".to_owned(),
            b"throw" => "throw_".to_owned(),
            b"true" => "true_".to_owned(),
            b"type" => "type_".to_owned(),
            b"unreachable" => "unreachable_".to_owned(),
            b"void" => "void_".to_owned(),
            _ => {
                // [A-Za-z0-9_$] -> identity
                // \ -> ::
                // anything else -> $xx
                let mut res = String::with_capacity(self.len());
                let mut first = true;
                for &ch in self {
                    if (b'A'..=b'Z').contains(&ch)
                        || (b'a'..=b'z').contains(&ch)
                        || (ch == b'_')
                        || (ch == b'$')
                    {
                        res.push(ch as char);
                    } else if (b'0'..=b'9').contains(&ch) {
                        if first {
                            res.push('_')
                        }
                        res.push(ch as char);
                    } else if ch == b'\\' {
                        res.push(':');
                        res.push(':');
                    } else {
                        res.push(b"0123456789abcdef"[(ch >> 4) as usize] as char);
                        res.push(b"0123456789abcdef"[(ch & 15) as usize] as char);
                    }
                    first = false;
                }
                res
            }
        }
    }
}

// Classes and functions live in different namespaces.

impl Mangle for ir::PropId {
    fn mangle(&self, strings: &StringInterner) -> String {
        self.as_bytes(strings).mangle(strings)
    }
}

#[derive(Eq, PartialEq, Hash, Clone, Debug)]
pub(crate) enum Intrinsic {
    AllocCurry,
    Construct(ir::ClassId),
    Factory(ir::ClassId),
    InitStatic(ir::ClassId),
    Invoke(TypeName),
    PropInit(ir::ClassId),
}

/// Represents a named callable thing.  This includes top-level functions and
/// methods.
#[derive(Eq, PartialEq, Hash, Clone, Debug)]
pub(crate) enum FunctionName {
    Builtin(crate::hack::Builtin),
    Function(ir::FunctionId),
    Intrinsic(Intrinsic),
    Method(TypeName, ir::MethodId),
    Unmangled(String),
}

impl FunctionName {
    pub(crate) fn method(class: ir::ClassId, is_static: IsStatic, method: ir::MethodId) -> Self {
        Self::Method(TypeName::class(class, is_static), method)
    }

    pub(crate) fn display<'r>(&'r self, strings: &'r StringInterner) -> impl fmt::Display + 'r {
        FmtFunctionName(strings, self)
    }

    pub(crate) fn cmp(&self, other: &Self, strings: &StringInterner) -> Ordering {
        let a = self.display(strings).to_string();
        let b = other.display(strings).to_string();
        a.cmp(&b)
    }
}

struct FmtFunctionName<'a>(&'a StringInterner, &'a FunctionName);

impl fmt::Display for FmtFunctionName<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let FmtFunctionName(strings, name) = *self;
        match name {
            FunctionName::Builtin(b) => b.as_str().fmt(f)?,
            FunctionName::Function(fid) => write!(
                f,
                "{TOP_LEVELS_CLASS}.{}",
                fid.as_bytes(strings).mangle(strings)
            )?,
            FunctionName::Intrinsic(intrinsic) => {
                let tn;
                let (ty, name) = match intrinsic {
                    Intrinsic::AllocCurry => (None, "__sil_allocate_curry"),
                    Intrinsic::Construct(cid) => {
                        tn = TypeName::Class(*cid);
                        (Some(&tn), members::__CONSTRUCT)
                    }
                    Intrinsic::Factory(cid) => {
                        tn = TypeName::StaticClass(*cid);
                        (Some(&tn), "__factory")
                    }
                    Intrinsic::InitStatic(cid) => {
                        tn = TypeName::StaticClass(*cid);
                        (Some(&tn), "$init_static")
                    }
                    Intrinsic::Invoke(tn) => (Some(tn), "__invoke"),
                    Intrinsic::PropInit(cid) => {
                        tn = TypeName::Class(*cid);
                        (Some(&tn), "_86pinit")
                    }
                };
                if let Some(ty) = ty {
                    write!(f, "{}.{}", ty.display(strings), name)?;
                } else {
                    f.write_str(name)?;
                }
            }
            FunctionName::Method(class, mid) => {
                write!(
                    f,
                    "{}.{}",
                    class.display(strings),
                    mid.as_bytes(strings).mangle(strings)
                )?;
            }
            FunctionName::Unmangled(s) => s.fmt(f)?,
        }
        Ok(())
    }
}

#[derive(Eq, PartialEq, Hash, Clone, Debug)]
pub(crate) enum GlobalName {
    Global(ir::GlobalId),
    StaticConst(ir::ClassId, ir::ConstId),
}

impl GlobalName {
    pub(crate) fn display<'r>(&'r self, strings: &'r StringInterner) -> impl fmt::Display + 'r {
        FmtGlobalName(strings, self)
    }

    pub(crate) fn cmp(&self, other: &Self, strings: &StringInterner) -> Ordering {
        let a = self.display(strings).to_string();
        let b = other.display(strings).to_string();
        a.cmp(&b)
    }
}

struct FmtGlobalName<'a>(&'a StringInterner, &'a GlobalName);

impl fmt::Display for FmtGlobalName<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let FmtGlobalName(strings, name) = *self;
        match name {
            GlobalName::Global(id) => {
                write!(f, "global::{}", id.as_bytes(strings).mangle(strings))
            }
            GlobalName::StaticConst(class, cid) => {
                write!(
                    f,
                    "const::{}::{}",
                    TypeName::StaticClass(*class).display(strings),
                    cid.as_bytes(strings).mangle(strings)
                )
            }
        }
    }
}

#[derive(Debug, Clone, Hash, Eq, PartialEq)]
pub(crate) enum TypeName {
    Class(ir::ClassId),
    HackMixed,
    StaticClass(ir::ClassId),
    UnmangledRef(&'static str),
}

impl TypeName {
    pub(crate) fn class(class: ir::ClassId, is_static: crate::class::IsStatic) -> Self {
        match is_static {
            IsStatic::Static => Self::StaticClass(class),
            IsStatic::NonStatic => Self::Class(class),
        }
    }

    pub(crate) fn display<'r>(&'r self, strings: &'r StringInterner) -> impl fmt::Display + 'r {
        FmtTypeName(strings, self)
    }
}

struct FmtTypeName<'a>(&'a StringInterner, &'a TypeName);

impl fmt::Display for FmtTypeName<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let FmtTypeName(strings, name) = *self;
        match name {
            TypeName::Class(cid) => f.write_str(&cid.as_bytes(strings).mangle(strings)),
            TypeName::HackMixed => f.write_str("HackMixed"),
            TypeName::StaticClass(cid) => {
                f.write_str(&cid.as_bytes(strings).mangle(strings))?;
                f.write_str("$static")
            }
            TypeName::UnmangledRef(s) => s.fmt(f),
        }
    }
}
