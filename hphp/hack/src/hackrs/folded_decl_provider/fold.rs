// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use eq_modulo_pos::EqModuloPos;
use hash::IndexMap;
use hash::IndexSet;
use oxidized::global_options::GlobalOptions;
use pos::ClassConstName;
use pos::MethodName;
use pos::ModuleName;
use pos::Positioned;
use pos::PropName;
use pos::TypeConstName;
use pos::TypeName;
use special_names as sn;
use ty::decl::folded::Constructor;
use ty::decl::subst::Subst;
use ty::decl::AbstractTypeconst;
use ty::decl::Abstraction;
use ty::decl::CeVisibility;
use ty::decl::ClassConst;
use ty::decl::ClassConstKind;
use ty::decl::ClassEltFlags;
use ty::decl::ClassEltFlagsArgs;
use ty::decl::ClassishKind;
use ty::decl::ConcreteTypeconst;
use ty::decl::ConsistentKind;
use ty::decl::FoldedClass;
use ty::decl::FoldedElement;
use ty::decl::Requirement;
use ty::decl::ShallowClass;
use ty::decl::ShallowClassConst;
use ty::decl::ShallowMethod;
use ty::decl::ShallowProp;
use ty::decl::ShallowTypeconst;
use ty::decl::TaccessType;
use ty::decl::Ty;
use ty::decl::TypeConst;
use ty::decl::Typeconst;
use ty::decl::Visibility;
use ty::decl_error::DeclError;
use ty::reason::Reason;

use super::inherit::Inherited;
use super::Result;
use super::Substitution;

mod decl_enum;

// note(sf, 2022-02-03): c.f. hphp/hack/src/decl/decl_folded_class.ml

