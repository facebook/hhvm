// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;

use pos::TypeName;
use ty::decl::subst::Subst;
use ty::decl::ty::ShapeType;
use ty::decl::AbstractTypeconst;
use ty::decl::ClassConst;
use ty::decl::ClassRefinement;
use ty::decl::ConcreteTypeconst;
use ty::decl::FunParam;
use ty::decl::FunType;
use ty::decl::PossiblyEnforcedTy;
use ty::decl::RefinedConst;
use ty::decl::RefinedConstBound;
use ty::decl::ShapeFieldType;
use ty::decl::TaccessType;
use ty::decl::Tparam;
use ty::decl::TrefinementType;
use ty::decl::Ty;
use ty::decl::Ty_;
use ty::decl::TypeConst;
use ty::decl::Typeconst;
use ty::decl::WhereConstraint;
use ty::reason::Reason;

// note(sf, 2022-02-14): c.f. `Decl_subst`, `Decl_instantiate`

#[derive(Debug, Clone)]
pub struct Substitution<'a, R: Reason> {
    pub subst: &'a Subst<R>,
}

impl<'a, R: Reason> Substitution<'a, R> {
    fn merge_hk_type(
        &self,
        orig_r: R,
        orig_var: TypeName,
        ty: &Ty<R>,
        args: impl Iterator<Item = Ty<R>>,
    ) -> Ty<R> {
        let ty_: &Ty_<R> = ty.node();
        let res_ty_ = match ty_ {
            Ty_::Tapply(params) => {
                // We could insist on `existing_args.is_empty()` here
                // unless we want to support partial application.
                let (name, existing_args) = &**params;
                Ty_::Tapply(Box::new((
                    name.clone(),
                    existing_args.iter().cloned().chain(args).collect(),
                )))
            }
            Ty_::Tgeneric(params) => {
                // Same here.
                let (name, ref existing_args) = **params;
                Ty_::Tgeneric(Box::new((
                    name,
                    existing_args.iter().cloned().chain(args).collect(),
                )))
            }
            // We could insist on existing_args = [] here unless we want to
            // support partial application.
            _ => ty_.clone(),
        };
        let r = ty.reason().clone();
        Ty::new(R::instantiate(r, orig_var, orig_r), res_ty_)
    }

    pub fn instantiate(&self, ty: &Ty<R>) -> Ty<R> {
        // PERF: If subst is empty then instantiation is a no-op. We can save a
        // significant amount of CPU by avoiding recursively deconstructing the
        // `ty` data type.
        if self.subst.0.is_empty() {
            return ty.clone();
        }
        let r = ty.reason().clone();
        let ty_: &Ty_<R> = ty.node();
        match ty_ {
            Ty_::Tgeneric(params) => {
                let (x, ref existing_args) = **params;
                let args = existing_args.iter().map(|arg| self.instantiate(arg));
                match self.subst.0.get(&x) {
                    Some(x_ty) => self.merge_hk_type(r, x, x_ty, args),
                    None => Ty::generic(r, x, args.collect()),
                }
            }
            _ => Ty::new(r, self.instantiate_(ty_)),
        }
    }

