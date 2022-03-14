// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{inherit::Inherited, subst::Subst, subst::Substitution};
use crate::decl_defs::{
    folded::Constructor, AbstractTypeconst, CeVisibility, ClassConst, ClassConstKind,
    ClassEltFlags, ClassEltFlagsArgs, ClassishKind, ConsistentKind, DeclTy, FoldedClass,
    FoldedElement, Requirement, ShallowClass, ShallowClassConst, ShallowMethod, ShallowProp,
    ShallowTypeconst, TaccessType, TypeConst, Typeconst, Visibility,
};
use crate::reason::Reason;
use crate::special_names::SpecialNames;
use crate::typing_error::{Primary, TypingError};
use eq_modulo_pos::EqModuloPos;
use oxidized::global_options::GlobalOptions;
use pos::{
    ClassConstName, ClassConstNameIndexMap, MethodNameIndexMap, ModuleName, Positioned, PropName,
    PropNameIndexMap, PropNameIndexSet, TypeConstName, TypeConstNameIndexMap, TypeName,
    TypeNameIndexMap, TypeNameIndexSet,
};
use std::sync::Arc;

// note(sf, 2022-02-03): c.f. hphp/hack/src/decl/decl_folded_class.ml

#[derive(Debug)]
pub struct DeclFolder<'a, R: Reason> {
    special_names: &'static SpecialNames,
    #[allow(dead_code)] // This can be removed after this field's first use.
    opts: &'a GlobalOptions,
    /// The class whose folded decl we are producing.
    child: &'a ShallowClass<R>,
    /// The folded decls of all (recursive) ancestors of `child`.
    parents: &'a TypeNameIndexMap<Arc<FoldedClass<R>>>,
    /// Hack errors which will be written to `child`'s folded decl.
    errors: Vec<TypingError<R>>,
}

#[derive(PartialEq)]
enum Pass {
    Extends,
    Traits,
    Xhp,
}