#[derive(Debug)]
pub struct DeclFolder<'a, R: Reason> {
    /// Options affecting typechecking behaviors.
    opts: &'a GlobalOptions,
    /// The class whose folded decl we are producing.
    child: &'a ShallowClass<R>,
    /// The folded decls of all (recursive) ancestors of `child`.
    parents: &'a IndexMap<TypeName, Arc<FoldedClass<R>>>,
    /// Hack errors which will be written to `child`'s folded decl.
    errors: Vec<DeclError<R::Pos>>,
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
        child: &'a ShallowClass<R>,
        parents: &'a IndexMap<TypeName, Arc<FoldedClass<R>>>,
        errors: Vec<DeclError<R::Pos>>,
    ) -> Result<Arc<FoldedClass<R>>> {
        let this = Self {
            opts,
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

    fn synthesize_const_defaults(&self, consts: &mut IndexMap<ClassConstName, ClassConst<R>>) {
        for c in consts.values_mut() {
            if c.kind == ClassConstKind::CCAbstract(true) {
                c.kind = ClassConstKind::CCConcrete;
            }
        }
    }

    /// When all type constants have been inherited and declared, this step
    /// synthesizes the defaults of abstract type constants into concrete type
    /// constants.
    fn synthesize_type_const_defaults(
        &self,
        type_consts: &mut IndexMap<TypeConstName, TypeConst<R>>,
        consts: &mut IndexMap<ClassConstName, ClassConst<R>>,
    ) {
        for (name, tc) in type_consts.iter_mut() {
            if let Typeconst::TCAbstract(atc) = &mut tc.kind {
                if let Some(ty) = atc.default.take() {
                    tc.kind = Typeconst::TCConcrete(ConcreteTypeconst { ty });
                    tc.is_concretized = true;
                    if let Some(c) = consts.get_mut(&ClassConstName(name.as_symbol())) {
                        c.kind = ClassConstKind::CCConcrete;
                    }
                }
            }
        }
    }

    /// Every class, interface, and trait implicitly defines a `::class` to allow
    /// accessing its fully qualified name as a string.
    fn decl_class_class(&self, consts: &mut IndexMap<ClassConstName, ClassConst<R>>) {
        // note(sf, 2022-02-08): c.f. Decl_folded_class.class_class_decl
        let pos = self.child.name.pos();
        let name = self.child.name.id();
        let reason = R::class_class(pos.clone(), name);
        let classname_ty = Ty::apply(
            reason.clone(),
            Positioned::new(pos.clone(), *sn::classes::cClassname),
            [Ty::this(reason)].into(),
        );
        let class_const = ClassConst {
            is_synthesized: true,
            kind: ClassConstKind::CCConcrete,
            pos: pos.clone(),
            ty: classname_ty,
            origin: name,
            refs: Box::default(),
        };
        consts.insert(*sn::members::mClass, class_const);
    }

    /// Each concrete type constant `T = τ` implicitly defines a class
    /// constant of the same name `T` having type `TypeStructure<τ>`.
    fn type_const_structure(&self, stc: &ShallowTypeconst<R>) -> ClassConst<R> {
        let pos = stc.name.pos();
        let r = R::witness_from_decl(pos.clone());
        let tsid = Positioned::new(pos.clone(), *sn::fb::cTypeStructure);
        // The type `this`.
        let tthis = Ty::this(r.clone());
        // The type `this::T`.
        let taccess = Ty::access(
            r.clone(),
            TaccessType {
                ty: tthis,
                type_const: stc.name.clone(),
            },
        );
        // The type `TypeStructure<this::T>`.
        let ts_ty = Ty::apply(r, tsid, [taccess].into());
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

    fn maybe_add_supportdyn_bound(&self, p: &R::Pos, kind: &mut Typeconst<R>) {
        if self.opts.tco_everything_sdt {
            if let Typeconst::TCAbstract(AbstractTypeconst {
                as_constraint: as_constraint @ None,
                ..
            }) = kind
            {
                *as_constraint = Some(decl_enforceability::supportdyn_mixed(
                    p.clone(),
                    R::witness_from_decl(p.clone()),
                ));
            }
        }
    }

    fn decl_type_const(
        &self,
        type_consts: &mut IndexMap<TypeConstName, TypeConst<R>>,
        class_consts: &mut IndexMap<ClassConstName, ClassConst<R>>,
        stc: &ShallowTypeconst<R>,
    ) {
        // note(sf, 2022-02-10): c.f. Decl_folded_class.typeconst_fold
        match self.child.kind {
            ClassishKind::Cenum => return,
            ClassishKind::CenumClass(_)
            | ClassishKind::Ctrait
            | ClassishKind::Cinterface
            | ClassishKind::Cclass(_) => {}
        }
        let TypeConstName(name) = stc.name.id();
        let ptc = type_consts.get(stc.name.id_ref());
        let ptc_enforceable = ptc.and_then(|tc| tc.enforceable.as_ref());
        let ptc_reifiable = ptc.and_then(|tc| tc.reifiable.as_ref());
        let mut kind = stc.kind.clone();
        if !stc.is_ctx {
            self.maybe_add_supportdyn_bound(stc.name.pos(), &mut kind);
        }
        let type_const = TypeConst {
            is_synthesized: false,
            name: stc.name.clone(),
            kind,
            origin: self.child.name.id(),
            enforceable: ty::decl::ty::Enforceable(
                stc.enforceable.as_ref().or(ptc_enforceable).cloned(),
            ),
            reifiable: stc.reifiable.as_ref().or(ptc_reifiable).cloned(),
            is_concretized: false,
            is_ctx: stc.is_ctx,
        };
        type_consts.insert(TypeConstName(name), type_const);
        class_consts.insert(ClassConstName(name), self.type_const_structure(stc));
    }

    fn decl_class_const(
        &self,
        consts: &mut IndexMap<ClassConstName, ClassConst<R>>,
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

    fn decl_prop(&self, props: &mut IndexMap<PropName, FoldedElement>, sp: &ShallowProp<R>) {
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
            safe_global_variable: false,
        };
        let elt = FoldedElement {
            origin: self.child.name.id(),
            visibility: vis,
            deprecated: None,
            flags: ClassEltFlags::new(flag_args),
            sort_text: None,
        };
        props.insert(prop, elt);
    }

    fn decl_static_prop(
        &self,
        static_props: &mut IndexMap<PropName, FoldedElement>,
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
            needs_init: false,
            safe_global_variable: prop_flags.is_safe_global_variable(),
        };
        let elt = FoldedElement {
            origin: self.child.name.id(),
            visibility: vis,
            deprecated: None,
            flags: ClassEltFlags::new(flag_args),
            sort_text: None,
        };
        static_props.insert(prop, elt);
    }

    fn decl_method(
        &self,
        methods: &mut IndexMap<MethodName, FoldedElement>,
        sm: &ShallowMethod<R>,
    ) {
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
            safe_global_variable: false,
        };
        let elt = FoldedElement {
            origin: cls,
            visibility: vis,
            deprecated: sm.deprecated,
            flags: ClassEltFlags::new(flag_args),
            sort_text: sm.sort_text.to_owned(),
        };

        methods.insert(meth, elt);
    }

    fn decl_constructor(&self, constructor: &mut Constructor) {
        // Constructors in children of `self.child` must be consistent?
        let consistency = if self.child.is_final {
            ConsistentKind::FinalClass
        } else if (self.child.user_attributes.iter())
            .any(|ua| ua.name.id() == *sn::user_attributes::uaConsistentConstruct)
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
                safe_global_variable: false,
            };
            FoldedElement {
                origin: self.child.name.id(),
                visibility: vis,
                deprecated: sm.deprecated,
                flags: ClassEltFlags::new(flag_args),
                sort_text: sm.sort_text.to_owned(),
            }
        });

        let consistency = ConsistentKind::coalesce(constructor.consistency, consistency);
        if elt.is_none() {
            // Child class doesn't define ctor; just update consistency.
            constructor.consistency = consistency;
        } else {
            // Child constructor exists, replace wholesale.
            *constructor = Constructor::new(elt, consistency)
        }
    }

    fn get_implements(&self, ty: &Ty<R>, ancestors: &mut IndexMap<TypeName, Ty<R>>) {
        let (_, pos_id, tyl) = ty.unwrap_class_type();
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

    // HHVM implicitly adds StringishObject interface for every class/interface/trait
    // with a __toString method; "string" also implements this interface
    pub fn stringish_object_parent(cls: &ShallowClass<R>) -> Option<Ty<R>> {
        if cls.name.id() != *sn::classes::cStringishObject {
            (cls.methods.iter())
                .find(|meth| meth.name.id() == *sn::members::__toString)
                .map(|meth| {
                    Ty::apply(
                        R::hint(meth.name.pos().clone()),
                        Positioned::new(
                            meth.name.pos().clone(),
                            sn::classes::cStringishObject.clone(),
                        ),
                        Box::new([]),
                    )
                })
        } else {
            None
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
        parent_name: TypeName,
    ) {
        match (parent_kind, self.child.kind) {
            // What is allowed
            (ClassishKind::Cclass(_), ClassishKind::Cclass(_))
            | (ClassishKind::Ctrait, ClassishKind::Ctrait)
            | (ClassishKind::Cinterface, ClassishKind::Cinterface)
            | (ClassishKind::CenumClass(_), ClassishKind::CenumClass(_)) => {}
            // enums extend `BuiltinEnum` under the hood
            (ClassishKind::Cclass(k), ClassishKind::Cenum | ClassishKind::CenumClass(_))
                if k.is_abstract() => {}
            // What is disallowed
            _ => self.errors.push(DeclError::WrongExtendKind {
                parent_pos: parent_pos.clone(),
                parent_kind,
                parent_name,
                pos: self.child.name.pos().clone(),
                kind: self.child.kind,
                name: self.child.name.id(),
            }),
        }
    }

    fn check_use_kind(
        &mut self,
        parent_pos: &R::Pos,
        parent_kind: ClassishKind,
        parent_name: TypeName,
        parent_is_module_level_trait: bool,
    ) {
        let child_is_module_level_trait = self
            .child
            .user_attributes
            .iter()
            .any(|ua| ua.name.id() == *sn::user_attributes::uaModuleLevelTrait);
        match (parent_kind, self.child.kind) {
            (ClassishKind::Ctrait, ClassishKind::Ctrait)
                if child_is_module_level_trait && !parent_is_module_level_trait =>
            {
                self.errors.push(DeclError::WrongUseKind {
                    parent_pos: parent_pos.clone(),
                    parent_name,
                    pos: self.child.name.pos().clone(),
                    name: self.child.name.id(),
                })
            }
            _ => {}
        }
    }

    fn add_class_parent_or_trait(
        &mut self,
        pass: Pass,
        extends: &mut IndexSet<TypeName>,
        ty: &Ty<R>,
    ) {
        let (_, pos_id, _) = ty.unwrap_class_type();
        extends.insert(pos_id.id());
        if let Some(cls) = self.parents.get(&pos_id.id()) {
            match pass {
                Pass::Extends => {
                    self.check_extend_kind(pos_id.pos(), cls.kind, cls.name);
                }
                Pass::Traits => {
                    self.check_use_kind(
                        pos_id.pos(),
                        cls.kind,
                        cls.name,
                        cls.is_module_level_trait,
                    );
                }
                Pass::Xhp => {
                    // If we are crawling the xhp attribute deps, need to merge their xhp deps as well
                    // XHP attribute dependencies don't actually pull the trait into the class,
                    // so we need to track them totally separately.
                    extends.extend(cls.xhp_attr_deps.iter().cloned());
                }
            }
            extends.extend(cls.extends.iter().cloned());
        }
    }

    fn get_extends(&mut self) -> IndexSet<TypeName> {
        let mut extends = IndexSet::default();
        for extend in self.child.extends.iter() {
            self.add_class_parent_or_trait(Pass::Extends, &mut extends, extend)
        }
        for use_ in self.child.uses.iter() {
            self.add_class_parent_or_trait(Pass::Traits, &mut extends, use_)
        }
        extends
    }

    fn get_xhp_attr_deps(&mut self) -> IndexSet<TypeName> {
        let mut xhp_attr_deps = IndexSet::default();
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
        req_ancestors_extends: &mut IndexSet<TypeName>,
        parent_ty: &Ty<R>,
    ) {
        let (_, pos_id, parent_params) = parent_ty.unwrap_class_type();
        if let Some(cls) = self.parents.get(&pos_id.id()) {
            let subst = Subst::new(&cls.tparams, parent_params);
            let substitution = Substitution { subst: &subst };
            req_ancestors.extend(
                cls.req_ancestors
                    .iter()
                    .map(|req| substitution.instantiate(&req.ty))
                    .map(|ty| Requirement::new(pos_id.pos().clone(), ty)),
            );
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

    fn flatten_parent_class_class_reqs(
        &self,
        req_class_ancestors: &mut Vec<Requirement<R>>,
        parent_ty: &Ty<R>,
    ) {
        let (_, pos_id, parent_params) = parent_ty.unwrap_class_type();
        if let Some(parent_type) = self.parents.get(&pos_id.id()) {
            let subst = Subst::new(&parent_type.tparams, parent_params);
            let substitution = Substitution { subst: &subst };
            req_class_ancestors.extend(
                (parent_type.req_class_ancestors.iter())
                    .map(|req| substitution.instantiate(&req.ty))
                    .map(|ty| Requirement::new(pos_id.pos().clone(), ty)),
            );
        }
    }

    fn declared_class_req(
        &self,
        req_ancestors: &mut Vec<Requirement<R>>,
        req_ancestors_extends: &mut IndexSet<TypeName>,
        req_ty: &Ty<R>,
    ) {
        let (_, pos_id, _) = req_ty.unwrap_class_type();
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
        let mut seen_reqs: IndexMap<TypeName, Vec<Ty<R>>> = IndexMap::default();
        // Reverse to match the OCaml ordering for building the seen_reqs map
        // (since OCaml uses `rev_filter_map` for perf reasons)
        req_ancestors.reverse();
        req_ancestors.retain(|req_extend| {
            let (_, pos_id, targs) = req_extend.ty.unwrap_class_type();
            if let Some(seen_targs) = seen_reqs.get(&pos_id.id()) {
                if targs.eq_modulo_pos_and_reason(seen_targs) {
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
        });
        // Reverse again to match the OCaml ordering for the returned list
        req_ancestors.reverse();
    }

    fn get_class_requirements(
        &self,
    ) -> (
        Box<[Requirement<R>]>,
        IndexSet<TypeName>,
        Box<[Requirement<R>]>,
    ) {
        let mut req_ancestors = vec![];
        let mut req_ancestors_extends = IndexSet::default();

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

        let mut req_class_ancestors: Vec<_> = (self.child.req_class.iter())
            .map(|req_ty| {
                let (_, pos_id, _) = req_ty.unwrap_class_type();
                Requirement::new(pos_id.pos().clone(), req_ty.clone())
            })
            .collect();

        for ty in self.child.uses.iter() {
            self.flatten_parent_class_class_reqs(&mut req_class_ancestors, ty);
        }

        self.naive_dedup(&mut req_class_ancestors);

        (
            req_ancestors.into_boxed_slice(),
            req_ancestors_extends,
            req_class_ancestors.into_boxed_slice(),
        )
    }

    fn get_sealed_whitelist(&self) -> Option<IndexSet<TypeName>> {
        (self.child.user_attributes.iter())
            .find(|ua| ua.name.id() == *sn::user_attributes::uaSealed)
            .map(|ua| ua.classname_params().iter().copied().collect())
    }

    fn get_deferred_init_members_helper(&self) -> IndexSet<PropName> {
        let shallow_props = (self.child.props.iter())
            .filter(|prop| prop.xhp_attr.is_none())
            .filter(|prop| !prop.flags.is_lateinit())
            .filter(|prop| prop.flags.needs_init())
            .map(|prop| prop.name.id());

        let extends_props = (self.child.extends.iter())
            .map(|extend| extend.unwrap_class_type())
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
            .map(|ty| ty.unwrap_class_type())
            .filter_map(|(_, pos_id, _)| self.parents.get(&pos_id.id()))
            .find(|parent| parent.has_concrete_constructor() && self.child.constructor.is_some())
            .map(|_| *sn::members::parentConstruct)
        };

        shallow_props
            .chain(extends_props)
            .chain(parent_construct.into_iter())
            .collect()
    }

    /// Return all init-requiring props of the class and its ancestors from the
    /// given shallow class decl and the ancestors' folded decls.
    fn get_deferred_init_members(&self, cstr: &Option<FoldedElement>) -> IndexSet<PropName> {
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
            _ => IndexSet::default(),
        }
    }

    fn decl_class_impl(mut self) -> Result<Arc<FoldedClass<R>>> {
        let Inherited {
            substs,
            mut props,
            mut static_props,
            mut methods,
            mut static_methods,
            mut constructor,
            mut consts,
            mut type_consts,
            support_dynamic_type,
        } = Inherited::make(self.opts, self.child, self.parents)?;

        for sp in self.child.props.iter() {
            self.decl_prop(&mut props, sp);
        }
        for sp in self.child.static_props.iter() {
            self.decl_static_prop(&mut static_props, sp);
        }
        for sm in self.child.methods.iter() {
            self.decl_method(&mut methods, sm);
        }
        for sm in self.child.static_methods.iter() {
            self.decl_method(&mut static_methods, sm);
        }

        self.decl_constructor(&mut constructor);

        for c in self.child.consts.iter() {
            self.decl_class_const(&mut consts, c);
        }
        self.decl_class_class(&mut consts);

        for tc in self.child.typeconsts.iter() {
            self.decl_type_const(&mut type_consts, &mut consts, tc);
        }

        if self.child.kind == ClassishKind::Cclass(Abstraction::Concrete)
            || self.child.kind == ClassishKind::CenumClass(Abstraction::Concrete)
        {
            self.synthesize_const_defaults(&mut consts);
            self.synthesize_type_const_defaults(&mut type_consts, &mut consts);
        }

        let stringish_object_opt = DeclFolder::stringish_object_parent(self.child);
        // Order matters - the earlier, the higher precedence its ancestors will have
        let direct_ancestors = (stringish_object_opt.iter())
            .chain(self.child.extends.iter())
            .chain(self.child.implements.iter())
            .chain(self.child.uses.iter());

        let mut ancestors = Default::default();
        for ty in direct_ancestors.rev() {
            self.get_implements(ty, &mut ancestors);
        }

        let extends = self.get_extends();
        let xhp_attr_deps = self.get_xhp_attr_deps();

        let (req_ancestors, req_ancestors_extends, req_class_ancestors) =
            self.get_class_requirements();

        // TODO(T88552052) can make logic more explicit now, enum members appear to
        // only need abstract without default and concrete type consts
        let enum_inner_ty = type_consts
            .get(&*sn::fb::tInner)
            .and_then(|tc| match &tc.kind {
                Typeconst::TCConcrete(tc) => Some(&tc.ty),
                Typeconst::TCAbstract(atc) => atc.default.as_ref(),
            });
        self.rewrite_class_consts_for_enum(enum_inner_ty, &ancestors, &mut consts);

        let sealed_whitelist = self.get_sealed_whitelist();

        let deferred_init_members = self.get_deferred_init_members(&constructor.elt);

        let mut tparams = self.child.tparams.clone();
        decl_enforceability::maybe_add_supportdyn_constraints(
            self.opts,
            Some(self.child),
            self.child.name.pos(),
            &mut tparams,
        );

        let fc = Arc::new(FoldedClass {
            name: self.child.name.id(),
            pos: self.child.name.pos().clone(),
            kind: self.child.kind,
            is_final: self.child.is_final,
            is_const: (self.child.user_attributes.iter())
                .any(|ua| ua.name.id() == *sn::user_attributes::uaConst),
            // Support both attribute and keyword for now, until typechecker changes are made
            is_internal: self.child.is_internal,
            is_xhp: self.child.is_xhp,
            support_dynamic_type: self.opts.tco_implicit_inherit_sdt && support_dynamic_type
                || self.child.support_dynamic_type
                || (self.child.user_attributes.iter())
                    .any(|ua| ua.name.id() == *sn::user_attributes::uaDynamicallyReferenced),
            enum_type: self.child.enum_type.clone(),
            has_xhp_keyword: self.child.has_xhp_keyword,
            module: self.child.module.clone(),
            is_module_level_trait: (self.child.user_attributes.iter())
                .any(|ua| ua.name.id() == *sn::user_attributes::uaModuleLevelTrait),
            tparams,
            where_constraints: self.child.where_constraints.clone(),
            substs,
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
            xhp_marked_empty: self.child.xhp_marked_empty,
            req_ancestors,
            req_ancestors_extends,
            req_class_ancestors,
            sealed_whitelist,
            deferred_init_members,
            decl_errors: self.errors.into_boxed_slice(),
            docs_url: self.child.docs_url.clone(),
        });

        Ok(fc)
    }
}
