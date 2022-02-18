// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::Class;
use crate::decl_defs::FoldedClass;
use crate::folded_decl_provider::FoldedDeclProvider;
use crate::reason::Reason;
use crate::typing_defs::ClassElt;
use dashmap::DashMap;
use once_cell::sync::OnceCell;
use pos::{BuildMethodNameHasher, BuildPropNameHasher, MethodName, PropName};
use std::fmt;
use std::sync::Arc;

#[derive(Debug)]
struct EagerMembers<R: Reason> {
    props: DashMap<PropName, Arc<ClassElt<R>>, BuildPropNameHasher>,
    static_props: DashMap<PropName, Arc<ClassElt<R>>, BuildPropNameHasher>,
    methods: DashMap<MethodName, Arc<ClassElt<R>>, BuildMethodNameHasher>,
    static_methods: DashMap<MethodName, Arc<ClassElt<R>>, BuildMethodNameHasher>,
    constructor: OnceCell<Option<Arc<ClassElt<R>>>>,
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
    members: EagerMembers<R>,
}

impl<R: Reason> fmt::Debug for ClassType<R> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        self.fetch_all_members();
        f.debug_struct("ClassType")
            .field("class", &self.class)
            .field("members", &self.members)
            .finish()
    }
}

impl<R: Reason> EagerMembers<R> {
    fn new() -> Self {
        Self {
            props: DashMap::default(),
            static_props: DashMap::default(),
            methods: DashMap::default(),
            static_methods: DashMap::default(),
            constructor: OnceCell::new(),
        }
    }
}

impl<R: Reason> ClassType<R> {
    pub fn new(provider: Arc<dyn FoldedDeclProvider<R>>, class: Arc<FoldedClass<R>>) -> Self {
        Self {
            provider,
            class,
            members: EagerMembers::new(),
        }
    }

    fn fetch_all_members(&self) {
        for (&prop, _) in self.class.props.iter() {
            self.get_prop(prop);
        }
        for (&prop, _) in self.class.static_props.iter() {
            self.get_static_prop(prop);
        }
        for (&method, _) in self.class.methods.iter() {
            self.get_method(method);
        }
        for (&method, _) in self.class.static_methods.iter() {
            self.get_static_method(method);
        }
        self.get_constructor();
    }
}

impl<R: Reason> Class<R> for ClassType<R> {
    fn get_prop(&self, name: PropName) -> Option<Arc<ClassElt<R>>> {
        if let Some(class_elt) = self.members.props.get(&name) {
            return Some(Arc::clone(&class_elt));
        }
        let folded_elt = match self.class.props.get(&name) {
            Some(fe) => fe,
            None => return None,
        };
        let ty = self
            .provider
            .get_shallow_property_type(folded_elt.origin, name)
            .expect("prop found in self.class.props, but not in folded decl provider");
        // TODO: perform substitutions on ty
        let class_elt = Arc::new(ClassElt::new(folded_elt, ty));
        self.members.props.insert(name, Arc::clone(&class_elt));
        Some(class_elt)
    }

    fn get_static_prop(&self, name: PropName) -> Option<Arc<ClassElt<R>>> {
        if let Some(class_elt) = self.members.static_props.get(&name) {
            return Some(Arc::clone(&class_elt));
        }
        let folded_elt = match self.class.static_props.get(&name) {
            Some(fe) => fe,
            None => return None,
        };
        let ty = self
            .provider
            .get_shallow_static_property_type(folded_elt.origin, name)
            .expect("prop found in self.class.static_props, but not in folded decl provider");
        // TODO: perform substitutions on ty
        let class_elt = Arc::new(ClassElt::new(folded_elt, ty));
        self.members
            .static_props
            .insert(name, Arc::clone(&class_elt));
        Some(class_elt)
    }

    fn get_method(&self, name: MethodName) -> Option<Arc<ClassElt<R>>> {
        if let Some(class_elt) = self.members.methods.get(&name) {
            return Some(Arc::clone(&class_elt));
        }
        let folded_elt = match self.class.methods.get(&name) {
            Some(fe) => fe,
            None => return None,
        };
        let ty = self
            .provider
            .get_shallow_method_type(folded_elt.origin, name)
            .expect("method found in self.class.methods, but not in folded decl provider");
        // TODO: perform substitutions on ty
        let class_elt = Arc::new(ClassElt::new(folded_elt, ty));
        self.members.methods.insert(name, Arc::clone(&class_elt));
        Some(class_elt)
    }

    fn get_static_method(&self, name: MethodName) -> Option<Arc<ClassElt<R>>> {
        if let Some(class_elt) = self.members.static_methods.get(&name) {
            return Some(Arc::clone(&class_elt));
        }
        let folded_elt = match self.class.static_methods.get(&name) {
            Some(fe) => fe,
            None => return None,
        };
        let ty = self
            .provider
            .get_shallow_static_method_type(folded_elt.origin, name)
            .expect("method found in self.class.static_methods, but not in folded decl provider");
        // TODO: perform substitutions on ty
        let class_elt = Arc::new(ClassElt::new(folded_elt, ty));
        self.members
            .static_methods
            .insert(name, Arc::clone(&class_elt));
        Some(class_elt)
    }

    fn get_constructor(&self) -> Option<Arc<ClassElt<R>>> {
        self.members
            .constructor
            .get_or_init(|| {
                let folded_elt = match &self.class.constructor {
                    Some(fe) => fe,
                    None => return None,
                };
                let ty = self
                    .provider
                    .get_shallow_constructor_type(folded_elt.origin)
                    .expect("constructor found in self.class, but not in folded decl provider");
                // TODO: perform substitutions on ty
                Some(Arc::new(ClassElt::new(folded_elt, ty)))
            })
            .as_ref()
            .map(Arc::clone)
    }
}
