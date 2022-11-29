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

use std::sync::Arc;

use anyhow::Error;
use log::trace;

use super::func;
use super::hack;
use super::textual;
use crate::func::MethodInfo;
use crate::mangle::Mangle;
use crate::mangle::MangleWithClass as _;
use crate::state;
use crate::state::UnitState;
use crate::textual::TextualFile;
use crate::types::convert_ty;

type Result<T = (), E = Error> = std::result::Result<T, E>;

#[derive(Copy, Clone, Eq, PartialEq)]
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
    txf: &mut TextualFile<'_>,
    state: &mut UnitState,
    class: ir::Class<'_>,
) -> Result {
    trace!("Convert Class {}", class.name.as_bstr(&state.strings));

    let mut class = crate::lower::lower_class(class, Arc::clone(&state.strings));

    write_type(txf, state, &class, IsStatic::Static)?;
    write_type(txf, state, &class, IsStatic::NonStatic)?;

    write_init_static(txf, state, &class)?;

    let methods = std::mem::take(&mut class.methods);
    for method in methods {
        write_method(txf, state, &class, method)?;
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
fn static_singleton_name(class: ir::ClassId, strings: &ir::StringInterner) -> String {
    let cname = class.mangle(strings);
    format!("static_singleton::{cname}")
}

/// Write the type for a (class, is_static) with the properties of the class.
fn write_type(
    txf: &mut TextualFile<'_>,
    state: &mut UnitState,
    class: &ir::Class<'_>,
    is_static: IsStatic,
) -> Result {
    let mut fields: Vec<(String, textual::Ty, textual::Visibility)> = Vec::new();
    for prop in &class.properties {
        let ir::Property {
            name,
            mut flags,
            ref attributes,
            visibility,
            ref initial_value,
            ref type_info,
            doc_comment: _,
        } = *prop;

        let instance_match = match is_static {
            IsStatic::Static => flags.is_static(),
            IsStatic::NonStatic => !flags.is_static(),
        };
        if !instance_match {
            continue;
        }
        flags.clear(ir::Attr::AttrStatic);

        let name = name.mangle(&state.strings);

        let vis = if flags.is_private() {
            textual::Visibility::Private
        } else if flags.is_protected() {
            textual::Visibility::Protected
        } else {
            textual::Visibility::Public
        };
        flags.clear(ir::Attr::AttrPrivate);
        flags.clear(ir::Attr::AttrProtected);
        flags.clear(ir::Attr::AttrPublic);
        flags.clear(ir::Attr::AttrSystemInitialValue);

        if !flags.is_empty() {
            trace!("CLASS FLAGS: {:?}", flags);
            textual_todo! { txf.write_comment(&format!("TODO: class flags: {flags:?}"))? };
        }
        if !attributes.is_empty() {
            textual_todo! { txf.write_comment(&format!("TODO: class attributes: {attributes:?}"))? };
        }
        if visibility == ir::Visibility::Private {
            txf.write_comment(&format!("TODO: private {name}"))?;
        }
        if let Some(initial_value) = initial_value {
            txf.write_comment(&format!("TODO: initial value {initial_value:?}"))?;
        }

        let ty = convert_ty(type_info.enforced.clone(), &state.strings);

        fields.push((name, ty, vis));
    }

    let fields = fields.iter().map(|(name, ty, visibility)| textual::Field {
        name: name.as_str(),
        ty,
        visibility: *visibility,
    });

    let cname = mangled_class_name(class.name, is_static, &state.strings);
    txf.define_type(&cname, &class.src_loc, fields)?;
    Ok(())
}

/// Build the `init_static` function for a static class.
///
/// Declares globals for:
///   - static singleton
/// Writes the `init_static` function itself which initializes the globals and
/// returns the memoized static singleton.
fn write_init_static(
    txf: &mut TextualFile<'_>,
    state: &mut UnitState,
    class: &ir::Class<'_>,
) -> Result {
    let singleton_name = static_singleton_name(class.name, &state.strings);
    state
        .decls
        .declare_global(&singleton_name, static_ty(class.name, &state.strings));

    txf.define_function(
        &init_static_name(class.name, &state.strings),
        &class.src_loc,
        &[],
        tx_ty!(void),
        &[],
        |w| {
            let sz = 0; // TODO: properties
            let p = hack::call_builtin(w, hack::Builtin::AllocWords, [sz])?;

            let singleton_expr = textual::Expr::deref(textual::Var::named(singleton_name));
            w.store(singleton_expr, p, static_ty(class.name, &state.strings))?;

            w.ret(0)?;
            Ok(())
        },
    )
}

fn write_method(
    txf: &mut TextualFile<'_>,
    state: &mut UnitState,
    class: &ir::Class<'_>,
    method: ir::Method<'_>,
) -> Result {
    trace!(
        "Convert Method {}::{}",
        class.name.as_bstr(&state.strings),
        method.name.as_bstr(&state.strings)
    );

    let this_ty = if method.attrs.is_static() {
        static_ty(class.name, &state.strings)
    } else {
        non_static_ty(class.name, &state.strings)
    };

    let method_info = MethodInfo {
        class,
        is_static: method.attrs.is_static(),
    };

    func::write_func(
        txf,
        state,
        &method.name.mangle(class.name, &state.strings),
        this_ty,
        method.func,
        Some(&method_info),
    )
}

/// Loads the static singleton for a class.
pub(crate) fn load_static_class(
    w: &mut textual::FuncWriter<'_, '_>,
    class: ir::ClassId,
    strings: &ir::StringInterner,
    decls: &mut state::Decls,
) -> Result<textual::Sid> {
    // Blindly load the static singleton, assuming it's already been initialized.
    let singleton_name = static_singleton_name(class, strings);
    decls.declare_global(&singleton_name, static_ty(class, strings));
    let singleton_expr = textual::Expr::deref(textual::Var::named(singleton_name));
    let value = w.load(static_ty(class, strings), singleton_expr)?;
    hack::call_builtin(w, hack::Builtin::SilLazyInitialize, [value])?;
    Ok(value)
}
