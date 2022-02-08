// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{
    Abstraction, ClassishKind, DeclTy, FoldedClass, FoldedElement, ShallowClass, SubstContext,
};
use crate::folded_decl_provider::subst::Subst;
use crate::reason::Reason;
use pos::{MethodName, MethodNameMap, PropNameMap, TypeName, TypeNameMap};
use std::collections::hash_map::Entry;
use std::sync::Arc;

// note(sf, 2022-02-03): c.f. hphp/hack/src/decl/decl_inherit.ml

#[derive(Debug)]
pub(crate) struct Inherited<R: Reason> {
    // note(sf, 2022-01-27): c.f. `Decl_inherit.inherited`
    pub(crate) substs: TypeNameMap<SubstContext<R>>,
    pub(crate) props: PropNameMap<FoldedElement<R>>,
    pub(crate) static_props: PropNameMap<FoldedElement<R>>,
    pub(crate) methods: MethodNameMap<FoldedElement<R>>,
    pub(crate) static_methods: MethodNameMap<FoldedElement<R>>,
    pub(crate) constructor: Option<FoldedElement<R>>,
}

impl<R: Reason> std::default::Default for Inherited<R> {
    fn default() -> Self {
        Self {
            substs: Default::default(),
            props: Default::default(),
            static_props: Default::default(),
            methods: Default::default(),
            static_methods: Default::default(),
            constructor: Default::default(),
        }
    }
}

impl<R: Reason> Inherited<R> {
    // Reasons to keep the old signature:
    //   - We don't want to override a concrete method with an
    //     abstract one;
    //   - We don't want to override a method that's actually
    //     implemented by the programmer with one that's "synthetic",
    //     e.g. arising merely from a require-extends declaration in a
    //     trait.
    // When these two considerations conflict, we give precedence to
    // abstractness for determining priority of the method.
    fn should_keep_old_sig(new_sig: &FoldedElement<R>, old_sig: &FoldedElement<R>) -> bool {
        !old_sig.is_abstract() && new_sig.is_abstract()
            || old_sig.is_abstract() == new_sig.is_abstract()
                && !old_sig.is_synthesized()
                && new_sig.is_synthesized()
    }

    fn add_constructor(&mut self, constructor: Option<FoldedElement<R>>) {
        match (constructor.as_ref(), self.constructor.as_ref()) {
            (None, _) => {}
            (Some(other_ctor), Some(self_ctor))
                if Self::should_keep_old_sig(other_ctor, self_ctor) => {}
            (_, _) => self.constructor = constructor,
        }
    }

    fn add_substs(&mut self, other_substs: TypeNameMap<SubstContext<R>>) {
        for (key, new_sc) in other_substs {
            match self.substs.entry(key) {
                Entry::Vacant(e) => {
                    e.insert(new_sc);
                }
                Entry::Occupied(mut e) => {
                    if e.get().from_req_extends && !new_sc.from_req_extends {
                        // If the old substitution context came via required extends
                        // then we want to use the substitutions from the actual
                        // extends instead. e.g.
                        // ```
                        // class Base<+T> {}
                        // trait MyTrait { require extends Base<mixed>; }
                        // class Child extends Base<int> { use MyTrait; }
                        // ```
                        // Here the substitution context `{MyTrait/[T -> mixed]}`
                        // should be overridden by `{Child/[T -> int]}`, because
                        // it's the actual extension of class `Base`.
                        e.insert(new_sc);
                    }
                }
            }
        }
    }

    fn add_method(
        methods: &mut MethodNameMap<FoldedElement<R>>,
        (key, mut fe): (MethodName, FoldedElement<R>),
    ) {
        match methods.entry(key) {
            Entry::Vacant(entry) => {
                // The method didn't exist so far, let's add it.
                entry.insert(fe);
            }
            Entry::Occupied(mut entry) => {
                if !Self::should_keep_old_sig(&fe, entry.get()) {
                    fe.set_is_superfluous_override(false);
                    entry.insert(fe);
                } else {
                    // Otherwise, we *are* overwriting a method
                    // definition. This is OK when a naming
                    // conflict is parent class vs trait (trait
                    // wins!), but not really OK when the naming
                    // conflict is trait vs trait (we rely on HHVM
                    // to catch the error at runtime).
                }
            }
        }
    }

    fn add_methods(&mut self, other_methods: MethodNameMap<FoldedElement<R>>) {
        for (key, fe) in other_methods {
            Self::add_method(&mut self.methods, (key, fe))
        }
    }

    fn add_static_methods(&mut self, other_static_methods: MethodNameMap<FoldedElement<R>>) {
        for (key, fe) in other_static_methods {
            Self::add_method(&mut self.static_methods, (key, fe))
        }
    }

    fn add_props(&mut self, other_props: PropNameMap<FoldedElement<R>>) {
        self.props.extend(other_props)
    }

    fn add_static_props(&mut self, other_static_props: PropNameMap<FoldedElement<R>>) {
        self.static_props.extend(other_static_props)
    }

    fn add_inherited(&mut self, other: Self) {
        let Self {
            substs,
            props,
            static_props,
            methods,
            static_methods,
            constructor,
        } = other;
        self.add_substs(substs);
        self.add_props(props);
        self.add_static_props(static_props);
        self.add_methods(methods);
        self.add_static_methods(static_methods);
        self.add_constructor(constructor);
    }

    fn make_substitution(_cls: &FoldedClass<R>, params: &[DeclTy<R>]) -> TypeNameMap<DeclTy<R>> {
        Subst::new((), params).into()
    }

