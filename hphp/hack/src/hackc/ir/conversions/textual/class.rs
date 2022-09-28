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
use itertools::Itertools;
use log::trace;
use num_derive::ToPrimitive;
use num_traits::ToPrimitive as _;
use strum::EnumCount as _;
use strum::IntoEnumIterator as _;
use strum_macros::EnumCount;
use strum_macros::EnumIter;

use super::func;
use super::hack;
use super::textual;
use super::textual::Sid;
use crate::mangle::MangleClassId;
use crate::mangle::MangleId;
use crate::state::UnitState;

type Result<T = (), E = Error> = std::result::Result<T, E>;

const VTABLE_FIELD: &str = "vtable";

#[derive(Copy, Clone, Eq, PartialEq, EnumCount, EnumIter, ToPrimitive)]
enum VTableIndex {
    Invoke,
    GetProp,
    SetProp,
}

#[derive(Copy, Clone, Eq, PartialEq, EnumIter)]
pub(crate) enum IsStatic {
    Static,
    NonStatic,
}

impl IsStatic {
    fn matches_method(&self, method: &ir::Method<'_>) -> bool {
        match self {
            IsStatic::Static => method.attrs.is_static(),
            IsStatic::NonStatic => !method.attrs.is_static(),
        }
    }
}

pub(crate) struct StaticClassId(pub(crate) ir::ClassId);

impl MangleId for StaticClassId {
    fn mangle(&self, strings: &ir::StringInterner) -> String {
        format!("static::{}", self.0.mangle(strings))
    }
}

/// Classes are defined as:
///
/// type NAME = [ vtable; properties*; ]
///
/// vtable:
///   invoke::NAME
///   get_prop::NAME
///   set_prop::NAME
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

    for index in VTableIndex::iter() {
        for prop in IsStatic::iter() {
            match index {
                VTableIndex::Invoke => write_invoke(w, state, &class, prop)?,
                VTableIndex::GetProp => write_get_prop(w, state, &class, prop)?,
                VTableIndex::SetProp => write_set_prop(w, state, &class, prop)?,
            }
        }
    }

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
    textual::Ty::RawPtr(Box::new(textual::Ty::RawType(cname)))
}

/// For a given class return the Ty for its non-static type.
fn get_static_name(class: ir::ClassId, strings: &ir::StringInterner) -> String {
    let cname = class.mangle(strings);
    format!("get_static::{cname}")
}

/// The name of the invoke function for a (class, is_static).
fn invoke_name(state: &UnitState, class: &ir::Class<'_>, is_static: IsStatic) -> String {
    let cname = class.name.mangle(&state.strings);
    match is_static {
        IsStatic::Static => format!("invoke::static::{cname}"),
        IsStatic::NonStatic => format!("invoke::{cname}"),
    }
}

/// The name of the get_prop function for a (class, is_static).
fn get_prop_name(state: &UnitState, class: &ir::Class<'_>, is_static: IsStatic) -> String {
    let cname = class.name.mangle(&state.strings);
    match is_static {
        IsStatic::Static => format!("get_prop::static::{cname}"),
        IsStatic::NonStatic => format!("get_prop::{cname}"),
    }
}

/// The name of the set_prop function for a (class, is_static).
fn set_prop_name(state: &UnitState, class: &ir::Class<'_>, is_static: IsStatic) -> String {
    let cname = class.name.mangle(&state.strings);
    match is_static {
        IsStatic::Static => format!("set_prop::static::{cname}"),
        IsStatic::NonStatic => format!("set_prop::{cname}"),
    }
}

/// The name of the global singleton for a static class.
pub(crate) fn static_singleton_name(state: &UnitState, class: &ir::Class<'_>) -> String {
    let cname = class.name.mangle(&state.strings);
    format!("static_singleton::{cname}")
}

