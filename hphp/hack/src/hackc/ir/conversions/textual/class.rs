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

use std::cmp::Ordering;
use std::sync::Arc;

use anyhow::Error;
use hash::IndexMap;
use ir::LocalId;
use itertools::Itertools;
use log::trace;
use naming_special_names_rust::special_idents;

use super::func;
use super::textual;
use crate::func::FuncInfo;
use crate::func::MethodInfo;
use crate::mangle::FieldName;
use crate::mangle::FunctionName;
use crate::mangle::Intrinsic;
use crate::mangle::TypeName;
use crate::mangle::VarName;
use crate::state::UnitState;
use crate::textual::FieldAttribute;
use crate::textual::TextualFile;
use crate::types::convert_ty;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// Classes are defined as:
///
/// type NAME = [ properties*; ]
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

    pub(crate) fn as_attr(self) -> ir::Attr {
        match self {
            IsStatic::NonStatic => ir::Attr::AttrNone,
            IsStatic::Static => ir::Attr::AttrStatic,
        }
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

        // Note: Class constants and type constants turn into static properties
        // during lowering.

        if self.needs_factory {
            self.write_factory()?;
        }

        let mut methods = std::mem::take(&mut self.class.methods);
        methods.sort_by(|a, b| cmp_method(a, b, &self.unit_state.strings));
        for method in methods {
            self.write_method(method)?;
        }

        Ok(())
    }

    /// Write the type for a (class, is_static) with the properties of the class.
    fn write_type(&mut self, is_static: IsStatic) -> Result {
        let mut metadata: IndexMap<&str, textual::Const> = IndexMap::default();

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
            .map(|id| TypeName::class(id, is_static))
            .collect_vec();

        let cname = TypeName::class(self.class.name, is_static);
        self.txf.define_type(
            &cname,
            Some(&self.class.src_loc),
            extends.iter(),
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
            visibility: _,
            initial_value: _,
            ref type_info,
            doc_comment: _,
        } = *prop;

        flags.clear(ir::Attr::AttrStatic);

        let name = FieldName::Prop(name);

        let visibility = if flags.is_private() {
            textual::Visibility::Private
        } else if flags.is_protected() {
            textual::Visibility::Protected
        } else {
            textual::Visibility::Public
        };
        flags.clear(ir::Attr::AttrForbidDynamicProps);
        flags.clear(ir::Attr::AttrPrivate);
        flags.clear(ir::Attr::AttrProtected);
        flags.clear(ir::Attr::AttrPublic);
        flags.clear(ir::Attr::AttrSystemInitialValue);
        flags.clear(ir::Attr::AttrInterceptable);

        let mut tx_attributes = Vec::new();
        let comments = Vec::new();

        if !flags.is_empty() {
            textual_todo! {
                message = ("CLASS FLAGS: {:?}", flags),
                self.txf.write_comment(&format!("TODO: class flags: {flags:?}"))?
            };
        }

        for attribute in attributes {
            // We don't do anything with class attributes. They don't affect
            // normal program flow - but can be seen by reflection so it's
            // questionable if we need them for analysis.
            let name = TypeName::Class(attribute.name);
            if attribute.arguments.is_empty() {
                tx_attributes.push(FieldAttribute::Unparameterized { name });
            } else {
                let mut parameters = Vec::new();
                for arg in &attribute.arguments {
                    match arg.get_string() {
                        Some(sid) => {
                            parameters.push(self.unit_state.strings.lookup_bstr(sid).to_string())
                        }
                        _ => {
                            textual_todo! {
                                parameters.push(format!("TODO: {arg:?}"));
                            }
                        }
                    }
                }
                tx_attributes.push(FieldAttribute::Parameterized { name, parameters });
            }
        }

        let ty = convert_ty(&type_info.enforced, &self.unit_state.strings);

        fields.push(textual::Field {
            name,
            ty: ty.into(),
            visibility,
            attributes: tx_attributes,
            comments,
        });
        Ok(())
    }

    /// Build the factory for a class.
    ///
    /// The factory only allocates an object of the required type. The initialization is done via a separate constructor invocation on the allocated object.
    ///
    /// The factory method is used only when the class of an object to allocate is not known statically. Otherwise, we directly use Textual's typed allocation builtin.
    fn write_factory(&mut self) -> Result {
        let name = FunctionName::Intrinsic(Intrinsic::Factory(self.class.name));
        let static_ty = static_ty(self.class.name);
        let ty = textual::Return::new(non_static_ty(self.class.name));

        let this_lid = LocalId::Named(self.unit_state.strings.intern_str(special_idents::THIS));
        let params = vec![textual::Param {
            name: VarName::Local(this_lid),
            attrs: None,
            ty: static_ty,
        }];
        let attributes = textual::FuncAttributes::default();

        let coeffects = vec![]; // TODO(aorenste) is there any coeffects info around ?

        self.txf.define_function(
            &name,
            Some(&self.class.src_loc),
            &attributes,
            &coeffects,
            &params,
            &ty,
            &[],
            |fb| {
                let obj = fb.write_expr_stmt(textual::Expr::Alloc(ty.ty.deref()))?;
                fb.ret(obj)?;
                Ok(())
            },
        )
    }

    fn write_method(&mut self, method: ir::Method<'_>) -> Result {
        trace!(
            "Convert Method {}::{}",
            self.class.name.as_bstr(&self.unit_state.strings),
            method.name.as_bstr(&self.unit_state.strings)
        );

        let is_static = match method.func.attrs.is_static() {
            true => IsStatic::Static,
            false => IsStatic::NonStatic,
        };

        let this_ty = class_ty(self.class.name, is_static);

        let func_info = FuncInfo::Method(MethodInfo {
            name: method.name,
            attrs: method.func.attrs,
            class: &self.class,
            is_static,
            flags: method.flags,
        });

        if method.func.attrs.is_abstract() {
            func::write_func_decl(
                self.txf,
                self.unit_state,
                this_ty,
                method.func,
                Arc::new(func_info),
            )?;
        } else {
            func::lower_and_write_func(self.txf, self.unit_state, this_ty, method.func, func_info)?;
        }

        Ok(())
    }
}

/// For a given class return the Ty for its non-static (instance) type.
pub(crate) fn non_static_ty(class: ir::ClassId) -> textual::Ty {
    let cname = TypeName::Class(class);
    textual::Ty::Ptr(Box::new(textual::Ty::Type(cname)))
}

/// For a given class return the Ty for its static type.
pub(crate) fn static_ty(class: ir::ClassId) -> textual::Ty {
    let cname = TypeName::StaticClass(class);
    textual::Ty::Ptr(Box::new(textual::Ty::Type(cname)))
}

pub(crate) fn class_ty(class: ir::ClassId, is_static: IsStatic) -> textual::Ty {
    match is_static {
        IsStatic::Static => static_ty(class),
        IsStatic::NonStatic => non_static_ty(class),
    }
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

fn cmp_method(a: &ir::Method<'_>, b: &ir::Method<'_>, strings: &ir::StringInterner) -> Ordering {
    let line_a = a.func.locs[a.func.loc_id].line_begin as usize;
    let line_b = b.func.locs[b.func.loc_id].line_begin as usize;
    line_a
        .cmp(&line_b)
        .then_with(|| {
            // Same source line - use name.
            let name_a = strings.lookup_bstr(a.name.id);
            let name_b = strings.lookup_bstr(b.name.id);
            name_a.cmp(&name_b)
        })
        .then_with(|| {
            // Same name - use param count.
            a.func.params.len().cmp(&b.func.params.len())
        })
}
