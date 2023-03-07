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
use hash::IndexMap;
use ir::StringInterner;
use itertools::Itertools;
use log::trace;

use super::func;
use super::hack;
use super::textual;
use crate::func::FuncInfo;
use crate::func::MethodInfo;
use crate::lower;
use crate::mangle::Mangle;
use crate::mangle::MangleClass as _;
use crate::mangle::MangleWithClass;
use crate::state::UnitState;
use crate::textual::FieldAttribute;
use crate::textual::TextualFile;
use crate::typed_value;
use crate::types::convert_ty;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// Classes are defined as:
///
/// type NAME = [ properties*; ]
///
pub(crate) fn write_class(
    txf: &mut TextualFile<'_>,
    unit_state: &mut UnitState,
    class: ir::Class<'_>,
) -> Result {
    trace!("Convert Class {}", class.name.as_bstr(&unit_state.strings));

    let class = crate::lower::lower_class(class, Arc::clone(&unit_state.strings));

    let mut state = ClassState::new(txf, unit_state, class);
    state.write_class()
}

#[derive(Copy, Clone, Eq, PartialEq)]
pub(crate) enum IsStatic {
    Static,
    NonStatic,
}

impl IsStatic {
    pub(crate) fn as_bool(&self) -> bool {
        matches!(self, IsStatic::Static)
    }
}

struct ClassState<'a, 'b, 'c> {
    class: ir::Class<'c>,
    needs_factory: bool,
    txf: &'a mut TextualFile<'b>,
    unit_state: &'a mut UnitState,
}

impl<'a, 'b, 'c> ClassState<'a, 'b, 'c> {
    fn new(
        txf: &'a mut TextualFile<'b>,
        unit_state: &'a mut UnitState,
        class: ir::Class<'c>,
    ) -> Self {
        let needs_factory = !class.flags.is_interface()
            && !class.flags.is_trait()
            && !class.flags.is_enum()
            && !class.flags.contains(ir::Attr::AttrIsClosureClass);
        ClassState {
            class,
            needs_factory,
            txf,
            unit_state,
        }
    }
}