    fn inherit_hack_class(
        child: &ShallowClass<R>,
        parent_name: TypeName,
        parent: &FoldedClass<R>,
        argl: &[DeclTy<R>],
    ) -> Self {
        let subst = Self::make_substitution(parent, argl);
        // TODO(hrust): Do we need sharing?
        let mut substs = parent.substs.clone();
        substs.insert(
            parent_name,
            SubstContext {
                subst,
                class_context: child.name.id(),
                from_req_extends: false,
            },
        );
        Self {
            substs,
            props: parent.props.clone(),
            static_props: parent.static_props.clone(),
            methods: parent.methods.clone(),
            static_methods: parent.static_methods.clone(),
            constructor: parent.constructor.clone(),
        }
    }

    fn from_class(
        sc: &ShallowClass<R>,
        parents: &TypeNameMap<Arc<FoldedClass<R>>>,
        parent_ty: &DeclTy<R>,
    ) -> Self {
        if let Some((_, parent_pos_id, parent_tyl)) = parent_ty.unwrap_class_type() {
            if let Some(parent_folded_decl) = parents.get(&parent_pos_id.id()) {
                return Self::inherit_hack_class(
                    sc,
                    parent_pos_id.id(),
                    parent_folded_decl,
                    parent_tyl,
                );
            }
        }
        Self::default()
    }

    fn from_class_xhp_attrs_only(
        _sc: &ShallowClass<R>,
        parents: &TypeNameMap<Arc<FoldedClass<R>>>,
        ty: &DeclTy<R>,
    ) -> Self {
        if let Some((_, pos_id, _tyl)) = ty.unwrap_class_type() {
            if let Some(class) = parents.get(&pos_id.id()) {
                return Self::inherit_hack_xhp_attrs_only(class);
            }
        }
        Self::default()
    }

    fn add_from_xhp_attr_uses(
        &mut self,
        sc: &ShallowClass<R>,
        parents: &TypeNameMap<Arc<FoldedClass<R>>>,
    ) {
        for ty in &sc.xhp_attr_uses {
            self.add_inherited(Self::from_class_xhp_attrs_only(sc, parents, ty))
        }
    }

    fn add_from_parents(
        &mut self,
        sc: &ShallowClass<R>,
        parents: &TypeNameMap<Arc<FoldedClass<R>>>,
    ) {
        let mut tys: Vec<&DeclTy<R>> = Vec::new();
        match sc.kind {
            ClassishKind::Cclass(Abstraction::Abstract) => {
                tys.extend(&sc.implements);
                tys.extend(&sc.extends);
            }
            ClassishKind::Ctrait => {
                tys.extend(&sc.implements);
                tys.extend(&sc.extends);
                tys.extend(&sc.req_implements);
            }
            ClassishKind::Cclass(_)
            | ClassishKind::Cinterface
            | ClassishKind::Cenum
            | ClassishKind::CenumClass(_) => {
                tys.extend(&sc.extends);
            }
        };

        // Interfaces implemented, classes extended and interfaces required to
        // be implemented.
        for ty in tys.iter().rev() {
            self.add_inherited(Self::from_class(sc, parents, ty));
        }
    }

    fn add_from_requirements(
        &mut self,
        sc: &ShallowClass<R>,
        parents: &TypeNameMap<Arc<FoldedClass<R>>>,
    ) {
        for ty in &sc.req_extends {
            let mut inherited = Self::from_class(sc, parents, ty);
            inherited.mark_as_synthesized();
            self.add_inherited(inherited);
        }
    }

    // This logic deals with importing XHP attributes from an XHP class via the
    // "attribute :foo;" syntax.
    fn inherit_hack_xhp_attrs_only(class: &FoldedClass<R>) -> Self {
        // Filter out properties that are not XHP attributes.
        Inherited {
            props: class
                .props
                .iter()
                .filter(|(_, prop)| prop.get_xhp_attr().is_some())
                .map(|(name, prop)| (name.clone(), prop.clone()))
                .collect(),
            ..Default::default()
        }
    }

    fn add_from_traits(
        &mut self,
        sc: &ShallowClass<R>,
        parents: &TypeNameMap<Arc<FoldedClass<R>>>,
    ) {
        for ty in &sc.uses {
            self.add_inherited(Self::from_class(sc, parents, ty));
        }
    }

    fn mark_as_synthesized(&mut self) {
        for (_, ctx) in self.substs.iter_mut() {
            ctx.from_req_extends = true;
        }
        if let Some(ref mut elt) = self.constructor {
            elt.set_is_synthesized(true);
        }
        for (_, prop) in self.props.iter_mut() {
            prop.set_is_synthesized(true);
        }
        for (_, static_prop) in self.static_props.iter_mut() {
            static_prop.set_is_synthesized(true);
        }
        for (_, method) in self.methods.iter_mut() {
            method.set_is_synthesized(true);
        }
        for (_, static_method) in self.static_methods.iter_mut() {
            static_method.set_is_synthesized(true);
        }
        //TODO typeconsts, consts
    }

    pub(crate) fn make(sc: &ShallowClass<R>, parents: &TypeNameMap<Arc<FoldedClass<R>>>) -> Self {
        let mut inh = Self::default();
        inh.add_from_parents(sc, parents); // Members inherited from parents ...
        inh.add_from_requirements(sc, parents);
        inh.add_from_traits(sc, parents); // ... can be overridden by traits.
        inh.add_from_xhp_attr_uses(sc, parents);

        inh
    }
}
