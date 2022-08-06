// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::typing_defs_flags::FunTypeFlags;
use pos::Positioned;
use pos::SymbolMap;
use pos::TypeName;
use ty::decl;
use ty::local;
use ty::local::Exact;
use ty::local::FunParam;
use ty::local::FunType;
use ty::reason::Reason;

use crate::tast;
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;
use crate::typing_decl_provider::Class;
use crate::typing_decl_provider::TypeDecl;

/// Localization environment, controlling localization.
///
/// On OCaml, this  called `expand_env`.
#[derive(Debug, Clone)]
pub struct LocalizeEnv<R: Reason> {
    substs: SymbolMap<local::Ty<R>>,
}

impl<R: Reason> LocalizeEnv<R> {
    /// Localize a type without substitutions for generic parameters or `this`.
    pub fn no_subst() -> Self {
        LocalizeEnv {
            substs: Default::default(),
        }
    }

    /// A localize environment from the given instantiated class.
    pub fn with_class_subst(cls: &dyn Class<R>, targs: &[local::Ty<R>]) -> Self {
        let mut s = Self::no_subst();
        s.add_explicit_ty_substs(cls.get_tparams(), targs);
        s
    }

    /// Get the substitution for a certain type name.
    fn get_subst(&self, x: &TypeName) -> Option<local::Ty<R>> {
        self.substs.get(&x.0).cloned()
    }

    /// Add substitutions using a list of tparams and explicit targs.
    fn add_explicit_targs_substs(
        &mut self,
        tparams: &[decl::Tparam<R, decl::Ty<R>>],
        targs: &[tast::Targ<R>],
    ) {
        rupro_todo_assert!(tparams.len() == targs.len(), AST);
        for (param, targ) in tparams.iter().zip(targs.iter()) {
            let n = param.name.id().clone();
            let ty = targ.0.clone();
            self.substs.insert(n.0, ty);
        }
    }

    /// Add substitutions using a list of tparams and corresponding types.
    fn add_explicit_ty_substs(
        &mut self,
        tparams: &[decl::Tparam<R, decl::Ty<R>>],
        targs: &[local::Ty<R>],
    ) {
        rupro_todo_assert!(tparams.len() == targs.len(), AST);
        for (param, ty) in tparams.iter().zip(targs.iter()) {
            let n = param.name.id().clone();
            self.substs.insert(n.0, ty.clone());
        }
    }
}

/// Type localization is a "simple" conversion from `decl::Ty` to `Ty`.
///
/// Note that this sometimes requires non-trivial operations, specifically:
///
///  - expanding newtypes/types
///  - resolving the `this` type
///  - instantiating generics (TODO(hverr): do we want to extract this?)
///  - ...
impl<R: Reason> Infer<R> for decl::Ty<R> {
    type Params = LocalizeEnv<R>;
    type Typed = local::Ty<R>;

    fn infer(&self, env: &mut TEnv<R>, localize_env: LocalizeEnv<R>) -> Result<local::Ty<R>> {
        localize(env, &localize_env, self.clone())
    }
}

#[derive(Debug, Clone)]
pub struct LocalizeFunTypeParams<R: Reason> {
    /// Localization environment.
    pub localize_env: LocalizeEnv<R>,

    /// The types used to instantiate the type arguments.
    pub explicit_targs: Vec<tast::Targ<R>>,
}

impl<R: Reason> Infer<R> for decl::FunType<R, decl::Ty<R>> {
    type Params = LocalizeFunTypeParams<R>;
    type Typed = local::FunType<R>;

    fn infer(
        &self,
        env: &mut TEnv<R>,
        params: LocalizeFunTypeParams<R>,
    ) -> Result<local::FunType<R>> {
        localize_ft(env, &params, self)
    }
}

