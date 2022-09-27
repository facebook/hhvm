// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! To support late static binding classes are set up as a two tier system -
//! every class has a singleton "static class" which holds its static members.
//!
//! `(new C())->a` refers to the property "a" of an instance of "C".
//! `C::$a` refers to the property "$a" of the static class for "C".

use anyhow::Error;
use strum_macros::EnumIter;

use super::textual;
use crate::mangle::MangleId;

type Result<T = (), E = Error> = std::result::Result<T, E>;

#[derive(Copy, Clone, Eq, PartialEq, EnumIter)]
pub(crate) enum IsStatic {
    Static,
    NonStatic,
}

pub(crate) struct StaticClassId(pub(crate) ir::ClassId);

impl MangleId for StaticClassId {
    fn mangle(&self, strings: &ir::StringInterner) -> String {
        format!("static::{}", self.0.mangle(strings))
    }
}

pub(crate) fn mangled_class_name(
    name: ir::ClassId,
    is_static: IsStatic,
    strings: &ir::StringInterner,
) -> String {
    match is_static {
        IsStatic::Static => StaticClassId(name).mangle(strings),
        IsStatic::NonStatic => name.mangle(strings),
    }
}

pub(crate) fn load_static_class(
    w: &mut textual::FuncWriter<'_>,
    class: ir::ClassId,
    strings: &ir::StringInterner,
) -> Result<textual::Sid> {
    let name = mangled_class_name(class, IsStatic::Static, strings);
    w.load(
        tx_ty!(*void),
        textual::Expr::deref(textual::Var::named(name)),
    )
}
