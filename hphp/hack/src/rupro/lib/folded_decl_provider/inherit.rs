// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::subst::{Subst, Substitution};
use crate::alloc::Allocator;
use crate::decl_defs::{
    AbstractTypeconst, Abstraction, ClassConst, ClassConstKind, ClassishKind, DeclTy, FoldedClass,
    FoldedElement, ShallowClass, SubstContext, TypeConst, Typeconst,
};
use crate::reason::Reason;
use pos::{
    ClassConstNameMap, MethodName, MethodNameMap, PropNameMap, TypeConstNameMap, TypeNameMap,
};
use std::collections::hash_map::Entry;
use std::sync::Arc;

// note(sf, 2022-02-03): c.f. hphp/hack/src/decl/decl_inherit.ml

#[derive(Debug)]
pub struct Inherited<R: Reason> {
    // note(sf, 2022-01-27): c.f. `Decl_inherit.inherited`
    pub substs: TypeNameMap<SubstContext<R>>,
    pub props: PropNameMap<FoldedElement>,
    pub static_props: PropNameMap<FoldedElement>,
    pub methods: MethodNameMap<FoldedElement>,
    pub static_methods: MethodNameMap<FoldedElement>,
    pub constructor: Option<FoldedElement>,
    pub consts: ClassConstNameMap<ClassConst<R>>,
    pub type_consts: TypeConstNameMap<TypeConst<R>>,
}

impl<R: Reason> Default for Inherited<R> {
    fn default() -> Self {
        Self {
            substs: Default::default(),
            props: Default::default(),
            static_props: Default::default(),
            methods: Default::default(),
            static_methods: Default::default(),
            constructor: Default::default(),
            consts: Default::default(),
            type_consts: Default::default(),
        }
    }
}

impl<R: Reason> Inherited<R> {
    // Reasons to keep the old signature:
    //   - We don't want to override a concrete method with an
    //     abstract one;
    //   - We don't want to override a method that's actually
    //     implemented by the programmer with one that's "synthetic",
    //     e.g. arising merely from a require-extends declaration in a
    //     trait.
    // When these two considerations conflict, we give precedence to
    // abstractness for determining priority of the method.
    fn should_keep_old_sig(new_sig: &FoldedElement, old_sig: &FoldedElement) -> bool {
        !old_sig.is_abstract() && new_sig.is_abstract()
            || old_sig.is_abstract() == new_sig.is_abstract()
                && !old_sig.is_synthesized()
                && new_sig.is_synthesized()
    }

    fn add_constructor(&mut self, constructor: Option<FoldedElement>) {
        match (constructor.as_ref(), self.constructor.as_ref()) {
            (None, _) => {}
            (Some(other_ctor), Some(self_ctor))
                if Self::should_keep_old_sig(other_ctor, self_ctor) => {}
            (_, _) => self.constructor = constructor,
        }
    }

    fn add_substs(&mut self, other_substs: TypeNameMap<SubstContext<R>>) {
        for (key, new_sc) in other_substs {
            match self.substs.entry(key) {
                Entry::Vacant(e) => {
                    e.insert(new_sc);
                }
                Entry::Occupied(mut e) => {
                    if e.get().from_req_extends && !new_sc.from_req_extends {
                        // If the old substitution context came via required extends
                        // then we want to use the substitutions from the actual
                        // extends instead. e.g.
                        // ```
                        // class Base<+T> {}
                        // trait MyTrait { require extends Base<mixed>; }
                        // class Child extends Base<int> { use MyTrait; }
                        // ```
                        // Here the substitution context `{MyTrait/[T -> mixed]}`
                        // should be overridden by `{Child/[T -> int]}`, because
                        // it's the actual extension of class `Base`.
                        e.insert(new_sc);
                    }
                }
            }
        }
    }

