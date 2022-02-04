// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{DeclTy, FoldedClass, FoldedElement, ShallowClass, SubstContext};
use crate::folded_decl_provider::subst::Subst;
use crate::reason::Reason;
use oxidized_by_ref as obr;
use pos::{Symbol, SymbolMap, TypeName, TypeNameMap};
use std::collections::hash_map::Entry;
use std::sync::Arc;

// note(sf, 2022-02-03): c.f. hphp/hack/src/decl/decl_inherit.ml

#[derive(Debug)]
pub(crate) struct Inherited<R: Reason> {
    // note(sf, 2022-01-27): c.f. `Decl_inherit.inherited`
    pub(crate) substs: TypeNameMap<SubstContext<R>>,
    pub(crate) props: SymbolMap<FoldedElement<R>>,
    pub(crate) static_props: SymbolMap<FoldedElement<R>>,
    pub(crate) methods: SymbolMap<FoldedElement<R>>,
    pub(crate) static_methods: SymbolMap<FoldedElement<R>>,
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
        use obr::typing_defs_flags::ClassEltFlags;

        let is_new_sig_abstract = new_sig.flags.contains(ClassEltFlags::ABSTRACT);
        let is_old_sig_abstract = old_sig.flags.contains(ClassEltFlags::ABSTRACT);
        let is_new_sig_synthesized = new_sig.flags.contains(ClassEltFlags::SYNTHESIZED);
        let is_old_sig_synthesized = old_sig.flags.contains(ClassEltFlags::SYNTHESIZED);

        !is_old_sig_abstract && is_new_sig_abstract
            || is_old_sig_abstract == is_new_sig_abstract
                && !is_old_sig_synthesized
                && is_new_sig_synthesized
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
        for (key, subst) in other_substs {
            self.substs.entry(key).or_insert(subst);
        }
    }

    fn add_method(
        methods: &mut SymbolMap<FoldedElement<R>>,
        (key, mut fe): (Symbol, FoldedElement<R>),
    ) {
        use obr::typing_defs_flags::ClassEltFlags;

        match methods.entry(key) {
            Entry::Vacant(entry) => {
                // The method didn't exist so far, let's add it.
                entry.insert(fe);
            }
            Entry::Occupied(mut entry) => {
                if !Self::should_keep_old_sig(&fe, entry.get()) {
                    fe.flags.set(ClassEltFlags::SUPERFLUOUS_OVERRIDE, false);
                    std::mem::swap(entry.get_mut(), &mut fe);
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

    fn add_methods(&mut self, other_methods: SymbolMap<FoldedElement<R>>) {
        for (key, fe) in other_methods {
            Self::add_method(&mut self.methods, (key, fe))
        }
    }

    fn add_static_methods(&mut self, other_static_methods: SymbolMap<FoldedElement<R>>) {
        for (key, fe) in other_static_methods {
            Self::add_method(&mut self.static_methods, (key, fe))
        }
    }

    fn add_props(&mut self, other_props: SymbolMap<FoldedElement<R>>) {
        self.props.extend(other_props)
    }

    fn add_static_props(&mut self, other_static_props: SymbolMap<FoldedElement<R>>) {
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

    fn from_parent(sc: &ShallowClass<R>, parents: &TypeNameMap<Arc<FoldedClass<R>>>) -> Self {
        let all_inherited = sc
            .extends
            .iter()
            .map(|extends| Self::from_class(sc, parents, extends));
        let mut inh = Self::default();
        for parent_inh in all_inherited {
            inh.add_inherited(parent_inh)
        }
        inh
    }

    pub(crate) fn make(sc: &ShallowClass<R>, parents: &TypeNameMap<Arc<FoldedClass<R>>>) -> Self {
        Self::from_parent(sc, parents)
    }
}
