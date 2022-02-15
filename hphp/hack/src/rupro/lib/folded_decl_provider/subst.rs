// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::alloc::Allocator;
use crate::decl_defs::{
    DeclTy, DeclTy_, FunParam, FunType, PossiblyEnforcedTy, ShapeFieldType, TaccessType, Tparam,
    WhereConstraint,
};
use crate::reason::{Reason, ReasonImpl};
use pos::{TypeName, TypeNameMap};
use std::collections::BTreeMap;

// note(sf, 2022-02-14): c.f. `Decl_subst`, `Decl_instantiate`

/// Maps type names to types with which to replace them.
#[derive(Debug, Clone)]
pub struct Subst<R: Reason>(TypeNameMap<DeclTy<R>>);

impl<R: Reason> From<TypeNameMap<DeclTy<R>>> for Subst<R> {
    fn from(map: TypeNameMap<DeclTy<R>>) -> Self {
        Self(map)
    }
}

impl<R: Reason> From<Subst<R>> for TypeNameMap<DeclTy<R>> {
    fn from(subst: Subst<R>) -> Self {
        subst.0
    }
}

/// Any meaningful substitution operation requires an allocator in addition to
/// the substitution map itself so it's convenient to make that accessible
/// through `Self`.
#[derive(Debug, Clone)]
pub struct Substitution<'a, R: Reason> {
    pub alloc: &'a Allocator<R>,
    pub subst: &'a Subst<R>,
}

impl<R: Reason> Subst<R> {
    pub fn new(
        alloc: &Allocator<R>,
        tparams: &[Tparam<R, DeclTy<R>>],
        targs: &[DeclTy<R>],
    ) -> Self {
        // If there are fewer type arguments than type parameters, we'll have
        // emitted an error elsewhere. We bind missing types to `Tany` (rather
        // than `Terr`) here to keep parity with the OCaml implementation, which
        // produces `Tany` because of a now-dead feature called "silent_mode".
        let targs = targs
            .iter()
            .cloned()
            .chain(std::iter::repeat(alloc.decl_ty(R::none(), DeclTy_::DTany)));
        Self(
            tparams
                .iter()
                .map(|tparam| tparam.name.id())
                .zip(targs)
                .collect(),
        )
    }
}

impl<'a, R: Reason> Substitution<'a, R> {
    fn merge_hk_type(
        &self,
        orig_r: &R,
        orig_var: TypeName,
        ty: &DeclTy<R>,
        args: impl Iterator<Item = DeclTy<R>>,
    ) -> DeclTy<R> {
        let r = ty.reason();
        let ty_: &DeclTy_<R> = ty.node();
        let res_ty_ = match ty_ {
            DeclTy_::DTapply(params) => {
                // We could insist on `existing_args.is_empty()` here
                // unless we want to support partial application.
                let (name, existing_args) = &**params;
                DeclTy_::DTapply(Box::new((
                    name.clone(),
                    existing_args
                        .iter()
                        .cloned()
                        .chain(args)
                        .collect::<Box<[_]>>(),
                )))
            }
            DeclTy_::DTgeneric(params) => {
                // Same here.
                let (name, ref existing_args) = **params;
                DeclTy_::DTgeneric(Box::new((
                    name,
                    existing_args
                        .iter()
                        .cloned()
                        .chain(args)
                        .collect::<Box<[_]>>(),
                )))
            }
            _ => {
                // We could insist on existing_args = [] here unless we want to
                // support partial application.
                ty_.clone()
            }
        };
        self.alloc.decl_ty(
            R::mk(|| ReasonImpl::Rinstantiate(r.clone(), orig_var, orig_r.clone())),
            res_ty_,
        )
    }

    pub fn instantiate(&self, ty: &DeclTy<R>) -> DeclTy<R> {
        // PERF: If subst is empty then instantiation is a no-op. We can save a
        // significant amount of CPU by avoiding recursively deconstructing the
        // `ty` data type.
        if self.subst.0.is_empty() {
            return ty.clone();
        }
        let r = ty.reason();
        let ty_: &DeclTy_<R> = ty.node();
        match ty_ {
            DeclTy_::DTgeneric(params) => {
                let (x, ref existing_args) = **params;
                let args = existing_args.iter().map(|arg| self.instantiate(arg));
                match self.subst.0.get(&x) {
                    Some(x_ty) => self.merge_hk_type(r, x, x_ty, args),
                    None => {
                        let args = args.collect::<Box<[_]>>();
                        self.alloc
                            .decl_ty(r.clone(), DeclTy_::DTgeneric(Box::new((x, args))))
                    }
                }
            }
            _ => self.alloc.decl_ty(r.clone(), self.instantiate_(ty_)),
        }
    }