    fn add_method(
        methods: &mut MethodNameMap<FoldedElement>,
        (key, mut fe): (MethodName, FoldedElement),
    ) {
        match methods.entry(key) {
            Entry::Vacant(entry) => {
                // The method didn't exist so far, let's add it.
                entry.insert(fe);
            }
            Entry::Occupied(mut entry) => {
                if !Self::should_keep_old_sig(&fe, entry.get()) {
                    fe.set_is_superfluous_override(false);
                    entry.insert(fe);
                } else {
                    // Otherwise, we *are* overwriting a method
                    // definition. This is OK when a naming
                    // conflict is parent class vs trait (trait
                    // wins!), but not really OK when the naming
                    // conflict is trait vs trait (we rely on HHVM
                    // to catch the error at runtime).
                }
            }
        }
    }

    fn add_methods(&mut self, other_methods: MethodNameMap<FoldedElement>) {
        for (key, fe) in other_methods {
            Self::add_method(&mut self.methods, (key, fe))
        }
    }

    fn add_static_methods(&mut self, other_static_methods: MethodNameMap<FoldedElement>) {
        for (key, fe) in other_static_methods {
            Self::add_method(&mut self.static_methods, (key, fe))
        }
    }

    fn add_props(&mut self, other_props: PropNameMap<FoldedElement>) {
        self.props.extend(other_props)
    }

    fn add_static_props(&mut self, other_static_props: PropNameMap<FoldedElement>) {
        self.static_props.extend(other_static_props)
    }

    fn add_consts(&mut self, other_consts: ClassConstNameMap<ClassConst<R>>) {
        for (name, new_const) in other_consts {
            match self.consts.entry(name) {
                Entry::Vacant(e) => {
                    e.insert(new_const);
                }
                Entry::Occupied(mut e) => {
                    let old_const = e.get();
                    match (
                        new_const.is_synthesized,
                        old_const.is_synthesized,
                        new_const.kind,
                        old_const.kind,
                    ) {
                        (true, false, _, _) => {
                            // Don't replace a constant with a synthesized
                            // constant. This covers the following case:
                            // ```
                            // class HasFoo { abstract const int FOO; }
                            // trait T { require extends Foo; }
                            // class Child extends HasFoo {
                            //   use T;
                            // }
                            // ```
                            // In this case, `Child` still doesn't have a value
                            // for the `FOO` constant.
                        }
                        (_, _, ClassConstKind::CCAbstract(_), ClassConstKind::CCConcrete) => {
                            // Don't replace a concrete constant with an
                            // abstract constant found later in the MRO.
                        }
                        _ => {
                            e.insert(new_const);
                        }
                    }
                }
            }
        }
    }

    fn add_type_consts(&mut self, other_type_consts: TypeConstNameMap<TypeConst<R>>) {
        for (name, mut new_const) in other_type_consts {
            match self.type_consts.entry(name) {
                Entry::Vacant(e) => {
                    // The type constant didn't exist so far, let's add it.
                    e.insert(new_const);
                }
                Entry::Occupied(mut e) => {
                    let old_const = e.get();
                    if new_const.is_enforceable() && !old_const.is_enforceable() {
                        // If some typeconst in some ancestor was enforceable,
                        // then the child class' typeconst will be enforceable
                        // too, even if we didn't take that ancestor typeconst.
                        e.get_mut().enforceable = new_const.enforceable.clone();
                    }
                    let old_const = e.get();
                    match (&old_const.kind, &new_const.kind) {
                        // This covers the following case
                        // ```
                        // interface I1 { abstract const type T; }
                        // interface I2 { const type T = int; }
                        // class C implements I1, I2 {}
                        // ```
                        // Then `C::T == I2::T` since `I2::T `is not abstract
                        (Typeconst::TCConcrete(_), Typeconst::TCAbstract(_)) => {}
                        // This covers the following case
                        // ```
                        // interface I {
                        //   abstract const type T as arraykey;
                        // }
                        //
                        // abstract class A {
                        //   abstract const type T as arraykey = string;
                        // }
                        //
                        // final class C extends A implements I {}
                        // ```
                        // `C::T` must come from `A`, not `I`, as `A`
                        // provides the default that will synthesize into a
                        // concrete type constant in `C`.
                        (
                            Typeconst::TCAbstract(AbstractTypeconst {
                                default: Some(_), ..
                            }),
                            Typeconst::TCAbstract(AbstractTypeconst { default: None, .. }),
                        ) => {}
                        // When a type constant is declared in multiple
                        // parents we need to make a subtle choice of what
                        // type we inherit. For example in:
                        // ```
                        // interface I1 { abstract const type t as Container<int>; }
                        // interface I2 { abstract const type t as KeyedContainer<int, int>; }
                        // abstract class C implements I1, I2 {}
                        // ```
                        // Depending on the order the interfaces are
                        // declared, we may report an error. Since this
                        // could be confusing there is special logic in
                        // `Typing_extends` that checks for this potentially
                        // ambiguous situation and warns the programmer to
                        // explicitly declare `T` in `C`.
                        _ => {
                            if old_const.is_enforceable() && !new_const.is_enforceable() {
                                // If a typeconst we already inherited from some
                                // other ancestor was enforceable, then the one
                                // we inherit here will be enforceable too.
                                new_const.enforceable = old_const.enforceable.clone();
                            }
                            e.insert(new_const);
                        }
                    }
                }
            }
        }
    }

