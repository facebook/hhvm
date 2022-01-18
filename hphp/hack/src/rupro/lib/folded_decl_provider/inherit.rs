// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::{hash_map::Entry, HashMap};
use std::rc::Rc;

use crate::decl_defs::{DeclTy, FoldedClass, FoldedElement, ShallowClass, SubstContext};
use crate::folded_decl_provider::subst::Subst;
use crate::pos::Symbol;
use crate::reason::Reason;

pub(crate) struct Inherited<REASON: Reason> {
    pub(crate) ih_substs: HashMap<Symbol, SubstContext<REASON>>,
    pub(crate) ih_methods: HashMap<Symbol, FoldedElement>,
}

impl<REASON: Reason> Inherited<REASON> {
    fn empty() -> Self {
        Self {
            ih_substs: HashMap::new(),
            ih_methods: HashMap::new(),
        }
    }

    fn should_keep_old_sig(_new_sig: &FoldedElement, _old_sig: &FoldedElement) -> bool {
        true
    }

    fn add_substs(&mut self, other_substs: HashMap<Symbol, SubstContext<REASON>>) {
        for (key, subst) in other_substs {
            self.ih_substs.entry(key.clone()).or_insert(subst);
        }
    }

    fn add_methods(&mut self, other_methods: HashMap<Symbol, FoldedElement>) {
        for (key, mut fe) in other_methods {
            match self.ih_methods.entry(key.clone()) {
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
        let Inherited {
            ih_substs,
            ih_methods,
        } = other;
        self.add_substs(ih_substs);
        self.add_methods(ih_methods);
    }

    fn make_substitution(
        _cls: &FoldedClass<REASON>,
        params: &[DeclTy<REASON>],
    ) -> HashMap<Symbol, DeclTy<REASON>> {
        Subst::new((), params).into()
    }

    fn inherit_hack_class(
        child: &ShallowClass<REASON>,
        parent_name: &Symbol,
        parent: &FoldedClass<REASON>,
        argl: &[DeclTy<REASON>],
    ) -> Self {
        let subst = Self::make_substitution(parent, argl);
        // TODO(hrust): Do we need sharing?
        let mut substs = parent.dc_substs.clone();
        substs.insert(
            parent_name.clone(),
            SubstContext {
                sc_subst: subst,
                sc_class_context: child.sc_name.id().clone(),
            },
        );
        Self {
            ih_substs: substs,
            ih_methods: parent.dc_methods.clone(),
        }
    }

    fn from_class(
        sc: &ShallowClass<REASON>,
        parents: &HashMap<Symbol, Rc<FoldedClass<REASON>>>,
        parent_ty: &DeclTy<REASON>,
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

    fn from_parent(
        sc: &ShallowClass<REASON>,
        parents: &HashMap<Symbol, Rc<FoldedClass<REASON>>>,
    ) -> Self {
        let all_inherited = sc
            .sc_extends
            .iter()
            .map(|extends| Self::from_class(sc, parents, extends));
        let mut inh = Self::empty();
        for parent_inh in all_inherited {
            inh.add_inherited(parent_inh)
        }
        inh
    }

    pub(crate) fn make(
        sc: &ShallowClass<REASON>,
        parents: &HashMap<Symbol, Rc<FoldedClass<REASON>>>,
    ) -> Self {
        Self::from_parent(sc, parents)
    }
}
