// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use pos::ClassConstNameIndexMap;
use pos::Positioned;
use pos::TypeNameIndexMap;
use special_names as sn;
use ty::decl::folded::ClassConst;
use ty::decl::Prim;
use ty::decl::Ty;
use ty::decl::Ty_;
use ty::reason::Reason;

use super::DeclFolder;

struct EnumKind<R: Reason> {
    /// Underlying type of the enum, e.g. int or string. For subclasses of
    /// `Enum`, this is the type parameter of the Enum. For enum classes, this
    /// is `HH\MemberOf<E, I>`.
    // NB(jakebailey, 2022-03-11): `base` is copied from OCaml but not used here.
    // base: Ty<R>,

    /// Type containing the enum name.
    /// For subclasses of Enum, this is also the type parameter of Enum.
    ty: Ty<R>,

    /// Reflects what's after the `as` keyword in the enum definition.
    // NB(jakebailey, 2022-03-11): `constraint` is copied from OCaml but not used here.
    // constraint: Option<Ty<R>>,

    /// For enum classes, this is the raw interface I, as provided by the user.
    interface: Option<Ty<R>>,
}

impl<'a, R: Reason> DeclFolder<'a, R> {
    /// Figures out if `self.child` needs to be treated like an enum.
    fn enum_kind(
        &self,
        inner_ty: Option<&Ty<R>>,
        ancestors: &TypeNameIndexMap<Ty<R>>,
    ) -> Option<EnumKind<R>> {
        let is_enum_class = matches!(self.child.kind, ty::decl::ty::ClassishKind::CenumClass(..));
        match &self.child.enum_type {
            None => {
                let enum_ty = match ancestors.get(&*sn::fb::cEnum) {
                    None => return None,
                    Some(ty) => ty,
                };
                match enum_ty.unwrap_class_type() {
                    (_, name, [ty_exp]) if name.id() == *sn::fb::cEnum => Some(EnumKind {
                        // base: ty_exp.clone(),
                        ty: ty_exp.clone(),
                        // constraint: None,
                        interface: None,
                    }),
                    (_, name, _) if name.id() == *sn::fb::cEnum => {
                        // The fallback if the class does not declare `TInner` (i.e.
                        // it is abstract) is to use `this::TInner`
                        let r = || enum_ty.reason().clone();
                        let ty_exp = match inner_ty {
                            Some(ty) => ty.clone(),
                            None => Ty::access(
                                r(),
                                ty::decl::TaccessType {
                                    ty: Ty::this(r()),
                                    type_const: Positioned::new(
                                        enum_ty.pos().clone(),
                                        *sn::fb::tInner,
                                    ),
                                },
                            ),
                        };
                        Some(EnumKind {
                            // base: ty_exp.clone(),
                            ty: ty_exp,
                            // constraint: None,
                            interface: None,
                        })
                    }
                    _ => None,
                }
            }
            Some(enum_type) => {
                let reason = enum_type.base.reason();
                let pos = reason.pos();
                let enum_ty = Ty::apply(reason.clone(), self.child.name.clone(), [].into());
                let (te_base, te_interface) = if is_enum_class {
                    let te_interface = enum_type.base.clone();
                    // TODO(T77095784) make a new reason !
                    let te_base = Ty::apply(
                        reason.clone(),
                        Positioned::new(pos.clone(), *sn::classes::cMemberOf),
                        [enum_ty, enum_type.base.clone()].into(),
                    );
                    (te_base, Some(te_interface))
                } else {
                    (enum_type.base.clone(), None)
                };
                Some(EnumKind {
                    ty: Ty::apply(te_base.reason().clone(), self.child.name.clone(), [].into()),
                    // base: te_base,
                    // constraint: enum_type.constraint.clone(),
                    interface: te_interface,
                })
            }
        }
    }

    /// If `self.child` is an Enum, we give all of the constants in the class
    /// the type of the Enum. We don't do this for `Enum<mixed>` and
    /// `Enum<arraykey>`, since that could *lose* type information.
    pub fn rewrite_class_consts_for_enum(
        &self,
        inner_ty: Option<&Ty<R>>,
        ancestors: &TypeNameIndexMap<Ty<R>>,
        consts: &mut ClassConstNameIndexMap<ClassConst<R>>,
    ) {
        let EnumKind {
            // base: _,
            ty,
            // constraint: _,
            interface,
        } = match self.enum_kind(inner_ty, ancestors) {
            None => return,
            Some(kind) => kind,
        };

        // Don't rewrite enum classes.
        if interface.is_some() {
            return;
        }

        // Don't rewrite `Enum<mixed>` or `Enum<arraykey>`.
        if matches!(ty.node_ref(), Ty_::Tmixed | Ty_::Tprim(Prim::Tarraykey)) {
            return;
        }

        // A special constant called "class" gets added, and we don't
        // want to rewrite its type.
        // Also for enum class, the type is set in the lowerer.
        for (&name, c) in consts.iter_mut() {
            if name != *sn::members::mClass {
                c.ty = ty.clone();
            }
        }
    }
}
