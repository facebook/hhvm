// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{DeclTy, FoldedClass, FoldedElement, ShallowClass, SubstContext};
use crate::folded_decl_provider::subst::Subst;
use crate::reason::Reason;
use pos::{SymbolMap, TypeName, TypeNameMap};
use std::collections::hash_map::Entry;
use std::sync::Arc;

#[derive(Debug)]
pub(crate) struct Inherited<R: Reason> {
    // note(sf, 2022-01-27): c.f. `Decl_inherit.inherited`
    pub(crate) substs: TypeNameMap<SubstContext<R>>,
    pub(crate) methods: SymbolMap<FoldedElement<R>>,
    pub(crate) static_methods: SymbolMap<FoldedElement<R>>,
}

impl<R: Reason> std::default::Default for Inherited<R> {
    fn default() -> Self {
        Self {
            substs: Default::default(),
            methods: Default::default(),
            static_methods: Default::default(),
        }
    }
}

impl<R: Reason> Inherited<R> {
    fn should_keep_old_sig(_new_sig: &FoldedElement<R>, _old_sig: &FoldedElement<R>) -> bool {
        true
    }

    fn add_substs(&mut self, other_substs: TypeNameMap<SubstContext<R>>) {
        for (key, subst) in other_substs {
            self.substs.entry(key).or_insert(subst);
        }
    }

    fn add_methods(&mut self, other_methods: SymbolMap<FoldedElement<R>>) {
        for (key, mut fe) in other_methods {
            match self.methods.entry(key) {
                Entry::Vacant(entry) => {
                    entry.insert(fe);
                }
                Entry::Occupied(mut entry) => {
                    if !Self::should_keep_old_sig(&fe, entry.get()) {
                        std::mem::swap(entry.get_mut(), &mut fe);
                    }
                }
            }
        }
    }

    fn add_static_methods(&mut self, other_static_methods: SymbolMap<FoldedElement<R>>) {
        for (key, mut fe) in other_static_methods {
            match self.static_methods.entry(key) {
                Entry::Vacant(entry) => {
                    entry.insert(fe);
                }
                Entry::Occupied(mut entry) => {
                    if !Self::should_keep_old_sig(&fe, entry.get()) {
                        std::mem::swap(entry.get_mut(), &mut fe);
                    }
                }
            }
        }
    }

    fn add_inherited(&mut self, other: Self) {
        let Self {
            substs,
            methods,
            static_methods,
        } = other;
        self.add_substs(substs);
        self.add_methods(methods);
        self.add_static_methods(static_methods);
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
            methods: parent.methods.clone(),
            static_methods: parent.static_methods.clone(),
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
