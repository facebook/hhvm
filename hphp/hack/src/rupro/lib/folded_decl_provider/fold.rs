// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{inherit::Inherited, subst::Subst, subst::Substitution};
use crate::alloc::Allocator;
use crate::decl_defs::{
    AbstractTypeconst, CeVisibility, ClassConst, ClassConstKind, ClassEltFlags, ClassEltFlagsArgs,
    ClassishKind, ConsistentKind, DeclTy, FoldedClass, FoldedElement, ShallowClass,
    ShallowClassConst, ShallowMethod, ShallowProp, ShallowTypeconst, TaccessType, TypeConst,
    Typeconst, UserAttribute, Visibility,
};
use crate::reason::Reason;
use crate::special_names::SpecialNames;
use pos::{
    ClassConstName, ClassConstNameMap, MethodNameMap, ModuleName, Positioned, PropNameMap,
    TypeConstName, TypeConstNameMap, TypeName, TypeNameMap,
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
        let reason = R::class_class(pos.clone(), name);
        let classname_ty = DeclTy::apply(
            reason.clone(),
            Positioned::new(pos.clone(), self.special_names.classes.cClassname),
            [DeclTy::this(reason)].into(),
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

    /// Each concrete type constant `T = τ` implicitly defines a class
    /// constant of the same name `T` having type `TypeStructure<τ>`.
    fn type_const_structure(
        &self,
        sc: &ShallowClass<R>,
        stc: &ShallowTypeconst<R>,
    ) -> ClassConst<R> {
        let pos = stc.name.pos();
        let r = R::witness_from_decl(pos.clone());
        let tsid = Positioned::new(pos.clone(), TypeName(self.special_names.fb.cTypeStructure));
        // The type `this`.
        let tthis = DeclTy::this(r.clone());
        // The type `this::T`.
        let taccess = DeclTy::access(
            r.clone(),
            TaccessType {
                ty: tthis,
                type_const: stc.name.clone(),
            },
        );
        // The type `TypeStructure<this::T>`.
        let ts_ty = DeclTy::apply(r, tsid, [taccess].into());
        let kind = match &stc.kind {
            Typeconst::TCAbstract(AbstractTypeconst { default, .. }) => {
                ClassConstKind::CCAbstract(default.is_some())
            }
            Typeconst::TCConcrete(_) => ClassConstKind::CCConcrete,
        };
        // A class constant (which will be associated with the name `T`) of type
        // `TypeStructure<this::T>`.
        ClassConst {
            is_synthesized: true,
            kind,
            pos: pos.clone(),
            ty: ts_ty,
            origin: sc.name.id(),
            refs: Default::default(),
        }
    }

    fn decl_type_const(
        &self,
        type_consts: &mut TypeConstNameMap<TypeConst<R>>,
        class_consts: &mut ClassConstNameMap<ClassConst<R>>,
        sc: &ShallowClass<R>,
        stc: &ShallowTypeconst<R>,
    ) {
        // note(sf, 2022-02-10): c.f. Decl_folded_class.typeconst_fold
        match sc.kind {
            ClassishKind::Cenum | ClassishKind::CenumClass(_) => {}
            ClassishKind::Ctrait | ClassishKind::Cinterface | ClassishKind::Cclass(_) => {
                let TypeConstName(name) = stc.name.id();
                let ptc = type_consts.get(stc.name.id_ref());
                let ptc_enforceable = ptc.and_then(|tc| tc.enforceable.as_ref());
                let ptc_reifiable = ptc.and_then(|tc| tc.reifiable.as_ref());
                let type_const = TypeConst {
                    is_synthesized: false,
                    name: stc.name.clone(),
                    kind: stc.kind.clone(),
                    origin: sc.name.id(),
                    enforceable: stc.enforceable.as_ref().or(ptc_enforceable).cloned(),
                    reifiable: stc.reifiable.as_ref().or(ptc_reifiable).cloned(),
                    is_concreteized: false,
                    is_ctx: stc.is_ctx,
                };
                type_consts.insert(TypeConstName(name), type_const);
                class_consts.insert(ClassConstName(name), self.type_const_structure(sc, stc));
            }
        }
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
        ancestors: &mut TypeNameMap<DeclTy<R>>,
    ) {
        match ty.unwrap_class_type() {
            None => {}
            Some((_, pos_id, tyl)) => match parents.get(&pos_id.id()) {
                None => {
                    // The class lives in PHP land.
                    ancestors.insert(pos_id.id(), ty.clone());
                }
                Some(cls) => {
                    let subst = Subst::new(self.alloc, &cls.tparams, tyl);
                    let substitution = Substitution {
                        alloc: self.alloc,
                        subst: &subst,
                    };
                    // Update `ancestors`.
                    for (&anc_name, anc_ty) in &cls.ancestors {
                        ancestors.insert(anc_name, substitution.instantiate(anc_ty));
                    }
                    // Now add `ty`.
                    ancestors.insert(pos_id.id(), ty.clone());
                }
            },
        }
    }

    pub fn decl_class(
        &self,
        sc: &ShallowClass<R>,
        parents: &TypeNameMap<Arc<FoldedClass<R>>>,
    ) -> Arc<FoldedClass<R>> {
        let inh = Inherited::make(self.alloc, sc, parents);

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

        let mut type_consts = inh.type_consts;
        sc.typeconsts
            .iter()
            .for_each(|tc| self.decl_type_const(&mut type_consts, &mut consts, sc, tc));

        Arc::new(FoldedClass {
            name: sc.name.id(),
            pos: sc.name.pos().clone(),
            kind: sc.kind,
            is_final: sc.is_final,
            is_const: sc
                .user_attributes
                .iter()
                .any(|ua| ua.name.id() == self.special_names.user_attributes.uaConst),
            is_internal: sc
                .user_attributes
                .iter()
                .any(|ua| ua.name.id() == self.special_names.user_attributes.uaInternal),
            is_xhp: sc.is_xhp,
            support_dynamic_type: sc.support_dynamic_type,
            has_xhp_keyword: sc.has_xhp_keyword,
            module: sc.module.clone(),
            tparams: sc.tparams.clone(),
            where_constraints: sc.where_constraints.clone(),
            substs: inh.substs,
            ancestors,
            props,
            static_props,
            methods,
            static_methods,
            constructor,
            consts,
            type_consts,
            xhp_enum_values: sc.xhp_enum_values.clone(),
        })
    }
}