    fn instantiate_(&self, x: &Ty_<R>) -> Ty_<R> {
        match x {
            Ty_::Tgeneric(_) => panic!("subst.rs: instantiate_: impossible!"),
            // IMPORTANT: We cannot expand `Taccess` during instantiation
            // because this can be called before all type consts have been
            // declared and inherited.
            Ty_::Taccess(ta) => Ty_::Taccess(Box::new(TaccessType {
                ty: self.instantiate(&ta.ty),
                type_const: ta.type_const.clone(),
            })),
            Ty_::TvecOrDict(tys) => Ty_::TvecOrDict(Box::new((
                self.instantiate(&tys.0),
                self.instantiate(&tys.1),
            ))),
            Ty_::Tthis
            | Ty_::Tmixed
            | Ty_::Twildcard
            | Ty_::Tdynamic
            | Ty_::Tnonnull
            | Ty_::Tany
            | Ty_::Tprim(_) => x.clone(),
            Ty_::Ttuple(tys) => Ty_::Ttuple(
                tys.iter()
                    .map(|t| self.instantiate(t))
                    .collect::<Box<[_]>>(),
            ),
            Ty_::Tunion(tys) => Ty_::Tunion(
                tys.iter()
                    .map(|t| self.instantiate(t))
                    .collect::<Box<[_]>>(),
            ),
            Ty_::Tintersection(tys) => Ty_::Tintersection(
                tys.iter()
                    .map(|t| self.instantiate(t))
                    .collect::<Box<[_]>>(),
            ),
            Ty_::Toption(ty) => {
                let ty = self.instantiate(ty);
                // We want to avoid double option: `??T`.
                match ty.node() as &Ty_<R> {
                    ty_node @ Ty_::Toption(_) => ty_node.clone(),
                    _ => Ty_::Toption(ty),
                }
            }
            Ty_::Tlike(ty) => Ty_::Tlike(self.instantiate(ty)),
            Ty_::Tfun(ft) => {
                let tparams = &ft.tparams;
                let outer_subst = self;
                let mut subst = self.subst.clone();
                for tp in tparams.iter() {
                    subst.0.remove(tp.name.id_ref());
                }
                let subst = Substitution { subst: &subst };
                let params = ft
                    .params
                    .iter()
                    .map(|fp| FunParam {
                        ty: subst.instantiate_possibly_enforced_ty(&fp.ty),
                        pos: fp.pos.clone(),
                        name: fp.name,
                        flags: fp.flags,
                        def_value: fp.def_value.clone(),
                    })
                    .collect::<Box<[_]>>();
                let ret = subst.instantiate_possibly_enforced_ty(&ft.ret);
                let tparams = tparams
                    .iter()
                    .map(|tp| Tparam {
                        constraints: tp
                            .constraints
                            .iter()
                            .map(|(ck, ty)| (*ck, subst.instantiate(ty)))
                            .collect::<Box<[_]>>(),
                        variance: tp.variance,
                        name: tp.name.clone(),
                        tparams: tp.tparams.clone(),
                        reified: tp.reified,
                        user_attributes: tp.user_attributes.clone(),
                    })
                    .collect::<Box<[_]>>();
                let where_constraints = ft
                    .where_constraints
                    .iter()
                    .map(|WhereConstraint(ty1, ck, ty2)| {
                        WhereConstraint(subst.instantiate(ty1), *ck, outer_subst.instantiate(ty2))
                    })
                    .collect::<Box<[_]>>();
                Ty_::Tfun(Box::new(FunType {
                    params,
                    ret,
                    tparams,
                    where_constraints,
                    flags: ft.flags,
                    implicit_params: ft.implicit_params.clone(),
                    cross_package: ft.cross_package.clone(),
                }))
            }
            Ty_::Tapply(params) => {
                let (name, tys) = &**params;
                Ty_::Tapply(Box::new((
                    name.clone(),
                    tys.iter()
                        .map(|ty| self.instantiate(ty))
                        .collect::<Box<[_]>>(),
                )))
            }
            Ty_::Tshape(params) => {
                let ShapeType(ref shape_kind, ref fdm) = **params;
                let shape_kind = self.instantiate(shape_kind);
                let fdm = fdm
                    .iter()
                    .map(|(f, sft)| {
                        (
                            *f,
                            ShapeFieldType {
                                field_name_pos: sft.field_name_pos.clone(),
                                ty: self.instantiate(&sft.ty),
                                optional: sft.optional,
                            },
                        )
                    })
                    .collect::<BTreeMap<_, _>>();
                Ty_::Tshape(Box::new(ShapeType(shape_kind, fdm)))
            }
            Ty_::Trefinement(tr) => Ty_::Trefinement(Box::new(TrefinementType {
                ty: self.instantiate(&tr.ty),
                refinement: ClassRefinement {
                    consts: (tr.refinement.consts.iter())
                        .map(|(k, v)| (*k, self.instantiate_class_type_refinement(v)))
                        .collect(),
                },
            })),
        }
    }

    fn instantiate_class_type_refinement(&self, rc: &RefinedConst<Ty<R>>) -> RefinedConst<Ty<R>> {
        use RefinedConstBound::*;
        let bound = match &rc.bound {
            Exact(ty) => Exact(self.instantiate(ty)),
            Loose(bounds) => Loose(ty::decl::RefinedConstBounds {
                lower: bounds.lower.iter().map(|ty| self.instantiate(ty)).collect(),
                upper: bounds.upper.iter().map(|ty| self.instantiate(ty)).collect(),
            }),
        };
        RefinedConst {
            bound,
            is_ctx: rc.is_ctx,
        }
    }

    fn instantiate_possibly_enforced_ty(
        &self,
        et: &PossiblyEnforcedTy<Ty<R>>,
    ) -> PossiblyEnforcedTy<Ty<R>> {
        PossiblyEnforcedTy {
            ty: self.instantiate(&et.ty),
            enforced: et.enforced,
        }
    }

    pub fn instantiate_class_const(&self, cc: &ClassConst<R>) -> ClassConst<R> {
        ClassConst {
            is_synthesized: cc.is_synthesized,
            kind: cc.kind,
            pos: cc.pos.clone(),
            ty: self.instantiate(&cc.ty),
            origin: cc.origin,
            refs: cc.refs.clone(),
        }
    }

    fn instantiate_type_const_kind(&self, kind: &Typeconst<R>) -> Typeconst<R> {
        match kind {
            Typeconst::TCAbstract(k) => Typeconst::TCAbstract(AbstractTypeconst {
                as_constraint: k.as_constraint.as_ref().map(|ty| self.instantiate(ty)),
                super_constraint: k.super_constraint.as_ref().map(|ty| self.instantiate(ty)),
                default: k.default.as_ref().map(|ty| self.instantiate(ty)),
            }),
            Typeconst::TCConcrete(k) => Typeconst::TCConcrete(ConcreteTypeconst {
                ty: self.instantiate(&k.ty),
            }),
        }
    }

    pub fn instantiate_type_const(&self, tc: &TypeConst<R>) -> TypeConst<R> {
        TypeConst {
            is_synthesized: tc.is_synthesized,
            name: tc.name.clone(),
            kind: self.instantiate_type_const_kind(&tc.kind),
            origin: tc.origin,
            enforceable: tc.enforceable.clone(),
            reifiable: tc.reifiable.clone(),
            is_concretized: tc.is_concretized,
            is_ctx: tc.is_ctx,
        }
    }
}