impl ClassState<'_, '_, '_> {
    fn write_class(&mut self) -> Result {
        self.write_type(IsStatic::Static)?;
        self.write_type(IsStatic::NonStatic)?;

        // Class constants are not inherited so we just turn them into globals.
        for constant in &self.class.constants {
            let name = constant
                .name
                .mangle_with_class_state(self, IsStatic::Static);
            let ty = if let Some(et) = self.class.enum_type.as_ref() {
                convert_enum_ty(et, self.strings())
            } else {
                textual::Ty::mixed()
            };
            self.txf.define_global(name, ty);
        }

        self.write_init_static()?;

        let methods = std::mem::take(&mut self.class.methods);
        for method in methods {
            self.write_method(method)?;
        }

        if self.needs_factory {
            // TODO: This should be the parameters from the constructor.
            self.write_factory(std::iter::empty())?;
        }

        Ok(())
    }

    fn strings(&self) -> &StringInterner {
        &self.unit_state.strings
    }

    /// Write the type for a (class, is_static) with the properties of the class.
    fn write_type(&mut self, is_static: IsStatic) -> Result {
        let mut metadata: IndexMap<&str, textual::Expr> = IndexMap::default();

        let kind = if self.class.flags.is_interface() {
            "interface"
        } else if self.class.flags.is_trait() {
            "trait"
        } else {
            "class"
        };
        metadata.insert("kind", kind.into());
        metadata.insert("static", is_static.as_bool().into());

        // Traits say they're final - because they're not "real" classes. But that
        // will be strange for us since we treat them as bases.
        if !self.class.flags.is_trait() && !self.class.flags.is_interface() {
            metadata.insert("final", self.class.flags.is_final().into());
        }

        let mut fields: Vec<textual::Field<'_>> = Vec::new();
        let properties = std::mem::take(&mut self.class.properties);
        for prop in &properties {
            if prop.flags.is_static() == is_static.as_bool() {
                self.write_property(&mut fields, prop)?;
            }
        }
        self.class.properties = properties;

        let mut extends: Vec<ir::ClassId> = Vec::new();
        if let Some(base) = compute_base(&self.class) {
            extends.push(base);
        }

        extends.extend(self.class.implements.iter());
        extends.extend(self.class.uses.iter());

        let extends = extends
            .into_iter()
            .map(|id| id.mangle_class(is_static, self.strings()))
            .collect_vec();

        let cname = self.class.name.mangle_class(is_static, self.strings());
        self.txf.define_type(
            &cname,
            &self.class.src_loc,
            extends.iter().map(String::as_str),
            fields.into_iter(),
            metadata.iter().map(|(k, v)| (*k, v)),
        )?;

        Ok(())
    }

    fn write_property(
        &mut self,
        fields: &mut Vec<textual::Field<'_>>,
        prop: &ir::Property<'_>,
    ) -> Result {
        let ir::Property {
            name,
            mut flags,
            ref attributes,
            visibility: ir_visibility,
            ref initial_value,
            ref type_info,
            doc_comment: _,
        } = *prop;

        flags.clear(ir::Attr::AttrStatic);

        let name = name.mangle(&self.unit_state.strings);

        let visibility = if flags.is_private() {
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

        let mut tx_attributes = Vec::new();
        let comments = Vec::new();

        if !flags.is_empty() {
            trace!("CLASS FLAGS: {:?}", flags);
            textual_todo! { self.txf.write_comment(&format!("TODO: class flags: {flags:?}"))? };
        }

        for attribute in attributes {
            // We don't do anything with class attributes. They don't affect
            // normal program flow - but can be seen by reflection so it's
            // questionable if we need them for analysis.
            let name = attribute
                .name
                .mangle_class(IsStatic::NonStatic, &self.unit_state.strings);
            if attribute.arguments.is_empty() {
                tx_attributes.push(FieldAttribute::Unparameterized { name });
            } else {
                let mut parameters = Vec::new();
                for arg in &attribute.arguments {
                    textual_todo! {
                        parameters.push(format!("TODO: {arg:?}"));
                    }
                }
                tx_attributes.push(FieldAttribute::Parameterized { name, parameters });
            }
        }

        if ir_visibility == ir::Visibility::Private {
            self.txf.write_comment(&format!("TODO: private {name}"))?;
        }
        if let Some(initial_value) = initial_value {
            self.txf
                .write_comment(&format!("TODO: initial value {initial_value:?}"))?;
        }

        let ty = convert_ty(&type_info.enforced, &self.unit_state.strings);

        fields.push(textual::Field {
            name: name.into(),
            ty: ty.into(),
            visibility,
            attributes: tx_attributes,
            comments,
        });
        Ok(())
    }

    /// Build the `init_static` function for a static class.
    ///
    /// Declares globals for:
    ///   - static singleton
    /// Writes the `init_static` function itself which initializes the globals and
    /// returns the memoized static singleton.
    fn write_init_static(&mut self) -> Result {
        let singleton_name = static_singleton_name(self.class.name, &self.unit_state.strings);
        self.txf.define_global(
            singleton_name.clone(),
            static_ty(self.class.name, &self.unit_state.strings),
        );

        self.txf.define_function(
            &init_static_name(self.class.name, &self.unit_state.strings),
            &self.class.src_loc,
            &[],
            &textual::Ty::Void,
            &[],
            |fb| {
                let sz = 0; // TODO: properties
                let p = hack::call_builtin(fb, hack::Builtin::AllocWords, [sz])?;

                let singleton_expr = textual::Expr::deref(textual::Var::global(singleton_name));
                fb.store(
                    singleton_expr,
                    p,
                    &static_ty(self.class.name, &self.unit_state.strings),
                )?;

                // constants
                for constant in &self.class.constants {
                    if let Some(value) = constant.value.as_ref() {
                        let name = constant.name.mangle_with_class(
                            self.class.name,
                            IsStatic::Static,
                            &self.unit_state.strings,
                        );
                        let var = textual::Var::global(name);
                        let value = typed_value::typed_value_expr(value, &self.unit_state.strings);
                        fb.store(textual::Expr::deref(var), value, &textual::Ty::mixed())?;
                    }
                }

                fb.ret(0)?;
                Ok(())
            },
        )
    }

    /// Build the factory for a class.
    fn write_factory<'s>(
        &mut self,
        params: impl Iterator<Item = (&'s str, textual::Ty)>,
    ) -> Result {
        let strings = &self.unit_state.strings;
        let name = ir::MethodId::factory(strings).mangle_with_class(
            self.class.name,
            IsStatic::Static,
            strings,
        );
        let static_ty = static_ty(self.class.name, strings);
        let ty = non_static_ty(self.class.name, strings);
        let cons_id = ir::MethodId::constructor(strings);

        let params = std::iter::once(("$this", static_ty))
            .chain(params)
            .collect_vec();
        let params = params.iter().map(|(s, ty)| (*s, ty)).collect_vec();

        self.txf
            .define_function(&name, &self.class.src_loc, &params, &ty, &[], |fb| {
                let mut operands = Vec::new();
                for (name, ty) in &params[1..] {
                    let id = strings.intern_str(*name);
                    let lid = textual::Var::Local(ir::LocalId::Named(id));
                    operands.push(fb.load(ty, textual::Expr::deref(lid))?);
                }

                let cons = cons_id.mangle_with_class(self.class.name, IsStatic::NonStatic, strings);
                let obj = fb.write_expr_stmt(textual::Expr::Alloc(ty.deref()))?;
                fb.call_static(&cons, obj.into(), operands)?;
                fb.ret(obj)?;
                Ok(())
            })
    }

    fn write_method(&mut self, method: ir::Method<'_>) -> Result {
        trace!(
            "Convert Method {}::{}",
            self.class.name.as_bstr(&self.unit_state.strings),
            method.name.as_bstr(&self.unit_state.strings)
        );

        let is_static = match method.attrs.is_static() {
            true => IsStatic::Static,
            false => IsStatic::NonStatic,
        };

        let this_ty = class_ty(self.class.name, is_static, &self.unit_state.strings);
        let func = {
            let method_info = MethodInfo {
                name: method.name,
                class: &self.class,
                is_static,
                strings: Arc::clone(&self.unit_state.strings),
                flags: method.flags,
            };
            let func = lower::lower_func(
                method.func,
                Arc::new(FuncInfo::Method(method_info)),
                Arc::clone(&self.unit_state.strings),
            );
            ir::verify::verify_func(&func, &Default::default(), &self.unit_state.strings);
            func
        };

        if self.needs_factory && method.name.is_constructor(&self.unit_state.strings) {
            self.needs_factory = false;

            let (param_names, param_tys, _) =
                func::compute_func_params(&func.params, self.unit_state, this_ty.clone())?;
            self.write_factory(
                param_names
                    .iter()
                    .map(|s| s.as_str())
                    .zip(param_tys.iter().cloned())
                    .skip(1),
            )?;
        }

        let method_info = MethodInfo {
            name: method.name,
            class: &self.class,
            is_static,
            strings: Arc::clone(&self.unit_state.strings),
            flags: method.flags,
        };
        func::write_func(
            self.txf,
            self.unit_state,
            this_ty,
            func,
            Arc::new(FuncInfo::Method(method_info)),
        )?;

        Ok(())
    }
}