    fn add_inherited(&mut self, other: Self) {
        let Self {
            substs,
            props,
            static_props,
            methods,
            static_methods,
            constructor,
            consts,
            type_consts,
        } = other;
        self.add_substs(substs);
        self.add_props(props);
        self.add_static_props(static_props);
        self.add_methods(methods);
        self.add_static_methods(static_methods);
        self.add_constructor(constructor);
        self.add_consts(consts);
        self.add_type_consts(type_consts);
    }

    fn mark_as_synthesized(&mut self) {
        for ctx in self.substs.values_mut() {
            ctx.from_req_extends = true;
        }
        if let Some(ref mut elt) = self.constructor {
            elt.set_is_synthesized(true);
        }
        for prop in self.props.values_mut() {
            prop.set_is_synthesized(true);
        }
        for static_prop in self.static_props.values_mut() {
            static_prop.set_is_synthesized(true);
        }
        for method in self.methods.values_mut() {
            method.set_is_synthesized(true);
        }
        for static_method in self.static_methods.values_mut() {
            static_method.set_is_synthesized(true);
        }
        for classconst in self.consts.values_mut() {
            classconst.set_is_synthesized(true);
        }

        //TODO typeconsts
    }
}

struct MemberFolder<'a, R: Reason> {
    alloc: &'a Allocator<R>,
    child: &'a ShallowClass<R>,
    parents: &'a TypeNameMap<Arc<FoldedClass<R>>>,
    members: Inherited<R>,
}

impl<'a, R: Reason> MemberFolder<'a, R> {
    fn make_substitution(
        &self,
        cls: &FoldedClass<R>,
        params: &[DeclTy<R>],
    ) -> TypeNameMap<DeclTy<R>> {
        Subst::new(self.alloc, &cls.tparams, params).into()
    }

    // c.f. Decl_inherit.from_class
    fn members_from_class(&self, parent_ty: &DeclTy<R>) -> Inherited<R> {
        if let Some((_, parent_pos_id, parent_tyl)) = parent_ty.unwrap_class_type() {
            if let Some(parent_folded_decl) = self.parents.get(&parent_pos_id.id()) {
                let subst = self.make_substitution(parent_folded_decl, parent_tyl);
                // TODO(hrust): Do we need sharing?
                let mut substs = parent_folded_decl.substs.clone();
                substs.insert(
                    parent_folded_decl.name,
                    SubstContext {
                        subst,
                        class_context: self.child.name.id(),
                        from_req_extends: false,
                    },
                );
                return Inherited {
                    substs,
                    props: parent_folded_decl.props.clone(),
                    static_props: parent_folded_decl.static_props.clone(),
                    methods: parent_folded_decl.methods.clone(),
                    static_methods: parent_folded_decl.static_methods.clone(),
                    constructor: parent_folded_decl.constructor.clone(),
                    consts: parent_folded_decl.consts.clone(),
                    type_consts: parent_folded_decl.type_consts.clone(),
                };
            }
        }
        Default::default()
    }

