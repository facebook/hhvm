// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::alloc::Allocator;
use crate::cache::Cache;
use crate::decl_defs::{
    CeVisibility, ClassConst, ClassConstKind, ClassEltFlags, ClassEltFlagsArgs, ConsistentKind,
    DeclTy, DeclTy_, FoldedClass, FoldedElement, ShallowClass, ShallowMethod, ShallowProp,
    UserAttribute, Visibility,
};
use crate::folded_decl_provider::{inherit::Inherited, subst::Subst};
use crate::reason::{Reason, ReasonImpl};
use crate::shallow_decl_provider::ShallowDeclProvider;
use crate::special_names::SpecialNames;
use pos::{
    ClassConstName, MethodName, MethodNameMap, ModuleName, Positioned, PropName, PropNameMap,
    TypeName, TypeNameMap, TypeNameSet,
};
use std::sync::Arc;

// note(sf, 2022-02-03): c.f. hphp/hack/src/decl/decl_folded_class.ml

/// A `FoldedDeclProvider` which, if the requested class name is not present in
/// its cache, recursively computes the folded decl for that class by requesting
/// the shallow decls of that class and its ancestors from its
/// `ShallowDeclProvider`.
#[derive(Debug)]
pub struct LazyFoldedDeclProvider<R: Reason> {
    cache: Arc<dyn Cache<TypeName, Arc<FoldedClass<R>>>>,
    alloc: &'static Allocator<R>,
    special_names: &'static SpecialNames,
    shallow_decl_provider: Arc<dyn ShallowDeclProvider<R>>,
}

impl<R: Reason> LazyFoldedDeclProvider<R> {
    pub fn new(
        cache: Arc<dyn Cache<TypeName, Arc<FoldedClass<R>>>>,
        alloc: &'static Allocator<R>,
        special_names: &'static SpecialNames,
        shallow_decl_provider: Arc<dyn ShallowDeclProvider<R>>,
    ) -> Self {
        Self {
            cache,
            alloc,
            special_names,
            shallow_decl_provider,
        }
    }
}

impl<R: Reason> super::FoldedDeclProvider<R> for LazyFoldedDeclProvider<R> {
    fn get_class(&self, name: TypeName) -> Option<Arc<FoldedClass<R>>> {
        let mut stack = Default::default();
        self.get_folded_class_impl(&mut stack, name)
    }

    fn get_shallow_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        self.shallow_decl_provider
            .get_property_type(class_name, property_name)
    }

    fn get_shallow_static_property_type(
        &self,
        class_name: TypeName,
        property_name: PropName,
    ) -> Option<DeclTy<R>> {
        self.shallow_decl_provider
            .get_static_property_type(class_name, property_name)
    }

    fn get_shallow_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        self.shallow_decl_provider
            .get_method_type(class_name, method_name)
    }

    fn get_shallow_static_method_type(
        &self,
        class_name: TypeName,
        method_name: MethodName,
    ) -> Option<DeclTy<R>> {
        self.shallow_decl_provider
            .get_static_method_type(class_name, method_name)
    }

    fn get_shallow_constructor_type(&self, class_name: TypeName) -> Option<DeclTy<R>> {
        self.shallow_decl_provider.get_constructor_type(class_name)
    }
}

impl<R: Reason> LazyFoldedDeclProvider<R> {
    fn detect_cycle(&self, stack: &mut TypeNameSet, pos_id: &Positioned<TypeName, R::Pos>) -> bool {
        if stack.contains(&pos_id.id()) {
            todo!("TODO(hrust): register error");
        }
        false
    }

    fn visibility(
        &self,
        cls: TypeName,
        module: Option<&Positioned<ModuleName, R::Pos>>,
        vis: Visibility,
    ) -> CeVisibility<R::Pos> {
        match vis {
            Visibility::Public => CeVisibility::Public,
            Visibility::Private => CeVisibility::Private(cls),
            Visibility::Protected => CeVisibility::Protected(cls),
            Visibility::Internal => module.map_or(CeVisibility::Public, |pid| {
                CeVisibility::Internal(pid.clone())
            }),
        }
    }

    /// Every class, interface, and trait implicitly defines a `::class` to allow
    /// accessing its fully qualified name as a string.
    fn decl_class_class(&self, class_id: &Positioned<TypeName, R::Pos>) -> ClassConst<R> {
        // note(sf, 2022-02-08): c.f. Decl_folded_class.class_class_decl
        let pos = class_id.pos();
        let name = class_id.id();
        let reason = R::mk(|| ReasonImpl::RclassClass(pos.clone(), name));
        let classname_ty = self.alloc.decl_ty(
            reason.clone(),
            DeclTy_::DTapply(
                Positioned::new(pos.clone(), self.special_names.classes.cClassname),
                vec![self.alloc.decl_ty(reason, DeclTy_::DTthis)],
            ),
        );
        ClassConst {
            is_synthesized: true,
            kind: ClassConstKind::CCConcrete,
            pos: pos.clone(),
            ty: classname_ty,
            origin: name.clone(),
            refs: Vec::new(),
        }
    }

