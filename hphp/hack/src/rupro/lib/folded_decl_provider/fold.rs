// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::alloc::Allocator;
use crate::decl_defs::{
    CeVisibility, ClassConst, ClassConstKind, ClassEltFlags, ClassEltFlagsArgs, ConsistentKind,
    DeclTy, DeclTy_, FoldedClass, FoldedElement, ShallowClass, ShallowClassConst, ShallowMethod,
    ShallowProp, UserAttribute, Visibility,
};
use crate::folded_decl_provider::{inherit::Inherited, subst::Subst};
use crate::reason::{Reason, ReasonImpl};
use crate::special_names::SpecialNames;
use pos::{
    ClassConstName, ClassConstNameMap, MethodNameMap, ModuleName, Positioned, PropNameMap,
    TypeName, TypeNameMap,
};
use std::sync::Arc;

// note(sf, 2022-02-03): c.f. hphp/hack/src/decl/decl_folded_class.ml

#[derive(Debug)]
pub struct DeclFolder<R: Reason> {
    alloc: &'static Allocator<R>,
    special_names: &'static SpecialNames,
}

impl<R: Reason> DeclFolder<R> {
    pub fn new(alloc: &'static Allocator<R>, special_names: &'static SpecialNames) -> Self {
        Self {
            alloc,
            special_names,
        }
    }

    fn visibility(
        &self,
        cls: TypeName,
        module: Option<ModuleName>,
        vis: Visibility,
    ) -> CeVisibility {
        match vis {
            Visibility::Public => CeVisibility::Public,
            Visibility::Private => CeVisibility::Private(cls),
            Visibility::Protected => CeVisibility::Protected(cls),
            Visibility::Internal => module.map_or(CeVisibility::Public, |module_name| {
                CeVisibility::Internal(module_name)
            }),
        }
    }

    /// Every class, interface, and trait implicitly defines a `::class` to allow
    /// accessing its fully qualified name as a string.
    fn decl_class_class(
        &self,
        consts: &mut ClassConstNameMap<ClassConst<R>>,
        sc: &ShallowClass<R>,
    ) {
        // note(sf, 2022-02-08): c.f. Decl_folded_class.class_class_decl
        let pos = sc.name.pos();
        let name = sc.name.id();
        let reason = R::mk(|| ReasonImpl::RclassClass(pos.clone(), name));
        let classname_ty = self.alloc.decl_ty(
            reason.clone(),
            DeclTy_::DTapply(Box::new((
                Positioned::new(pos.clone(), self.special_names.classes.cClassname),
                vec![self.alloc.decl_ty(reason, DeclTy_::DTthis)].into_boxed_slice(),
            ))),
        );
        let class_const = ClassConst {
            is_synthesized: true,
            kind: ClassConstKind::CCConcrete,
            pos: pos.clone(),
            ty: classname_ty,
            origin: name,
            refs: Box::default(),
        };
        consts.insert(
            ClassConstName(self.special_names.members.mClass),
            class_const,
        );
    }

    fn decl_class_const(
        &self,
        consts: &mut ClassConstNameMap<ClassConst<R>>,
        sc: &ShallowClass<R>,
        c: &ShallowClassConst<R>,
    ) {
        // note(sf, 2022-02-10): c.f. Decl_folded_class.class_const_fold
        let class_const = ClassConst {
            is_synthesized: false,
            kind: c.kind,
            pos: c.name.pos().clone(),
            ty: c.ty.clone(),
            origin: sc.name.id(),
            refs: c.refs.clone(),
        };
        consts.insert(c.name.id(), class_const);
    }

    fn decl_prop(
        &self,
        props: &mut PropNameMap<FoldedElement>,
        sc: &ShallowClass<R>,
        sp: &ShallowProp<R>,
    ) {
        // note(sf, 2022-02-08): c.f. Decl_folded_class.prop_decl
        let cls = sc.name.id();
        let prop = sp.name.id();
        let vis = self.visibility(cls, sc.module.as_ref().map(Positioned::id), sp.visibility);
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
        static_props: &mut PropNameMap<FoldedElement>,
        sc: &ShallowClass<R>,
        sp: &ShallowProp<R>,
    ) {
        let cls = sc.name.id();
        let prop = sp.name.id();
        let vis = self.visibility(cls, sc.module.as_ref().map(Positioned::id), sp.visibility);
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
        methods: &mut MethodNameMap<FoldedElement>,
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
            (_, v) => self.visibility(cls, sc.module.as_ref().map(Positioned::id), v),
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

    fn decl_constructor(&self, constructor: &mut Option<FoldedElement>, sc: &ShallowClass<R>) {
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
                let vis =
                    self.visibility(cls, sc.module.as_ref().map(Positioned::id), sm.visibility);
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

    pub fn decl_class(
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
        sc.consts
            .iter()
            .for_each(|c| self.decl_class_const(&mut consts, sc, c));
        self.decl_class_class(&mut consts, sc);

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
            type_consts: Default::default(), //TODO
        })
    }
}
