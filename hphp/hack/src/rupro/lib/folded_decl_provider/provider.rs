// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::{HashMap, HashSet};
use std::sync::Arc;

use hcons::Hc;

use crate::decl_defs::{
    CeVisibility, DeclTy, DeclTy_, FoldedClass, FoldedElement, ShallowClass, ShallowMethod,
};
use crate::folded_decl_provider::FoldedDeclCache;
use crate::folded_decl_provider::{inherit::Inherited, subst::Subst};
use crate::pos::{PosId, Symbol};
use crate::reason::Reason;
use crate::shallow_decl_provider::ShallowDeclProvider;

#[derive(Debug)]
pub struct FoldedDeclProvider<R: Reason> {
    cache: Arc<dyn FoldedDeclCache<Reason = R>>,
    shallow_decl_provider: Arc<ShallowDeclProvider<R>>,
}

impl<R: Reason> FoldedDeclProvider<R> {
    pub fn new(
        cache: Arc<dyn FoldedDeclCache<Reason = R>>,
        shallow_decl_provider: Arc<ShallowDeclProvider<R>>,
    ) -> Self {
        Self {
            cache,
            shallow_decl_provider,
        }
    }

    pub fn get_folded_class(&self, name: &Symbol) -> Option<Arc<FoldedClass<R>>> {
        let mut stack = HashSet::new();
        self.get_folded_class_impl(&mut stack, name)
    }

    fn detect_cycle(&self, stack: &mut HashSet<Symbol>, pos_id: &PosId<R::Pos>) -> bool {
        if stack.contains(pos_id.id()) {
            unimplemented!("TODO(hrust): register error");
        }
        false
    }

    fn visibility(
        &self,
        cls: &Symbol,
        module: Option<&PosId<R::Pos>>,
        vis: oxidized::ast_defs::Visibility,
    ) -> CeVisibility<R> {
        use oxidized::ast_defs::Visibility;
        match vis {
            Visibility::Public => CeVisibility::Public,
            Visibility::Private => CeVisibility::Private(Hc::clone(cls)),
            Visibility::Protected => CeVisibility::Protected(Hc::clone(cls)),
            Visibility::Internal => module.map_or(CeVisibility::Public, |pid| {
                CeVisibility::Internal(pid.clone())
            }),
        }
    }

    fn decl_method(
        &self,
        methods: &mut HashMap<Symbol, FoldedElement<R>>,
        sc: &ShallowClass<R>,
        sm: &ShallowMethod<R>,
    ) {
        let cls = sc.name.id();
        let meth = sm.name.id();
        let vis = match (methods.get(meth), sm.visibility) {
            (
                Some(FoldedElement {
                    visibility: CeVisibility::Protected(s),
                    ..
                }),
                oxidized::ast_defs::Visibility::Protected,
            ) => CeVisibility::Protected(Hc::clone(s)),
            (_, v) => self.visibility(cls, sc.module.as_ref(), v),
        };
        let elt = FoldedElement {
            origin: Hc::clone(cls),
            visibility: vis,
        };
        methods.insert(Hc::clone(meth), elt);
    }

    fn decl_class_type(
        &self,
        stack: &mut HashSet<Symbol>,
        acc: &mut HashMap<Symbol, Arc<FoldedClass<R>>>,
        ty: &DeclTy<R>,
    ) {
        match &**ty.node() {
            DeclTy_::DTapply(pos_id, _tyl) => {
                if !self.detect_cycle(stack, pos_id) {
                    if let Some(folded_decl) = self.get_folded_class_impl(stack, pos_id.id()) {
                        acc.insert(Hc::clone(pos_id.id()), folded_decl);
                    }
                }
            }
            _ => {}
        }
    }

    fn decl_class_parents(
        &self,
        stack: &mut HashSet<Symbol>,
        sc: &ShallowClass<R>,
    ) -> HashMap<Symbol, Arc<FoldedClass<R>>> {
        let mut acc = HashMap::new();
        sc.extends
            .iter()
            .for_each(|ty| self.decl_class_type(stack, &mut acc, ty));
        acc
    }

    fn get_implements(
        &self,
        parents: &HashMap<Symbol, Arc<FoldedClass<R>>>,
        ty: &DeclTy<R>,
        inst: &mut HashMap<Symbol, DeclTy<R>>,
    ) {
        match ty.unwrap_class_type() {
            None => {}
            Some((_, pos_id, tyl)) => match parents.get(pos_id.id()) {
                None => {
                    inst.insert(Hc::clone(pos_id.id()), ty.clone());
                }
                Some(cls) => {
                    let subst = Subst::new((), tyl);
                    for (anc_name, anc_ty) in &cls.ancestors {
                        inst.insert(Hc::clone(anc_name), subst.instantiate(anc_ty));
                    }
                    inst.insert(Hc::clone(pos_id.id()), ty.clone());
                }
            },
        }
    }

    fn decl_class_impl(
        &self,
        sc: &ShallowClass<R>,
        parents: &HashMap<Symbol, Arc<FoldedClass<R>>>,
    ) -> Arc<FoldedClass<R>> {
        let inh = Inherited::make(sc, parents);

        let mut methods = inh.methods;
        sc.methods
            .iter()
            .for_each(|sm| self.decl_method(&mut methods, sc, sm));

        let mut static_methods = inh.static_methods;
        sc.static_methods
            .iter()
            .for_each(|sm| self.decl_method(&mut static_methods, sc, sm));

        let mut anc = HashMap::new();
        sc.extends
            .iter()
            .for_each(|ty| self.get_implements(parents, ty, &mut anc));

        Arc::new(FoldedClass {
            name: sc.name.id().clone(),
            pos: sc.name.pos().clone(),
            substs: inh.substs,
            ancestors: anc,
            methods,
            static_methods,
        })
    }

    fn decl_class(
        &self,
        stack: &mut HashSet<Symbol>,
        name: &Symbol,
    ) -> Option<Arc<FoldedClass<R>>> {
        let shallow_class = self.shallow_decl_provider.get_shallow_class(name)?;
        stack.insert(Hc::clone(name));
        let parents = self.decl_class_parents(stack, &shallow_class);
        Some(self.decl_class_impl(&shallow_class, &parents))
    }

    fn get_folded_class_impl(
        &self,
        stack: &mut HashSet<Symbol>,
        name: &Symbol,
    ) -> Option<Arc<FoldedClass<R>>> {
        match self.cache.get_folded_class(name) {
            Some(rc) => Some(rc),
            None => match self.decl_class(stack, name) {
                None => None,
                Some(rc) => {
                    self.cache
                        .put_folded_class(Hc::clone(name), Arc::clone(&rc));
                    Some(rc)
                }
            },
        }
    }
}