    fn decl_prop(
        &self,
        props: &mut PropNameMap<FoldedElement<R>>,
        sc: &ShallowClass<R>,
        sp: &ShallowProp<R>,
    ) {
        // note(sf, 2022-02-08): c.f. Decl_folded_class.prop_decl
        let cls = sc.name.id();
        let prop = sp.name.id();
        let vis = self.visibility(cls, sc.module.as_ref(), sp.visibility);
        let prop_flags = &sp.flags;
        let flag_args = ClassEltFlagsArgs {
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
            flags: ClassEltFlags::new(flag_args),
        };
        props.insert(prop, elt);
    }

    fn decl_static_prop(
        &self,
        static_props: &mut PropNameMap<FoldedElement<R>>,
        sc: &ShallowClass<R>,
        sp: &ShallowProp<R>,
    ) {
        let cls = sc.name.id();
        let prop = sp.name.id();
        let vis = self.visibility(cls, sc.module.as_ref(), sp.visibility);
        let prop_flags = &sp.flags;
        let flag_args = ClassEltFlagsArgs {
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
            flags: ClassEltFlags::new(flag_args),
        };
        static_props.insert(prop, elt);
    }

    fn decl_method(
        &self,
        methods: &mut MethodNameMap<FoldedElement<R>>,
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
                Visibility::Protected,
            ) => CeVisibility::Protected(*cls),
            (_, v) => self.visibility(cls, sc.module.as_ref(), v),
        };

        let meth_flags = &sm.flags;
        let flag_args = ClassEltFlagsArgs {
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
            flags: ClassEltFlags::new(flag_args),
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

    fn decl_constructor(&self, constructor: &mut Option<FoldedElement<R>>, sc: &ShallowClass<R>) {
        // Constructors in children of `sc` must be consistent?
        let _consistent_kind = if sc.is_final {
            ConsistentKind::FinalClass
        } else if sc.user_attributes.iter().any(|UserAttribute { name, .. }| {
            name.id() == self.special_names.user_attributes.uaConsistentConstruct
        }) {
            ConsistentKind::ConsistentConstruct
        } else {
            ConsistentKind::Inconsistent
        };

        match &sc.constructor {
            None => {}
            Some(sm) => {
                let cls = sc.name.id();
                let vis = self.visibility(cls, sc.module.as_ref(), sm.visibility);
                let meth_flags = &sm.flags;
                let flag_args = ClassEltFlagsArgs {
                    xhp_attr: None,
                    is_abstract: meth_flags.is_abstract(),
                    is_final: meth_flags.is_final(),
                    is_superfluous_override: false,
                    is_lsb: false,
                    is_synthesized: false,
                    is_const: false,
                    is_lateinit: false,
                    is_dynamicallycallable: false,
                    is_readonly_prop: false,
                    supports_dynamic_type: false,
                    needs_init: false,
                };
                let elt = FoldedElement {
                    origin: sc.name.id(),
                    visibility: vis,
                    deprecated: sm.deprecated,
                    flags: ClassEltFlags::new(flag_args),
                };
                *constructor = Some(elt)
            }
        }
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

        let mut constructor = inh.constructor;
        self.decl_constructor(&mut constructor, sc);

        let mut ancestors = Default::default();
        sc.extends
            .iter()
            .for_each(|ty| self.get_implements(parents, ty, &mut ancestors));

        let mut consts = inh.consts;
        consts.insert(
            ClassConstName(self.special_names.members.mClass),
            self.decl_class_class(&sc.name),
        );

        Arc::new(FoldedClass {
            name: sc.name.id(),
            pos: sc.name.pos().clone(),
            substs: inh.substs,
            ancestors,
            props,
            static_props,
            methods,
            static_methods,
            constructor,
            consts,
        })
    }

    fn decl_class(&self, stack: &mut TypeNameSet, name: TypeName) -> Option<Arc<FoldedClass<R>>> {
        let shallow_class = self.shallow_decl_provider.get_class(name)?;
        stack.insert(name);
        let parents = self.decl_class_parents(stack, &shallow_class);
        Some(self.decl_class_impl(&shallow_class, &parents))
    }

    fn get_folded_class_impl(
        &self,
        stack: &mut TypeNameSet,
        name: TypeName,
    ) -> Option<Arc<FoldedClass<R>>> {
        match self.cache.get(name) {
            Some(rc) => Some(rc),
            None => match self.decl_class(stack, name) {
                None => None,
                Some(rc) => {
                    self.cache.insert(name, Arc::clone(&rc));
                    Some(rc)
                }
            },
        }
    }
}
