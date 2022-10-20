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
//! All classes (static or non-static have a "vtable" field which is used to
//! dispatch to well-known methods (defined in VTableIndex).
//!
//! To get the static class singleton call the class `get_static` function which
//! initializes and returns the cached singleton (see `load_static_class`).

use anyhow::Error;
use ir::BlockId;
use log::trace;
use strum_macros::EnumIter;

use super::func;
use super::hack;
use super::textual;
use crate::mangle::MangleClassId;
use crate::mangle::MangleId;
use crate::state::UnitState;

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

    write_get_static(w, state, &class)?;

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

/// For a given class return the Ty for its static type.
pub(crate) fn static_ty(class: ir::ClassId, strings: &ir::StringInterner) -> textual::Ty {
    let cname = mangled_class_name(class, IsStatic::Static, strings);
    textual::Ty::Ptr(Box::new(textual::Ty::Type(cname)))
}

/// For a given class return the Ty for its non-static type.
fn get_static_name(class: ir::ClassId, strings: &ir::StringInterner) -> String {
    let cname = class.mangle(strings);
    format!("get_static::{cname}")
}

/// The name of the global singleton for a static class.
pub(crate) fn static_singleton_name(state: &UnitState, class: &ir::Class<'_>) -> String {
    let cname = class.name.mangle(&state.strings);
    format!("static_singleton::{cname}")
}

/// Returns an iterator to help dynamic allocation of BlockIds.
fn bid_allocator() -> impl Iterator<Item = BlockId> {
    std::iter::successors(Some(BlockId::from_usize(1)), |n| {
        Some(BlockId::from_usize(n.as_usize() + 1))
    })
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

/// Build the `get_static` function for a static class.
///
/// Declares globals for:
///   - static singleton
///   - static vtable
///   - non-static vtable
/// Writes the `get_static` function itself which initializes the globals and
/// returns the memoized static singleton.
fn write_get_static(
    w: &mut dyn std::io::Write,
    state: &mut UnitState,
    class: &ir::Class<'_>,
) -> Result {
    textual::declare_global(
        w,
        &static_singleton_name(state, class),
        textual::Ty::Ptr(Box::new(static_ty(class.name, &state.strings))),
    )?;

    textual::write_function(
        w,
        &state.strings,
        &get_static_name(class.name, &state.strings),
        &class.src_loc,
        &[],
        static_ty(class.name, &state.strings),
        |w| {
            let mut bid_allocator = bid_allocator();

            // Load the class singleton to see if it's already been initialized.
            let singleton_name = static_singleton_name(state, class);
            let singleton_expr = textual::Expr::deref(textual::Var::named(singleton_name));
            let singleton = w.load(static_ty(class.name, &state.strings), singleton_expr)?;

            let bid_uninitialized = bid_allocator.next().unwrap();
            let bid_initialized = bid_allocator.next().unwrap();
            let uninitialized = hack::call_builtin(w, hack::Builtin::RawPtrIsNull, [singleton])?;
            w.jmp(&[bid_initialized, bid_uninitialized], ())?;

            w.write_label(bid_initialized, &[])?;
            w.prune_not(uninitialized)?;
            w.ret(singleton)?;

            w.write_label(bid_uninitialized, &[])?;
            w.prune(uninitialized)?;

            let singleton = get_static_init_static_singleton(w, state, class)?;

            w.ret(singleton)?;
            Ok(())
        },
    )
}

/// Helper for `write_get_static` - initializes the static class singleton.
fn get_static_init_static_singleton(
    w: &mut textual::FuncWriter<'_>,
    state: &UnitState,
    class: &ir::Class<'_>,
) -> Result<textual::Sid> {
    let sz = class.properties.len();

    let p = hack::call_builtin(w, hack::Builtin::AllocWords, [sz as i64])?;

    let singleton_name = static_singleton_name(state, class);
    let singleton_expr = textual::Expr::deref(textual::Var::named(singleton_name));
    w.store(singleton_expr, p, static_ty(class.name, &state.strings))?;

    Ok(p)
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

    func::write_func(
        w,
        state,
        &method.name.mangle(class.name, &state.strings),
        &method.func,
    )
}

/// Loads the static singleton for a class.
pub(crate) fn load_static_class(
    w: &mut textual::FuncWriter<'_>,
    class: ir::ClassId,
    strings: &ir::StringInterner,
) -> Result<textual::Sid> {
    let name = get_static_name(class, strings);
    w.call(&name, ())
}
