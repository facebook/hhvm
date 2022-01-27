// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::{hash_map::Entry, HashMap};
use std::rc::Rc;

use hcons::Hc;

use crate::decl_defs::{DeclTy, FoldedClass, FoldedElement, ShallowClass, SubstContext};
use crate::folded_decl_provider::subst::Subst;
use crate::pos::Symbol;
use crate::reason::Reason;

pub(crate) struct Inherited<R: Reason> {
    pub(crate) substs: HashMap<Symbol, SubstContext<R>>,
    pub(crate) methods: HashMap<Symbol, FoldedElement<R>>,
}

impl<R: Reason> Inherited<R> {
    fn empty() -> Self {
        Self {
            substs: HashMap::new(),
            methods: HashMap::new(),
        }
    }

    fn should_keep_old_sig(_new_sig: &FoldedElement<R>, _old_sig: &FoldedElement<R>) -> bool {
        true
    }

    fn add_substs(&mut self, other_substs: HashMap<Symbol, SubstContext<R>>) {
        for (key, subst) in other_substs {
            self.substs.entry(Hc::clone(&key)).or_insert(subst);
        }
    }

    fn add_methods(&mut self, other_methods: HashMap<Symbol, FoldedElement<R>>) {
        for (key, mut fe) in other_methods {
            match self.methods.entry(Hc::clone(&key)) {
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
        let Inherited { substs, methods } = other;
        self.add_substs(substs);
        self.add_methods(methods);
    }

    fn make_substitution(
        _cls: &FoldedClass<R>,
        params: &[DeclTy<R>],
    ) -> HashMap<Symbol, DeclTy<R>> {
        Subst::new((), params).into()
    }

    fn inherit_hack_class(
        child: &ShallowClass<R>,
        parent_name: &Symbol,
        parent: &FoldedClass<R>,
        argl: &[DeclTy<R>],
    ) -> Self {
        let subst = Self::make_substitution(parent, argl);
        // TODO(hrust): Do we need sharing?
        let mut substs = parent.substs.clone();
        substs.insert(
            Hc::clone(parent_name),
            SubstContext {
                subst,
                class_context: Hc::clone(child.name.id()),
            },
        );
        Self {
            substs,
            methods: parent.methods.clone(),
        }
    }

    fn from_class(
        sc: &ShallowClass<R>,
        parents: &HashMap<Symbol, Rc<FoldedClass<R>>>,
        parent_ty: &DeclTy<R>,
    ) -> Self {
        if let Some((_, parent_pos_id, parent_tyl)) = parent_ty.unwrap_class_type() {
            if let Some(parent_folded_decl) = parents.get(parent_pos_id.id()) {
                return Self::inherit_hack_class(
                    sc,
                    parent_pos_id.id(),
                    parent_folded_decl,
                    parent_tyl,
                );
            }
        }
        Self::empty()
    }

    fn from_parent(sc: &ShallowClass<R>, parents: &HashMap<Symbol, Rc<FoldedClass<R>>>) -> Self {
        let all_inherited = sc
            .extends
            .iter()
            .map(|extends| Self::from_class(sc, parents, extends));
        let mut inh = Self::empty();
        for parent_inh in all_inherited {
            inh.add_inherited(parent_inh)
        }
        inh
    }

    pub(crate) fn make(
        sc: &ShallowClass<R>,
        parents: &HashMap<Symbol, Rc<FoldedClass<R>>>,
    ) -> Self {
        Self::from_parent(sc, parents)
    }
}