    fn instantiate_(&self, x: &DeclTy_<R>) -> DeclTy_<R> {
        match x {
            DeclTy_::DTgeneric(_) => panic!("subst.rs: instantiate_: impossible!"),
            // IMPORTANT: We cannot expand `DTaccess` during instantiation
            // because this can be called before all type consts have been
            // declared and inherited.
            DeclTy_::DTaccess(ta) => DeclTy_::DTaccess(Box::new(TaccessType {
                ty: self.instantiate(&ta.ty),
                type_const: ta.type_const.clone(),
            })),
            DeclTy_::DTvecOrDict(tys) => DeclTy_::DTvecOrDict(Box::new((
                self.instantiate(&tys.0),
                self.instantiate(&tys.1),
            ))),
            DeclTy_::DTthis
            | DeclTy_::DTvar(_)
            | DeclTy_::DTmixed
            | DeclTy_::DTdynamic
            | DeclTy_::DTnonnull
            | DeclTy_::DTany
            | DeclTy_::DTerr
            | DeclTy_::DTprim(_) => x.clone(),
            DeclTy_::DTtuple(tys) => DeclTy_::DTtuple(
                tys.iter()
                    .map(|t| self.instantiate(t))
                    .collect::<Box<[_]>>(),
            ),
            DeclTy_::DTunion(tys) => DeclTy_::DTunion(
                tys.iter()
                    .map(|t| self.instantiate(t))
                    .collect::<Box<[_]>>(),
            ),
            DeclTy_::DTintersection(tys) => DeclTy_::DTintersection(
                tys.iter()
                    .map(|t| self.instantiate(t))
                    .collect::<Box<[_]>>(),
            ),
            DeclTy_::DToption(ty) => {
                let ty = self.instantiate(ty);
                // We want to avoid double option: `??T`.
                match ty.node() as &DeclTy_<R> {
                    ty_node @ DeclTy_::DToption(_) => ty_node.clone(),
                    _ => DeclTy_::DToption(ty),
                }
            }
            DeclTy_::DTlike(ty) => DeclTy_::DTlike(self.instantiate(ty)),
            DeclTy_::DTfun(ft) => {
                let tparams = &ft.tparams;
                let outer_subst = self;
                let mut subst = self.subst.clone();
                for tp in tparams.iter() {
                    subst.0.remove(tp.name.id_ref());
                }
                let subst = Substitution {
                    alloc: self.alloc,
                    subst: &subst,
                };
                let params = ft
                    .params
                    .iter()
                    .map(|fp| FunParam {
                        ty: subst.instantiate_possibly_enforced_ty(&fp.ty),
                        pos: fp.pos.clone(),
                        name: fp.name,
                        flags: fp.flags,
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
                DeclTy_::DTfun(Box::new(FunType {
                    params,
                    ret,
                    tparams,
                    where_constraints,
                    flags: ft.flags,
                    implicit_params: ft.implicit_params.clone(),
                    ifc_decl: ft.ifc_decl.clone(),
                }))
            }
            DeclTy_::DTapply(params) => {
                let (name, tys) = &**params;
                DeclTy_::DTapply(Box::new((
                    name.clone(),
                    tys.iter()
                        .map(|ty| self.instantiate(ty))
                        .collect::<Box<[_]>>(),
                )))
            }
            DeclTy_::DTshape(params) => {
                let (shape_kind, ref fdm) = **params;
                let fdm = fdm
                    .iter()
                    .map(|(f, sft)| {
                        (
                            *f,
                            ShapeFieldType {
                                ty: self.instantiate(&sft.ty),
                                optional: sft.optional,
                            },
                        )
                    })
                    .collect::<BTreeMap<_, _>>();
                DeclTy_::DTshape(Box::new((shape_kind, fdm)))
            }
        }
    }

    fn instantiate_possibly_enforced_ty(
        &self,
        et: &PossiblyEnforcedTy<DeclTy<R>>,
    ) -> PossiblyEnforcedTy<DeclTy<R>> {
        PossiblyEnforcedTy {
            ty: self.instantiate(&et.ty),
            enforced: et.enforced,
        }
    }
}