/// The name of the global vtable for a (class, is_static).
fn vtable_name(state: &UnitState, class: &ir::Class<'_>, is_static: IsStatic) -> String {
    let cname = class.name.mangle(&state.strings);
    match is_static {
        IsStatic::Static => format!("vtable::static::{cname}"),
        IsStatic::NonStatic => format!("vtable::{cname}"),
    }
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
    let mut fields = Vec::new();
    // Add our VTable
    fields.push((VTABLE_FIELD, tx_ty!(**void)));
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
        &vtable_name(state, class, IsStatic::Static),
        tx_ty!(**void),
    )?;
    textual::declare_global(
        w,
        &vtable_name(state, class, IsStatic::NonStatic),
        tx_ty!(**void),
    )?;
    textual::declare_global(
        w,
        &static_singleton_name(state, class),
        textual::Ty::RawPtr(Box::new(static_ty(class.name, &state.strings))),
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

            let vtable = get_static_init_vtable(w, state, class, IsStatic::Static)?;
            get_static_init_vtable(w, state, class, IsStatic::NonStatic)?;
            let singleton = get_static_init_static_singleton(w, state, class, vtable)?;

            w.ret(singleton)?;
            Ok(())
        },
    )
}

/// Helper for `write_get_static` - writes the vtable for a (class, is_static).
fn get_static_init_vtable(
    w: &mut textual::FuncWriter<'_>,
    state: &UnitState,
    class: &ir::Class<'_>,
    is_static: IsStatic,
) -> Result<Sid> {
    let vtable_name = vtable_name(state, class, is_static);
    let vtable_global = textual::Expr::deref(textual::Var::named(vtable_name));

    let vtable = hack::call_builtin(w, hack::Builtin::AllocWords, [VTableIndex::COUNT as i64])?;
    w.store(vtable_global, vtable, tx_ty!(**void))?;

    for idx in VTableIndex::iter() {
        let name = match idx {
            VTableIndex::Invoke => invoke_name(state, class, is_static),
            VTableIndex::GetProp => get_prop_name(state, class, is_static),
            VTableIndex::SetProp => set_prop_name(state, class, is_static),
        };
        let fnptr = textual::Expr::Deref(textual::Var::named(name));
        w.store(
            textual::Expr::index(vtable, idx.to_i64().unwrap()),
            fnptr,
            tx_ty!(*void),
        )?;
    }

    Ok(vtable)
}

/// Helper for `write_get_static` - initializes the static class singleton.
fn get_static_init_static_singleton(
    w: &mut textual::FuncWriter<'_>,
    state: &UnitState,
    class: &ir::Class<'_>,
    vtable: Sid,
) -> Result<textual::Sid> {
    let sz = /* vtable */ 1  + class.properties.len();

    let p = hack::call_builtin(w, hack::Builtin::AllocWords, [sz as i64])?;
    w.store(textual::Expr::index(p, 0), vtable, tx_ty!(*void))?;

    let singleton_name = static_singleton_name(state, class);
    let singleton_expr = textual::Expr::deref(textual::Var::named(singleton_name));
    w.store(singleton_expr, p, static_ty(class.name, &state.strings))?;

    Ok(p)
}