fn localize<R: Reason>(
    env: &mut TEnv<R>,
    localize_env: &LocalizeEnv<R>,
    ty: decl::Ty<R>,
) -> Result<local::Ty<R>> {
    use decl::Ty_::*;
    let r = ty.reason().clone();
    let res = match &**ty.node() {
        Tprim(p) => local::Ty::prim(r, *p),
        Tapply(box (pos_id, tyl)) => localize_tapply(env, localize_env, r, pos_id.clone(), tyl)?,
        Tfun(box ft) => local::Ty::fun(
            r,
            localize_ft(
                env,
                &LocalizeFunTypeParams {
                    explicit_targs: vec![],
                    localize_env: localize_env.clone(),
                },
                ft,
            )?,
        ),
        Tgeneric(box (tparam, hl)) => {
            rupro_todo_assert!(hl.is_empty(), HKD);
            match localize_env.get_subst(tparam) {
                Some(x_ty) => {
                    let x_ty = env.resolve_ty(&x_ty);
                    let r_inst = R::instantiate(x_ty.reason().clone(), tparam.clone(), r);
                    rupro_todo_assert!(hl.is_empty(), HKD);
                    local::Ty::new(r_inst, (&*x_ty).clone())
                }
                None => local::Ty::generic(r, tparam.clone(), vec![]),
            }
        }
        t => rupro_todo!(AST, "{:?}", t),
    };
    Ok(res)
}

fn localize_tapply<R: Reason>(
    env: &mut TEnv<R>,
    localize_env: &LocalizeEnv<R>,
    r: R,
    sid: Positioned<TypeName, R::Pos>,
    ty_args: &[decl::Ty<R>],
) -> Result<local::Ty<R>> {
    let class_info = env.decls().get_type(sid.id())?;
    let class_info = match &class_info {
        Some(TypeDecl::Class(cls)) => Some(cls.as_ref()),
        Some(TypeDecl::Typedef(..)) | None => None,
    };
    localize_class_instantiation(env, localize_env, r, sid, ty_args, class_info)
}

fn localize_class_instantiation<R: Reason>(
    env: &mut TEnv<R>,
    localize_env: &LocalizeEnv<R>,
    r: R,
    sid: Positioned<TypeName, R::Pos>,
    ty_args: &[decl::Ty<R>],
    class_info: Option<&dyn Class<R>>,
) -> Result<local::Ty<R>> {
    use local::Ty_::*;
    let res = match class_info {
        None => rupro_todo!(Localization),
        Some(class_info) => {
            rupro_todo_assert!(class_info.get_enum_type().is_none(), AST);
            let ty_args = ty_args.infer(env, localize_env.clone())?;
            local::Ty::new(r, Tclass(sid, Exact::Nonexact, ty_args))
        }
    };
    Ok(res)
}

fn localize_ft<R: Reason>(
    env: &mut TEnv<R>,
    localize_params: &LocalizeFunTypeParams<R>,
    ft: &decl::FunType<R, decl::Ty<R>>,
) -> Result<local::FunType<R>> {
    let mut localize_params = localize_params.clone();
    localize_params
        .localize_env
        .add_explicit_targs_substs(&ft.tparams, &localize_params.explicit_targs);

    let tparams = ft.tparams.infer(env, ())?.into_boxed_slice();
    let params: Vec<_> = ft
        .params
        .iter()
        .map(|fp| {
            let ty =
                localize_possibly_enforced_ty(env, &localize_params.localize_env, fp.ty.clone())?;
            let fp = FunParam {
                pos: fp.pos.clone(),
                name: fp.name,
                ty,
            };
            Ok(fp)
        })
        .collect::<Result<_>>()?;
    let ret = localize_possibly_enforced_ty(env, &localize_params.localize_env, ft.ret.clone())?;
    let flags = ft.flags.clone() | FunTypeFlags::INSTANTIATED_TARGS;
    let ft = FunType {
        tparams,
        params,
        ret,
        flags,
    };
    Ok(ft)
}

fn localize_possibly_enforced_ty<R: Reason>(
    env: &mut TEnv<R>,
    localize_env: &LocalizeEnv<R>,
    ty: decl::PossiblyEnforcedTy<decl::Ty<R>>,
) -> Result<local::Ty<R>> {
    localize(env, localize_env, ty.ty)
}
