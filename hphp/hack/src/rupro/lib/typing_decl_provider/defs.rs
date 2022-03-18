// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{Class, Error, Result};
use crate::decl_defs::{ty::ConsistentKind, DeclTy, FoldedClass};
use crate::dependency_registrar::DeclName;
use crate::folded_decl_provider::{FoldedDeclProvider, Substitution};
use crate::reason::Reason;
use crate::typing_defs::ClassElt;
use once_cell::unsync::OnceCell;
use pos::{MethodName, MethodNameMap, PropName, PropNameMap, TypeName};
use std::cell::RefCell;
use std::fmt;
use std::rc::Rc;
use std::sync::Arc;

/// c.f. OCaml type `Typing_classes_heap.eager_members`
#[derive(Debug)]
struct Members<R: Reason> {
    props: PropNameMap<Rc<ClassElt<R>>>,
    static_props: PropNameMap<Rc<ClassElt<R>>>,
    methods: MethodNameMap<Rc<ClassElt<R>>>,
    static_methods: MethodNameMap<Rc<ClassElt<R>>>,
    constructor: OnceCell<Option<Rc<ClassElt<R>>>>,
}

/// A typing `ClassType` (c.f. the `Eager` variant of OCaml type
/// `Typing_classes_heap.class_t`) contains a folded decl and a cache of class
/// members. The purpose of the class-member-cache is to abstract over the fact
/// that class elements in a folded decl don't contain their type (in hh_server,
/// the type is stored on a separate heap, to reduce overfetching and
/// duplication). When asked for a class member, the `ClassType` checks its
/// member-cache. If not present, it looks up the type of the member using the
/// `FoldedDeclProvider`, and populates its member-cache with a new `ClassElt`
/// containing that type and any other metadata from the `FoldedElt`.
pub struct ClassType<R: Reason> {
    provider: Arc<dyn FoldedDeclProvider<R>>,
    class: Arc<FoldedClass<R>>,
    members: RefCell<Members<R>>,
}

impl<R: Reason> fmt::Debug for ClassType<R> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        self.fetch_all_members().unwrap();
        f.debug_struct("ClassType")
            .field("class", &self.class)
            .field("members", &self.members.borrow())
            .finish()
    }
}

impl<R: Reason> Members<R> {
    fn new() -> Self {
        Self {
            props: Default::default(),
            static_props: Default::default(),
            methods: Default::default(),
            static_methods: Default::default(),
            constructor: OnceCell::new(),
        }
    }
}

impl<R: Reason> ClassType<R> {
    pub fn new(provider: Arc<dyn FoldedDeclProvider<R>>, class: Arc<FoldedClass<R>>) -> Self {
        Self {
            provider,
            class,
            members: RefCell::new(Members::new()),
        }
    }

    fn fetch_all_members(&self) -> Result<()> {
        for (&prop, _) in self.class.props.iter() {
            self.get_prop(self.class.name.into(), prop)?;
        }
        for (&prop, _) in self.class.static_props.iter() {
            self.get_static_prop(self.class.name.into(), prop)?;
        }
        for (&method, _) in self.class.methods.iter() {
            self.get_method(self.class.name.into(), method)?;
        }
        for (&method, _) in self.class.static_methods.iter() {
            self.get_static_method(self.class.name.into(), method)?;
        }
        self.get_constructor(self.class.name.into())?;
        Ok(())
    }

    // Invariant violation: we expect our provider to provide member types for any
    // member from a FoldedClass it returned. See docs for `FoldedDeclProvider`.
    // c.f. OCaml exception `Decl_heap_elems_bug`
    fn member_type_missing<T>(&self, kind: &str, origin: TypeName, name: impl AsRef<str>) -> T {
        panic!(
            "Could not find {kind} {origin}::{} (inherited by {})",
            name.as_ref(),
            self.class.name
        );
    }

    // If `self.class` has a substitution context for `origin`, apply the
    // associated substitution to `ty`.
    fn instantiate(&self, ty: DeclTy<R>, origin: TypeName) -> DeclTy<R> {
        match self.class.substs.get(&origin) {
            Some(ctx) => Substitution { subst: &ctx.subst }.instantiate(&ty),
            None => ty,
        }
    }
}