/// Builds the `invoke` function for a (class, is_static).
fn write_invoke(
    w: &mut dyn std::io::Write,
    state: &mut UnitState,
    class: &ir::Class<'_>,
    is_static: IsStatic,
) -> Result {
    // function invoke(name: mixed, params: HackParams) -> mixed {
    //   if (name == "method1") {
    //     return method1(params);
    //   } else if (name == "method2") {
    //     return method2(params);
    //   } else {
    //     throw new BadMethod();
    //   }
    // }
    textual::write_function(
        w,
        &state.strings,
        &invoke_name(state, class, is_static),
        &class.src_loc,
        &[("name", tx_ty!(mixed)), ("params", tx_ty!(*HackParams))],
        tx_ty!(mixed),
        |w| {
            let name = w.load(
                tx_ty!(mixed),
                textual::Expr::deref(textual::Var::named("name")),
            )?;

            let params = w.load(
                tx_ty!(*HackParams),
                textual::Expr::deref(textual::Var::named("params")),
            )?;

            let mut bid_allocator = bid_allocator();

            let targets = class
                .methods
                .iter()
                .filter_map(|method| {
                    if is_static.matches_method(method) {
                        let bid = bid_allocator.next().unwrap();
                        Some((bid, method))
                    } else {
                        None
                    }
                })
                .collect_vec();

            let default = bid_allocator.next().unwrap();

            w.jmp(
                &targets
                    .iter()
                    .map(|(bid, _)| *bid)
                    .chain(std::iter::once(default))
                    .collect_vec(),
                (),
            )?;

            let mut cmps = Vec::new();

            for (bid, method) in targets {
                w.write_label(bid, &[])?;
                let mname = method.name.as_bytes();
                let pred = hack::expr_builtin(
                    hack::Builtin::Hhbc(hack::Hhbc::CmpEq),
                    (name, textual::Expr::hack_string(mname)),
                );
                w.prune(pred.clone())?;
                cmps.push(pred);
                let target = method.name.mangle(class.name, &state.strings);
                let res = w.call(&target, [params])?;
                w.ret(res)?;
            }

            // default case
            w.write_label(default, &[])?;
            for cmp in cmps {
                w.prune_not(cmp)?;
            }

            if let Some(_base) = class.base {
                todo!();
            } else {
                hack::call_builtin(w, hack::Builtin::BadMethodCall, ())?;
                w.unreachable()?;
            }

            Ok(())
        },
    )
}

/// Builds the `get_prop` function for a (class, is_static).
fn write_get_prop(
    w: &mut dyn std::io::Write,
    state: &mut UnitState,
    class: &ir::Class<'_>,
    is_static: IsStatic,
) -> Result {
    // function get_prop(name: mixed) -> mixed {
    //   if (name == "prop1") {
    //     return this->prop1;
    //   } else if (name == "prop2") {
    //     return this->prop2;
    //   } else {
    //     throw new BadProp();
    //   }
    // }
    textual::write_function(
        w,
        &state.strings,
        &get_prop_name(state, class, is_static),
        &class.src_loc,
        &[("name", tx_ty!(mixed))],
        tx_ty!(mixed),
        |w| {
            let _name = w.load(
                tx_ty!(mixed),
                textual::Expr::deref(textual::Var::named("name")),
            )?;

            if !class.properties.is_empty() {
                todo!();
            }

            // default case
            hack::call_builtin(w, hack::Builtin::BadProperty, ())?;
            w.unreachable()?;
            Ok(())
        },
    )
}

/// Builds the `set_prop` function for a (class, is_static).
fn write_set_prop(
    w: &mut dyn std::io::Write,
    state: &mut UnitState,
    class: &ir::Class<'_>,
    is_static: IsStatic,
) -> Result {
    // function set_prop(name: mixed, value: mixed) {
    //   if (name == "prop1") {
    //     this->prop1 = value;
    //   } else if (name == "prop2") {
    //     this->prop2 = value;
    //   } else {
    //     throw new BadProp();
    //   }
    // }
    textual::write_function(
        w,
        &state.strings,
        &set_prop_name(state, class, is_static),
        &class.src_loc,
        &[("name", tx_ty!(mixed)), ("value", tx_ty!(mixed))],
        tx_ty!(void),
        |w| {
            let _name = w.load(
                tx_ty!(mixed),
                textual::Expr::deref(textual::Var::named("name")),
            )?;

            let _value = w.load(
                tx_ty!(mixed),
                textual::Expr::deref(textual::Var::named("value")),
            )?;

            if !class.properties.is_empty() {
                todo!();
            }

            // default case
            hack::call_builtin(w, hack::Builtin::BadProperty, ())?;
            w.unreachable()?;
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
