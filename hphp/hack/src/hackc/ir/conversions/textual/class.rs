// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! To support late static binding classes are set up as a two tier system -
//! every class has a singleton "static class" which holds its static members.
//!
//! `(new C())->a` refers to the property "a" of an instance of "C".
//! `C::$a` refers to the property "$a" of the static class for "C".
//!
//! To get the static class singleton call `load_static_class`.

use anyhow::Error;
use log::trace;
use strum_macros::EnumIter;

use super::func;
use super::hack;
use super::textual;
use crate::mangle::Mangle;
use crate::mangle::MangleWithClass as _;
use crate::state::UnitState;

type Result<T = (), E = Error> = std::result::Result<T, E>;

#[derive(Copy, Clone, Eq, PartialEq, EnumIter)]
pub(crate) enum IsStatic {
    Static,
    NonStatic,
}

pub(crate) struct StaticClassId(pub(crate) ir::ClassId);

impl Mangle for StaticClassId {
    fn mangle(&self, strings: &ir::StringInterner) -> String {
        format!("{}$static", self.0.mangle(strings))
    }
}

/// Classes are defined as:
///
/// type NAME = [ properties*; ]
///
pub(crate) fn write_class(
    w: &mut dyn std::io::Write,
    state: &mut UnitState,
    mut class: ir::Class<'_>,
) -> Result {
    trace!("Convert Class {}", class.name.as_bstr(&state.strings));

    write_type(w, state, &class, IsStatic::Static)?;
    write_type(w, state, &class, IsStatic::NonStatic)?;

    write_init_static(w, state, &class)?;

    let methods = std::mem::take(&mut class.methods);
    for method in methods {
        write_method(w, state, &class, method)?;
    }

    Ok(())
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

/// For a given class return the Ty for its non-static (instance) type.
pub(crate) fn non_static_ty(class: ir::ClassId, strings: &ir::StringInterner) -> textual::Ty {
    let cname = mangled_class_name(class, IsStatic::NonStatic, strings);
    textual::Ty::Ptr(Box::new(textual::Ty::Type(cname)))
}

/// For a given class return the Ty for its static type.
pub(crate) fn static_ty(class: ir::ClassId, strings: &ir::StringInterner) -> textual::Ty {
    let cname = mangled_class_name(class, IsStatic::Static, strings);
    textual::Ty::Ptr(Box::new(textual::Ty::Type(cname)))
}

/// For a given class return the Ty for its non-static type.
fn init_static_name(class: ir::ClassId, strings: &ir::StringInterner) -> String {
    let method = ir::MethodName::new(ffi::Slice::new(b"$init_static"));
    method.mangle(class, strings)
}

/// The name of the global singleton for a static class.
pub(crate) fn static_singleton_name(class: ir::ClassId, strings: &ir::StringInterner) -> String {
    let cname = class.mangle(strings);
    format!("static_singleton::{cname}")
}

/// Write the type for a (class, is_static) with the properties of the class.
fn write_type(
    w: &mut dyn std::io::Write,
    state: &mut UnitState,
    class: &ir::Class<'_>,
    is_static: IsStatic,
) -> Result {
    let fields = Vec::new();
    if !class.properties.is_empty() {
        todo!();
    }

    let cname = mangled_class_name(class.name, is_static, &state.strings);
    textual::write_type(w, &cname, &class.src_loc, &fields, &state.strings)?;
    Ok(())
}

/// Build the `init_static` function for a static class.
///
/// Declares globals for:
///   - static singleton
/// Writes the `init_static` function itself which initializes the globals and
/// returns the memoized static singleton.
fn write_init_static(
    w: &mut dyn std::io::Write,
    state: &mut UnitState,
    class: &ir::Class<'_>,
) -> Result {
    textual::declare_global(
        w,
        &static_singleton_name(class.name, &state.strings),
        textual::Ty::Ptr(Box::new(static_ty(class.name, &state.strings))),
    )?;

    textual::write_function(
        w,
        &state.strings,
        &init_static_name(class.name, &state.strings),
        &class.src_loc,
        &[],
        tx_ty!(void),
        |w| {
            let sz = 0; // TODO: properties
            let p = hack::call_builtin(w, hack::Builtin::AllocWords, [sz])?;

            let singleton_name = static_singleton_name(class.name, &state.strings);
            let singleton_expr = textual::Expr::deref(textual::Var::named(singleton_name));
            w.store(singleton_expr, p, static_ty(class.name, &state.strings))?;

            w.ret(0)?;
            Ok(())
        },
    )
}

fn write_method(
    w: &mut dyn std::io::Write,
    state: &mut UnitState,
    class: &ir::Class<'_>,
    method: ir::Method<'_>,
) -> Result {
    trace!(
        "Convert Method {}::{}",
        class.name.as_bstr(&state.strings),
        method.name.as_bstr()
    );

    let this_ty = if method.attrs.is_static() {
        static_ty(class.name, &state.strings)
    } else {
        non_static_ty(class.name, &state.strings)
    };

    func::write_func(
        w,
        state,
        &method.name.mangle(class.name, &state.strings),
        this_ty,
        method.func,
    )
}

/// Loads the static singleton for a class.
pub(crate) fn load_static_class(
    w: &mut textual::FuncWriter<'_>,
    class: ir::ClassId,
    strings: &ir::StringInterner,
) -> Result<textual::Sid> {
    // Blindly load the static singleton, assuming it's already been initialized.
    let singleton_name = static_singleton_name(class, strings);
    let singleton_expr = textual::Expr::deref(textual::Var::named(singleton_name));
    let value = w.load(static_ty(class, strings), singleton_expr)?;
    w.call("__sil_lazy_initialize", [value])?;
    Ok(value)
}