impl<R: Reason> Class<R> for ClassType<R> {
    fn get_prop(&self, dependent: DeclName, name: PropName) -> Result<Option<Rc<ClassElt<R>>>> {
        if let Some(class_elt) = self.members.borrow().props.get(&name) {
            return Ok(Some(Rc::clone(class_elt)));
        }
        let folded_elt = match self.class.props.get(&name) {
            Some(fe) => fe,
            None => return Ok(None),
        };
        let origin = folded_elt.origin;
        let ty = self.instantiate(
            self.provider
                .get_shallow_property_type(dependent, origin, name)?
                .unwrap_or_else(|| self.member_type_missing("property", origin, name)),
            origin,
        );
        let class_elt = Rc::new(ClassElt::new(folded_elt, ty));
        self.members
            .borrow_mut()
            .props
            .insert(name, Rc::clone(&class_elt));
        Ok(Some(class_elt))
    }

    fn get_static_prop(
        &self,
        dependent: DeclName,
        name: PropName,
    ) -> Result<Option<Rc<ClassElt<R>>>> {
        if let Some(class_elt) = self.members.borrow().static_props.get(&name) {
            return Ok(Some(Rc::clone(class_elt)));
        }
        let folded_elt = match self.class.static_props.get(&name) {
            Some(fe) => fe,
            None => return Ok(None),
        };
        let origin = folded_elt.origin;
        let ty = self.instantiate(
            self.provider
                .get_shallow_static_property_type(dependent, origin, name)?
                .unwrap_or_else(|| self.member_type_missing("static property", origin, name)),
            origin,
        );
        let class_elt = Rc::new(ClassElt::new(folded_elt, ty));
        self.members
            .borrow_mut()
            .static_props
            .insert(name, Rc::clone(&class_elt));
        Ok(Some(class_elt))
    }

    fn get_method(&self, dependent: DeclName, name: MethodName) -> Result<Option<Rc<ClassElt<R>>>> {
        if let Some(class_elt) = self.members.borrow().methods.get(&name) {
            return Ok(Some(Rc::clone(class_elt)));
        }
        let folded_elt = match self.class.methods.get(&name) {
            Some(fe) => fe,
            None => return Ok(None),
        };
        let origin = folded_elt.origin;
        let ty = self.instantiate(
            self.provider
                .get_shallow_method_type(dependent, origin, name)?
                .unwrap_or_else(|| self.member_type_missing("method", origin, name)),
            origin,
        );
        let class_elt = Rc::new(ClassElt::new(folded_elt, ty));
        self.members
            .borrow_mut()
            .methods
            .insert(name, Rc::clone(&class_elt));
        Ok(Some(class_elt))
    }

    fn get_static_method(
        &self,
        dependent: DeclName,
        name: MethodName,
    ) -> Result<Option<Rc<ClassElt<R>>>> {
        if let Some(class_elt) = self.members.borrow().static_methods.get(&name) {
            return Ok(Some(Rc::clone(class_elt)));
        }
        let folded_elt = match self.class.static_methods.get(&name) {
            Some(fe) => fe,
            None => return Ok(None),
        };
        let origin = folded_elt.origin;
        let ty = self.instantiate(
            self.provider
                .get_shallow_static_method_type(dependent, origin, name)?
                .unwrap_or_else(|| self.member_type_missing("static method", origin, name)),
            origin,
        );
        let class_elt = Rc::new(ClassElt::new(folded_elt, ty));
        self.members
            .borrow_mut()
            .static_methods
            .insert(name, Rc::clone(&class_elt));
        Ok(Some(class_elt))
    }

    fn get_constructor(
        &self,
        dependent: DeclName,
    ) -> Result<(Option<Rc<ClassElt<R>>>, ConsistentKind)> {
        let elt = self
            .members
            .borrow_mut()
            .constructor
            .get_or_try_init::<_, Error>(|| {
                let folded_elt = match &self.class.constructor.elt {
                    Some(fe) => fe,
                    None => return Ok(None),
                };
                let origin = folded_elt.origin;
                let ty = self.instantiate(
                    self.provider
                        .get_shallow_constructor_type(dependent, origin)?
                        .unwrap_or_else(|| {
                            self.member_type_missing("constructor", origin, "__construct")
                        }),
                    origin,
                );
                Ok(Some(Rc::new(ClassElt::new(folded_elt, ty))))
            })?
            .as_ref()
            .map(Rc::clone);
        Ok((elt, self.class.constructor.consistency))
    }
}