trait MangleWithClassState {
    fn mangle_with_class_state(
        &self,
        state: &ClassState<'_, '_, '_>,
        is_static: IsStatic,
    ) -> String;
}

impl<T: MangleWithClass> MangleWithClassState for T {
    fn mangle_with_class_state(
        &self,
        state: &ClassState<'_, '_, '_>,
        is_static: IsStatic,
    ) -> String {
        self.mangle_with_class(state.class.name, is_static, &state.unit_state.strings)
    }
}

fn convert_enum_ty(ti: &ir::TypeInfo, strings: &StringInterner) -> textual::Ty {
    // Enum types are unenforced - and yet the constants ARE real type. So scan
    // the text of the type and do the best we can.
    //
    // If we recognize the type then use it.  Unless it's an "enum class" it can
    // only be an arraykey.
    //
    if ti
        .user_type
        .map_or(false, |id| strings.eq_str(id, "HH\\int"))
    {
        return textual::Ty::SpecialPtr(textual::SpecialTy::Int);
    }
    if ti
        .user_type
        .map_or(false, |id| strings.eq_str(id, "HH\\string"))
    {
        return textual::Ty::SpecialPtr(textual::SpecialTy::String);
    }

    // But it can be an alias - so we might just not recognize it - so default
    // to mixed.
    textual::Ty::mixed()
}

/// For a given class return the Ty for its non-static (instance) type.
pub(crate) fn non_static_ty(class: ir::ClassId, strings: &StringInterner) -> textual::Ty {
    let cname = class.mangle_class(IsStatic::NonStatic, strings);
    textual::Ty::Ptr(Box::new(textual::Ty::Type(cname)))
}

/// For a given class return the Ty for its static type.
pub(crate) fn static_ty(class: ir::ClassId, strings: &StringInterner) -> textual::Ty {
    let cname = class.mangle_class(IsStatic::Static, strings);
    textual::Ty::Ptr(Box::new(textual::Ty::Type(cname)))
}

pub(crate) fn class_ty(
    class: ir::ClassId,
    is_static: IsStatic,
    strings: &StringInterner,
) -> textual::Ty {
    match is_static {
        IsStatic::Static => static_ty(class, strings),
        IsStatic::NonStatic => non_static_ty(class, strings),
    }
}

/// For a given class return the Ty for its non-static type.
fn init_static_name(class: ir::ClassId, strings: &StringInterner) -> String {
    let method = ir::MethodName::new(ffi::Slice::new(b"$init_static"));
    method.mangle_with_class(class, IsStatic::Static, strings)
}

/// The name of the global singleton for a static class.
fn static_singleton_name(class: ir::ClassId, strings: &StringInterner) -> String {
    let name = ir::ConstId::new(strings.intern_str("static_singleton"));
    name.mangle_with_class(class, IsStatic::Static, strings)
}

fn compute_base(class: &ir::Class<'_>) -> Option<ir::ClassId> {
    if class.flags.is_trait() {
        // Traits express bases through a 'require extends'.
        let req = class
            .requirements
            .iter()
            .find(|r| r.kind == ir::TraitReqKind::MustExtend);
        req.map(|req| req.name)
    } else {
        class.base
    }
}

/// Loads the static singleton for a class.
pub(crate) fn load_static_class(
    fb: &mut textual::FuncBuilder<'_, '_>,
    class: ir::ClassId,
    strings: &StringInterner,
) -> Result<textual::Sid> {
    // Blindly load the static singleton, assuming it's already been initialized.
    let singleton_name = static_singleton_name(class, strings);
    fb.txf
        .define_global(singleton_name.clone(), static_ty(class, strings));
    let singleton_expr = textual::Expr::deref(textual::Var::global(singleton_name));
    let value = fb.load(&static_ty(class, strings), singleton_expr)?;
    hack::call_builtin(fb, hack::Builtin::SilLazyInitialize, [value])?;
    Ok(value)
}