impl<'a, R: Reason> DeclFolder<'a, R> {
    pub fn decl_class(
        opts: &'a GlobalOptions,
        special_names: &'static SpecialNames,
        child: &'a ShallowClass<R>,
        parents: &'a TypeNameIndexMap<Arc<FoldedClass<R>>>,
        errors: Vec<TypingError<R>>,
    ) -> Arc<FoldedClass<R>> {
        let this = Self {
            opts,
            special_names,
            child,
            parents,
            errors,
        };
        this.decl_class_impl()
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
    fn decl_class_class(&self, consts: &mut ClassConstNameIndexMap<ClassConst<R>>) {
        // note(sf, 2022-02-08): c.f. Decl_folded_class.class_class_decl
        let pos = self.child.name.pos();
        let name = self.child.name.id();
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
    fn type_const_structure(&self, stc: &ShallowTypeconst<R>) -> ClassConst<R> {
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
            origin: self.child.name.id(),
            refs: Default::default(),
        }
    }

    fn decl_type_const(
        &self,
        type_consts: &mut TypeConstNameIndexMap<TypeConst<R>>,
        class_consts: &mut ClassConstNameIndexMap<ClassConst<R>>,
        stc: &ShallowTypeconst<R>,
    ) {
        // note(sf, 2022-02-10): c.f. Decl_folded_class.typeconst_fold
        match self.child.kind {
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
                    origin: self.child.name.id(),
                    enforceable: stc.enforceable.as_ref().or(ptc_enforceable).cloned(),
                    reifiable: stc.reifiable.as_ref().or(ptc_reifiable).cloned(),
                    is_concretized: false,
                    is_ctx: stc.is_ctx,
                };
                type_consts.insert(TypeConstName(name), type_const);
                class_consts.insert(ClassConstName(name), self.type_const_structure(stc));
            }
        }
    }

    fn decl_class_const(
        &self,
        consts: &mut ClassConstNameIndexMap<ClassConst<R>>,
        c: &ShallowClassConst<R>,
    ) {
        // note(sf, 2022-02-10): c.f. Decl_folded_class.class_const_fold
        let class_const = ClassConst {
            is_synthesized: false,
            kind: c.kind,
            pos: c.name.pos().clone(),
            ty: c.ty.clone(),
            origin: self.child.name.id(),
            refs: c.refs.clone(),
        };
        consts.insert(c.name.id(), class_const);
    }

    fn decl_prop(&self, props: &mut PropNameIndexMap<FoldedElement>, sp: &ShallowProp<R>) {
        // note(sf, 2022-02-08): c.f. Decl_folded_class.prop_decl
        let cls = self.child.name.id();
        let prop = sp.name.id();
        let vis = self.visibility(
            cls,
            self.child.module.as_ref().map(Positioned::id),
            sp.visibility,
        );
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
            origin: self.child.name.id(),
            visibility: vis,
            deprecated: None,
            flags: ClassEltFlags::new(flag_args),
        };
        props.insert(prop, elt);
    }

    fn decl_static_prop(
        &self,
        static_props: &mut PropNameIndexMap<FoldedElement>,
        sp: &ShallowProp<R>,
    ) {
        let cls = self.child.name.id();
        let prop = sp.name.id();
        let vis = self.visibility(
            cls,
            self.child.module.as_ref().map(Positioned::id),
            sp.visibility,
        );
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
            origin: self.child.name.id(),
            visibility: vis,
            deprecated: None,
            flags: ClassEltFlags::new(flag_args),
        };
        static_props.insert(prop, elt);
    }

    fn decl_method(&self, methods: &mut MethodNameIndexMap<FoldedElement>, sm: &ShallowMethod<R>) {
        let cls = self.child.name.id();
        let meth = sm.name.id();
        let vis = match (methods.get(&meth), sm.visibility) {
            (
                Some(FoldedElement {
                    visibility: CeVisibility::Protected(cls),
                    ..
                }),
                Visibility::Protected,
            ) => CeVisibility::Protected(*cls),
            (_, v) => self.visibility(cls, self.child.module.as_ref().map(Positioned::id), v),
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

    fn decl_constructor(&self, constructor: &mut Constructor) {
        // Constructors in children of `self.child` must be consistent?
        let consistency = if self.child.is_final {
            ConsistentKind::FinalClass
        } else if (self.child.user_attributes.iter())
            .any(|ua| ua.name.id() == self.special_names.user_attributes.uaConsistentConstruct)
        {
            ConsistentKind::ConsistentConstruct
        } else {
            ConsistentKind::Inconsistent
        };

        let elt = self.child.constructor.as_ref().map(|sm| {
            let cls = self.child.name.id();
            let vis = self.visibility(
                cls,
                self.child.module.as_ref().map(Positioned::id),
                sm.visibility,
            );
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
            FoldedElement {
                origin: self.child.name.id(),
                visibility: vis,
                deprecated: sm.deprecated,
                flags: ClassEltFlags::new(flag_args),
            }
        });
        *constructor = Constructor::new(
            elt,
            ConsistentKind::coalesce(constructor.consistency, consistency),
        )
    }

    fn get_implements(&self, ty: &DeclTy<R>, ancestors: &mut TypeNameIndexMap<DeclTy<R>>) {
        if let Some((_, pos_id, tyl)) = ty.unwrap_class_type() {
            match self.parents.get(&pos_id.id()) {
                None => {
                    // The class lives in PHP land.
                    ancestors.insert(pos_id.id(), ty.clone());
                }
                Some(cls) => {
                    let subst = Subst::new(&cls.tparams, tyl);
                    let substitution = Substitution { subst: &subst };
                    // Update `ancestors`.
                    for (&anc_name, anc_ty) in &cls.ancestors {
                        ancestors.insert(anc_name, substitution.instantiate(anc_ty));
                    }
                    // Now add `ty`.
                    ancestors.insert(pos_id.id(), ty.clone());
                }
            }
        }
    }

    /// Check that the kind of a class is compatible with its parent
    /// For example, a class cannot extend an interface, an interface cannot
    /// extend a trait etc ...
    /// TODO: T87242856
    fn check_extend_kind(
        &mut self,
        parent_pos: &R::Pos,
        parent_kind: ClassishKind,
        parent_name: &TypeName,
        child_pos: &R::Pos,
        child_kind: ClassishKind,
        child_name: &TypeName,
    ) {
        match (parent_kind, child_kind) {
            // What is allowed
            (ClassishKind::Cclass(_), ClassishKind::Cclass(_))
            | (ClassishKind::Ctrait, ClassishKind::Ctrait)
            | (ClassishKind::Cinterface, ClassishKind::Cinterface)
            | (ClassishKind::CenumClass(_), ClassishKind::CenumClass(_)) => {}
            // enums extend `BuiltinEnum` under the hood
            (ClassishKind::Cclass(k), ClassishKind::Cenum | ClassishKind::CenumClass(_))
                if k.is_abstract() => {}
            // What is disallowed
            _ => self
                .errors
                .push(TypingError::primary(Primary::WrongExtendKind {
                    parent_pos: parent_pos.clone(),
                    parent_kind,
                    parent_name: *parent_name,
                    pos: child_pos.clone(),
                    kind: child_kind,
                    name: *child_name,
                })),
        }
    }

    fn add_class_parent_or_trait(
        &mut self,
        pass: Pass,
        extends: &mut TypeNameIndexSet,
        ty: &DeclTy<R>,
    ) {
        if let Some((_, pos_id, _)) = ty.unwrap_class_type() {
            extends.insert(pos_id.id());
            if let Some(cls) = self.parents.get(&pos_id.id()) {
                if pass == Pass::Extends {
                    self.check_extend_kind(
                        pos_id.pos(),
                        cls.kind,
                        &cls.name,
                        self.child.name.pos(),
                        self.child.kind,
                        &self.child.name.id(),
                    );
                }

                if pass == Pass::Xhp {
                    // If we are crawling the xhp attribute deps, need to merge their xhp deps as well
                    // XHP attribute dependencies don't actually pull the trait into the class,
                    // so we need to track them totally separately.
                    extends.extend(cls.xhp_attr_deps.iter().cloned());
                }
                extends.extend(cls.extends.iter().cloned());
            }
        }
    }

    fn get_extends(&mut self) -> TypeNameIndexSet {
        let mut extends = TypeNameIndexSet::new();
        for extend in self.child.extends.iter() {
            self.add_class_parent_or_trait(Pass::Extends, &mut extends, extend)
        }
        for use_ in self.child.uses.iter() {
            self.add_class_parent_or_trait(Pass::Traits, &mut extends, use_)
        }
        extends
    }

    fn get_xhp_attr_deps(&mut self) -> TypeNameIndexSet {
        let mut xhp_attr_deps = TypeNameIndexSet::new();
        for xhp_attr_use in self.child.xhp_attr_uses.iter() {
            self.add_class_parent_or_trait(Pass::Xhp, &mut xhp_attr_deps, xhp_attr_use)
        }
        xhp_attr_deps
    }

    /// Accumulate requirements so that we can successfully check the bodies
    /// of trait methods / check that classes satisfy these requirements
    fn flatten_parent_class_reqs(
        &self,
        req_ancestors: &mut Vec<Requirement<R>>,
        req_ancestors_extends: &mut TypeNameIndexSet,
        parent_ty: &DeclTy<R>,
    ) {
        if let Some((_, pos_id, parent_params)) = parent_ty.unwrap_class_type() {
            if let Some(cls) = self.parents.get(&pos_id.id()) {
                let subst = Subst::new(&cls.tparams, parent_params);
                let substitution = Substitution { subst: &subst };
                // TODO: Do we need to rev? Or was that just a limitation of OCaml's library?
                cls.req_ancestors.iter().rev().for_each(|req_ancestor| {
                    let ty = substitution.instantiate(&req_ancestor.ty);
                    req_ancestors.push(Requirement::new(pos_id.pos().clone(), ty));
                });
                match self.child.kind {
                    ClassishKind::Cclass(_) => {
                        // Not necessary to accumulate req_ancestors_extends for classes --
                        // it's not used
                    }
                    ClassishKind::Ctrait | ClassishKind::Cinterface => {
                        req_ancestors_extends.extend(cls.req_ancestors_extends.iter().cloned());
                    }
                    ClassishKind::Cenum | ClassishKind::CenumClass(_) => {
                        panic!();
                    }
                }
            }
        }
    }

    fn declared_class_req(
        &self,
        req_ancestors: &mut Vec<Requirement<R>>,
        req_ancestors_extends: &mut TypeNameIndexSet,
        req_ty: &DeclTy<R>,
    ) {
        if let Some((_, pos_id, _)) = req_ty.unwrap_class_type() {
            // Since the req is declared on this class, we should
            // emphatically *not* substitute: a require extends Foo<T> is
            // going to be this class's <T>
            req_ancestors.push(Requirement::new(pos_id.pos().clone(), req_ty.clone()));
            req_ancestors_extends.insert(pos_id.id());

            if let Some(cls) = self.parents.get(&pos_id.id()) {
                req_ancestors_extends.extend(cls.extends.iter().cloned());
                req_ancestors_extends.extend(cls.xhp_attr_deps.iter().cloned());
                // The req may be on an interface that has reqs of its own; the
                // flattened ancestry required by *those* reqs need to be added
                // in to, e.g., interpret accesses to protected functions inside
                // traits
                req_ancestors_extends.extend(cls.req_ancestors_extends.iter().cloned());
            }
        }
    }

    /// Cheap hack: we cannot do unification / subtyping in the decl phase because
    /// the type arguments of the types that we are trying to unify may not have
    /// been declared yet. See the test iface_require_circular.php for details.
    ///
    /// However, we don't want a super long req_extends list because of the perf
    /// overhead. And while we can't do proper unification we can dedup types
    /// that are syntactically equivalent.
    ///
    /// A nicer solution might be to add a phase in between type-decl and type-check
    /// that prunes the list via proper subtyping, but that's a little more work
    /// than I'm willing to do now.
    fn naive_dedup(&self, req_ancestors: &mut Vec<Requirement<R>>) {
        let mut seen_reqs: TypeNameIndexMap<Vec<DeclTy<R>>> = TypeNameIndexMap::new();
        // Reverse to match the OCaml ordering for building the seen_reqs map
        // (since OCaml uses `rev_filter_map` for perf reasons)
        req_ancestors.reverse();
        req_ancestors.retain(|req_extend| {
            if let Some((_, pos_id, targs)) = req_extend.ty.unwrap_class_type() {
                if let Some(seen_targs) = seen_reqs.get(&pos_id.id()) {
                    if targs.eq_modulo_pos(seen_targs) {
                        false
                    } else {
                        // Seems odd to replace the existing targs list when we
                        // see a different one, but the OCaml does it, so we
                        // need to as well
                        seen_reqs.insert(pos_id.id(), targs.to_vec());
                        true
                    }
                } else {
                    seen_reqs.insert(pos_id.id(), targs.to_vec());
                    true
                }
            } else {
                true
            }
        });
        // Reverse again to match the OCaml ordering for the returned list
        req_ancestors.reverse();
    }

    fn get_class_requirements(&self) -> (Vec<Requirement<R>>, TypeNameIndexSet) {
        let mut req_ancestors = vec![];
        let mut req_ancestors_extends = TypeNameIndexSet::new();

        for req_extend in self.child.req_extends.iter() {
            self.declared_class_req(&mut req_ancestors, &mut req_ancestors_extends, req_extend);
        }

        for req_implement in self.child.req_implements.iter() {
            self.declared_class_req(
                &mut req_ancestors,
                &mut req_ancestors_extends,
                req_implement,
            );
        }

        for use_ in self.child.uses.iter() {
            self.flatten_parent_class_reqs(&mut req_ancestors, &mut req_ancestors_extends, use_);
        }

        if self.child.kind.is_cinterface() {
            for extend in self.child.extends.iter() {
                self.flatten_parent_class_reqs(
                    &mut req_ancestors,
                    &mut req_ancestors_extends,
                    extend,
                );
            }
        } else {
            for implement in self.child.implements.iter() {
                self.flatten_parent_class_reqs(
                    &mut req_ancestors,
                    &mut req_ancestors_extends,
                    implement,
                );
            }
        }

        self.naive_dedup(&mut req_ancestors);
        (req_ancestors, req_ancestors_extends)
    }

    fn get_sealed_whitelist(&self) -> Option<TypeNameIndexSet> {
        self.child
            .user_attributes
            .iter()
            .find(|ua| ua.name.id() == self.special_names.user_attributes.uaSealed)
            .map(|ua| ua.classname_params.iter().copied().collect())
    }

    fn get_deferred_init_members_helper(&self) -> PropNameIndexSet {
        let shallow_props = self
            .child
            .props
            .iter()
            .filter(|prop| prop.xhp_attr.is_none())
            .filter(|prop| !prop.flags.is_lateinit())
            .filter(|prop| prop.flags.needs_init())
            .map(|prop| prop.name.id());

        let extends_props = self
            .child
            .extends
            .iter()
            .filter_map(|extend| extend.unwrap_class_type())
            .filter_map(|(_, pos_id, _)| self.parents.get(&pos_id.id()))
            .flat_map(|ty| ty.deferred_init_members.iter().copied());

        let parent_construct = if self.child.mode == oxidized::file_info::Mode::Mhhi {
            None
        } else {
            if self.child.kind == ClassishKind::Ctrait {
                self.child.req_extends.iter()
            } else {
                self.child.extends.iter()
            }
            .filter_map(|ty| ty.unwrap_class_type())
            .filter_map(|(_, pos_id, _)| self.parents.get(&pos_id.id()))
            .find(|parent| parent.has_concrete_constructor())
            .map(|_| PropName(self.special_names.members.parentConstruct))
        };

        shallow_props
            .chain(extends_props)
            .chain(parent_construct.into_iter())
            .collect()
    }

    /// Return all init-requiring props of the class and its ancestors from the
    /// given shallow class decl and the ancestors' folded decls.
    fn get_deferred_init_members(&self, cstr: &Option<FoldedElement>) -> PropNameIndexSet {
        let has_concrete_cstr = match cstr {
            Some(e) if !e.is_abstract() => true,
            _ => false,
        };
        let has_own_cstr = has_concrete_cstr && self.child.constructor.is_some();
        match self.child.kind {
            ClassishKind::Cclass(cls) if cls.is_abstract() && !has_own_cstr => {
                self.get_deferred_init_members_helper()
            }
            ClassishKind::Ctrait => self.get_deferred_init_members_helper(),
            _ => PropNameIndexSet::default(),
        }
    }

    fn decl_class_impl(mut self) -> Arc<FoldedClass<R>> {
        let inh = Inherited::make(self.child, self.parents);

        let mut props = inh.props;
        self.child
            .props
            .iter()
            .for_each(|sp| self.decl_prop(&mut props, sp));

        let mut static_props = inh.static_props;
        self.child
            .static_props
            .iter()
            .for_each(|ssp| self.decl_static_prop(&mut static_props, ssp));

        let mut methods = inh.methods;
        self.child
            .methods
            .iter()
            .for_each(|sm| self.decl_method(&mut methods, sm));

        let mut static_methods = inh.static_methods;
        self.child
            .static_methods
            .iter()
            .for_each(|sm| self.decl_method(&mut static_methods, sm));

        let mut constructor = inh.constructor;
        self.decl_constructor(&mut constructor);

        let direct_ancestors = (self.child.extends.iter())
            .chain(self.child.implements.iter())
            .chain(self.child.uses.iter());

        let mut ancestors = Default::default();
        for ty in direct_ancestors.rev() {
            self.get_implements(ty, &mut ancestors);
        }

        let mut consts = inh.consts;
        self.child
            .consts
            .iter()
            .for_each(|c| self.decl_class_const(&mut consts, c));
        self.decl_class_class(&mut consts);

        let mut type_consts = inh.type_consts;
        self.child
            .typeconsts
            .iter()
            .for_each(|tc| self.decl_type_const(&mut type_consts, &mut consts, tc));

        let extends = self.get_extends();
        let xhp_attr_deps = self.get_xhp_attr_deps();

        let (req_ancestors, req_ancestors_extends) = self.get_class_requirements();

        let sealed_whitelist = self.get_sealed_whitelist();

        let deferred_init_members = self.get_deferred_init_members(&constructor.elt);

        Arc::new(FoldedClass {
            name: self.child.name.id(),
            pos: self.child.name.pos().clone(),
            kind: self.child.kind,
            is_final: self.child.is_final,
            is_const: self
                .child
                .user_attributes
                .iter()
                .any(|ua| ua.name.id() == self.special_names.user_attributes.uaConst),
            is_internal: self
                .child
                .user_attributes
                .iter()
                .any(|ua| ua.name.id() == self.special_names.user_attributes.uaInternal),
            is_xhp: self.child.is_xhp,
            support_dynamic_type: self.child.support_dynamic_type,
            enum_type: self.child.enum_type.clone(),
            has_xhp_keyword: self.child.has_xhp_keyword,
            module: self.child.module.clone(),
            tparams: self.child.tparams.clone(),
            where_constraints: self.child.where_constraints.clone(),
            substs: inh.substs,
            ancestors,
            props,
            static_props,
            methods,
            static_methods,
            constructor,
            consts,
            type_consts,
            xhp_enum_values: self.child.xhp_enum_values.clone(),
            extends,
            xhp_attr_deps,
            req_ancestors: req_ancestors.into_boxed_slice(),
            req_ancestors_extends,
            sealed_whitelist,
            deferred_init_members,
            decl_errors: self.errors.into_boxed_slice(),
        })
    }
}
