// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::decl_defs::{
    CeVisibility, DeclTy, DeclTy_, FoldedClass, FoldedElement, ShallowClass, ShallowMethod,
    ShallowProp,
};
use crate::folded_decl_provider::FoldedDeclCache;
use crate::folded_decl_provider::{inherit::Inherited, subst::Subst};
use crate::reason::Reason;
use crate::shallow_decl_provider::ShallowDeclProvider;
use pos::{Positioned, Symbol, SymbolMap, TypeName, TypeNameMap, TypeNameSet};
use std::sync::Arc;

// note(sf, 2022-02-03): c.f. hphp/hack/src/decl/decl_folded_class.ml

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

    pub fn get_folded_class(&self, name: TypeName) -> Option<Arc<FoldedClass<R>>> {
        let mut stack = Default::default();
        self.get_folded_class_impl(&mut stack, name)
    }

    fn detect_cycle(&self, stack: &mut TypeNameSet, pos_id: &Positioned<TypeName, R::Pos>) -> bool {
        if stack.contains(&pos_id.id()) {
            todo!("TODO(hrust): register error");
        }
        false
    }

    fn visibility(
        &self,
        cls: TypeName,
        module: Option<&Positioned<Symbol, R::Pos>>,
        vis: oxidized::ast_defs::Visibility,
    ) -> CeVisibility<R::Pos> {
        use oxidized::ast_defs::Visibility;
        match vis {
            Visibility::Public => CeVisibility::Public,
            Visibility::Private => CeVisibility::Private(cls),
            Visibility::Protected => CeVisibility::Protected(cls),
            Visibility::Internal => module.map_or(CeVisibility::Public, |pid| {
                CeVisibility::Internal(pid.clone())
            }),
        }
    }

    fn decl_prop(
        &self,
        props: &mut SymbolMap<FoldedElement<R>>,
        sc: &ShallowClass<R>,
        sp: &ShallowProp<R>,
    ) {
        let cls = sc.name.id();
        let prop = sp.name.id();
        let vis = self.visibility(cls, sc.module.as_ref(), sp.visibility);
        let prop_flags = &sp.flags;
        let flag_args = oxidized_by_ref::typing_defs_flags::ClassEltFlagsArgs {
            xhp_attr: sp.xhp_attr,
            is_abstract: prop_flags.is_abstract(),
            is_final: true,
            is_superfluous_override: false,
            is_lsb: false,
            is_synthesized: false,
            is_const: prop_flags.is_const(),
            is_lateinit: prop_flags.is_lateinit(),
            is_dynamicallycallable: false,
            is_readonly_prop: prop_flags.is_readonly(),
            supports_dynamic_type: false,
            needs_init: prop_flags.needs_init(),
        };
        let elt = FoldedElement {
            origin: sc.name.id(),
            visibility: vis,
            deprecated: None,
            flags: oxidized_by_ref::typing_defs::ClassEltFlags::new(flag_args),
        };
        props.insert(prop, elt);
    }

    fn decl_static_prop(
        &self,
        static_props: &mut SymbolMap<FoldedElement<R>>,
        sc: &ShallowClass<R>,
        sp: &ShallowProp<R>,
    ) {
        let cls = sc.name.id();
        let prop = sp.name.id();
        let vis = self.visibility(cls, sc.module.as_ref(), sp.visibility);
        let prop_flags = &sp.flags;
        let flag_args = oxidized_by_ref::typing_defs_flags::ClassEltFlagsArgs {
            xhp_attr: sp.xhp_attr,
            is_abstract: prop_flags.is_abstract(),
            is_final: true,
            is_superfluous_override: false,
            is_lsb: prop_flags.is_lsb(),
            is_synthesized: false,
            is_const: prop_flags.is_const(),
            is_lateinit: prop_flags.is_lateinit(),
            is_dynamicallycallable: false,
            is_readonly_prop: prop_flags.is_readonly(),
            supports_dynamic_type: false,
            needs_init: prop_flags.needs_init(),
        };
        let elt = FoldedElement {
            origin: sc.name.id(),
            visibility: vis,
            deprecated: None,
            flags: oxidized_by_ref::typing_defs::ClassEltFlags::new(flag_args),
        };
        static_props.insert(prop, elt);
    }

    fn decl_method(
        &self,
        methods: &mut SymbolMap<FoldedElement<R>>,
        sc: &ShallowClass<R>,
        sm: &ShallowMethod<R>,
    ) {
        let cls = sc.name.id();
        let meth = sm.name.id();
        let vis = match (methods.get(&meth), sm.visibility) {
            (
                Some(FoldedElement {
                    visibility: CeVisibility::Protected(cls),
                    ..
                }),
                oxidized::ast_defs::Visibility::Protected,
            ) => CeVisibility::Protected(*cls),
            (_, v) => self.visibility(cls, sc.module.as_ref(), v),
        };

        let meth_flags = &sm.flags;
        let flag_args = oxidized_by_ref::typing_defs_flags::ClassEltFlagsArgs {
            xhp_attr: None,
            is_abstract: meth_flags.is_abstract(),
            is_final: meth_flags.is_final(),
            is_superfluous_override: meth_flags.is_override() && !methods.contains_key(&meth),
            is_lsb: false,
            is_synthesized: false,
            is_const: false,
            is_lateinit: false,
            is_dynamicallycallable: meth_flags.is_dynamicallycallable(),
            is_readonly_prop: false,
            supports_dynamic_type: meth_flags.supports_dynamic_type(),
            needs_init: false,
        };
        let elt = FoldedElement {
            origin: cls,
            visibility: vis,
            deprecated: sm.deprecated,
            flags: oxidized_by_ref::typing_defs::ClassEltFlags::new(flag_args),
        };

        methods.insert(meth, elt);
    }

    fn decl_class_type(
        &self,
        stack: &mut TypeNameSet,
        acc: &mut TypeNameMap<Arc<FoldedClass<R>>>,
        ty: &DeclTy<R>,
    ) {
        match &**ty.node() {
            DeclTy_::DTapply(pos_id, _tyl) => {
                if !self.detect_cycle(stack, pos_id) {
                    if let Some(folded_decl) = self.get_folded_class_impl(stack, pos_id.id()) {
                        acc.insert(pos_id.id(), folded_decl);
                    }
                }
            }
            _ => {}
        }
    }

    fn decl_class_parents(
        &self,
        stack: &mut TypeNameSet,
        sc: &ShallowClass<R>,
    ) -> TypeNameMap<Arc<FoldedClass<R>>> {
        let mut acc = Default::default();
        sc.extends
            .iter()
            .for_each(|ty| self.decl_class_type(stack, &mut acc, ty));
        acc
    }

    fn get_implements(
        &self,
        parents: &TypeNameMap<Arc<FoldedClass<R>>>,
        ty: &DeclTy<R>,
        inst: &mut TypeNameMap<DeclTy<R>>,
    ) {
        match ty.unwrap_class_type() {
            None => {}
            Some((_, pos_id, tyl)) => match parents.get(&pos_id.id()) {
                None => {
                    inst.insert(pos_id.id(), ty.clone());
                }
                Some(cls) => {
                    let subst = Subst::new((), tyl);
                    for (&anc_name, anc_ty) in &cls.ancestors {
                        inst.insert(anc_name, subst.instantiate(anc_ty));
                    }
                    inst.insert(pos_id.id(), ty.clone());
                }
            },
        }
    }

    fn decl_class_impl(
        &self,
        sc: &ShallowClass<R>,
        parents: &TypeNameMap<Arc<FoldedClass<R>>>,
    ) -> Arc<FoldedClass<R>> {
        let inh = Inherited::make(sc, parents);

        let mut props = inh.props;
        sc.props
            .iter()
            .for_each(|sp| self.decl_prop(&mut props, sc, sp));

        let mut static_props = inh.static_props;
        sc.static_props
            .iter()
            .for_each(|ssp| self.decl_static_prop(&mut static_props, sc, ssp));

        let mut methods = inh.methods;
        sc.methods
            .iter()
            .for_each(|sm| self.decl_method(&mut methods, sc, sm));

        let mut static_methods = inh.static_methods;
        sc.static_methods
            .iter()
            .for_each(|sm| self.decl_method(&mut static_methods, sc, sm));

        let mut anc = Default::default();
        sc.extends
            .iter()
            .for_each(|ty| self.get_implements(parents, ty, &mut anc));

        Arc::new(FoldedClass {
            name: sc.name.id(),
            pos: sc.name.pos().clone(),
            substs: inh.substs,
            ancestors: anc,
            props,
            static_props,
            methods,
            static_methods,
        })
    }

    fn decl_class(&self, stack: &mut TypeNameSet, name: TypeName) -> Option<Arc<FoldedClass<R>>> {
        let shallow_class = self.shallow_decl_provider.get_shallow_class(name)?;
        stack.insert(name);
        let parents = self.decl_class_parents(stack, &shallow_class);
        Some(self.decl_class_impl(&shallow_class, &parents))
    }

    fn get_folded_class_impl(
        &self,
        stack: &mut TypeNameSet,
        name: TypeName,
    ) -> Option<Arc<FoldedClass<R>>> {
        match self.cache.get_folded_class(name) {
            Some(rc) => Some(rc),
            None => match self.decl_class(stack, name) {
                None => None,
                Some(rc) => {
                    self.cache.put_folded_class(name, Arc::clone(&rc));
                    Some(rc)
                }
            },
        }
    }
}