    fn class_constants_from_class(&self, ty: &DeclTy<R>) -> Inherited<R> {
        if let Some((_, pos_id, tyl)) = ty.unwrap_class_type() {
            if let Some(class) = self.parents.get(&pos_id.id()) {
                let sig = Subst::new(self.alloc, &class.tparams, tyl);
                let subst = Substitution {
                    alloc: self.alloc,
                    subst: &sig,
                };
                let consts: ClassConstNameMap<_> = class
                    .consts
                    .iter()
                    .map(|(name, cc)| (*name, subst.instantiate_class_const(cc)))
                    .collect();
                let type_consts: TypeConstNameMap<_> = class
                    .type_consts
                    .iter()
                    .map(|(name, tc)| (*name, subst.instantiate_type_const(tc)))
                    .collect();
                return Inherited {
                    consts,
                    type_consts,
                    ..Default::default()
                };
            }
        }
        Default::default()
    }

    // This logic deals with importing XHP attributes from an XHP class via the
    // "attribute :foo" syntax.
    // c.f. Decl_inherit.from_class_xhp_attrs_only
    fn xhp_attrs_from_class(&self, ty: &DeclTy<R>) -> Inherited<R> {
        if let Some((_, pos_id, _tyl)) = ty.unwrap_class_type() {
            if let Some(class) = self.parents.get(&pos_id.id()) {
                // Filter out properties that are not XHP attributes.
                return Inherited {
                    props: class
                        .props
                        .iter()
                        .filter(|(_, prop)| prop.get_xhp_attr().is_some())
                        .map(|(name, prop)| (name.clone(), prop.clone()))
                        .collect(),
                    ..Default::default()
                };
            }
        }
        Default::default()
    }

    fn add_from_interface_constants(&mut self) {
        for ty in self.child.req_implements.iter() {
            self.members
                .add_inherited(self.class_constants_from_class(ty))
        }
    }

    fn add_from_xhp_attr_uses(&mut self) {
        for ty in self.child.xhp_attr_uses.iter() {
            self.members.add_inherited(self.xhp_attrs_from_class(ty))
        }
    }

    fn add_from_parents(&mut self) {
        let mut tys: Vec<&DeclTy<R>> = Vec::new();
        match self.child.kind {
            ClassishKind::Cclass(Abstraction::Abstract) => {
                tys.extend(self.child.implements.iter());
                tys.extend(self.child.extends.iter());
            }
            ClassishKind::Ctrait => {
                tys.extend(self.child.implements.iter());
                tys.extend(self.child.extends.iter());
                tys.extend(self.child.req_implements.iter());
            }
            ClassishKind::Cclass(_)
            | ClassishKind::Cinterface
            | ClassishKind::Cenum
            | ClassishKind::CenumClass(_) => {
                tys.extend(self.child.extends.iter());
            }
        };

        // Interfaces implemented, classes extended and interfaces required to
        // be implemented.
        for ty in tys.iter().rev() {
            self.members.add_inherited(self.members_from_class(ty));
        }
    }

    fn add_from_requirements(&mut self) {
        for ty in self.child.req_extends.iter() {
            let mut inherited = self.members_from_class(ty);
            inherited.mark_as_synthesized();
            self.members.add_inherited(inherited);
        }
    }

    fn add_from_traits(&mut self) {
        for ty in self.child.uses.iter() {
            self.members.add_inherited(self.members_from_class(ty));
        }
    }
}

impl<R: Reason> Inherited<R> {
    pub fn make(
        alloc: &Allocator<R>,
        child: &ShallowClass<R>,
        parents: &TypeNameMap<Arc<FoldedClass<R>>>,
    ) -> Self {
        let mut folder = MemberFolder {
            alloc,
            child,
            parents,
            members: Self::default(),
        };
        folder.add_from_parents(); // Members inherited from parents ...
        folder.add_from_requirements();
        folder.add_from_traits(); // ... can be overridden by traits.
        folder.add_from_xhp_attr_uses();
        folder.add_from_interface_constants();

        folder.members
    }
}
