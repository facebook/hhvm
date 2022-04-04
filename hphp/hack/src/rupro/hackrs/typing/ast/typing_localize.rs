// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::typing::ast::typing_trait::TC;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;
use crate::typing_decl_provider::{Class, TypeDecl};
use pos::{Positioned, TypeName};
use ty::decl;
use ty::local::{self, Exact, FunParam, FunType};
use ty::reason::Reason;

/// Localization environment, controlling localization.
///
/// On OCaml, this  called `expand_env`.
pub struct LocalizeEnv;

impl LocalizeEnv {
    /// Localize a type without substitutions for generic parameters or `this`.
    pub fn no_subst() -> Self {
        LocalizeEnv
    }
}

/// Type localization is a "simple" conversion from `decl::DeclTy` to `Ty`.
///
/// Note that this sometimes requires non-trivial operations, specifically:
///
///  - expanding newtypes/types
///  - resolving the `this` type
///  - instantiating generics (TODO(hverr): do we want to extract this?)
///  - ...
impl<R: Reason> TC<R> for decl::DeclTy<R> {
    type Params = LocalizeEnv;
    type Typed = local::Ty<R>;

    fn infer(&self, env: &TEnv<R>, localize_env: LocalizeEnv) -> Result<local::Ty<R>> {
        localize(env, &localize_env, self.clone())
    }
}

fn localize<R: Reason>(
    env: &TEnv<R>,
    localize_env: &LocalizeEnv,
    ty: decl::DeclTy<R>,
) -> Result<local::Ty<R>> {
    use decl::DeclTy_::*;
    let r = ty.reason().clone();
    let res = match &**ty.node() {
        DTprim(p) => local::Ty::prim(r, *p),
        DTapply(box (pos_id, tyl)) => localize_tapply(env, localize_env, r, pos_id.clone(), tyl)?,
        DTfun(box ft) => localize_ft(env, localize_env, r, ft)?,
        t => rupro_todo!(AST, "{:?}", t),
    };
    Ok(res)
}

fn localize_tapply<R: Reason>(
    env: &TEnv<R>,
    localize_env: &LocalizeEnv,
    r: R,
    sid: Positioned<TypeName, R::Pos>,
    ty_args: &[decl::DeclTy<R>],
) -> Result<local::Ty<R>> {
    let class_info = env.decls().get_type(sid.id())?;
    let class_info = match &class_info {
        Some(TypeDecl::Class(cls)) => Some(cls.as_ref()),
        Some(TypeDecl::Typedef(..)) | None => None,
    };
    localize_class_instantiation(env, localize_env, r, sid, ty_args, class_info)
}

fn localize_class_instantiation<R: Reason>(
    _env: &TEnv<R>,
    _localize_env: &LocalizeEnv,
    r: R,
    sid: Positioned<TypeName, R::Pos>,
    ty_args: &[decl::DeclTy<R>],
    class_info: Option<&dyn Class<R>>,
) -> Result<local::Ty<R>> {
    use local::Ty_::*;
    let res = match class_info {
        None => rupro_todo!(Localization),
        Some(class_info) => {
            rupro_todo_assert!(class_info.get_enum_type().is_none(), AST);
            rupro_todo_assert!(class_info.get_tparams().is_empty(), AST);
            rupro_todo_assert!(ty_args.is_empty(), AST);
            local::Ty::new(r, Tclass(sid, Exact::Nonexact, vec![]))
        }
    };
    Ok(res)
}

fn localize_ft<R: Reason>(
    env: &TEnv<R>,
    localize_env: &LocalizeEnv,
    r: R,
    ft: &decl::FunType<R, decl::DeclTy<R>>,
) -> Result<local::Ty<R>> {
    rupro_todo_assert!(ft.params.is_empty(), AST);
    let params: Vec<_> = ft
        .params
        .iter()
        .map(|fp| {
            let ty = localize_possibly_enforced_ty(env, localize_env, fp.ty.clone())?;
            let fp = FunParam {
                pos: fp.pos.clone(),
                name: fp.name,
                ty,
            };
            Ok(fp)
        })
        .collect::<Result<_>>()?;
    let ret = localize_possibly_enforced_ty(env, localize_env, ft.ret.clone())?;
    let ft = FunType { params, ret };
    let ty = local::Ty::fun(r, ft);
    Ok(ty)
}

fn localize_possibly_enforced_ty<R: Reason>(
    env: &TEnv<R>,
    localize_env: &LocalizeEnv,
    ty: decl::PossiblyEnforcedTy<decl::DeclTy<R>>,
) -> Result<local::Ty<R>> {
    localize(env, localize_env, ty.ty)
}
