// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;
use crate::typing_decl_provider::{Class, TypeDecl};
use pos::{Positioned, SymbolMap, TypeName};
use ty::decl;
use ty::local::{self, Exact, FunParam, FunType};
use ty::reason::Reason;

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

    /// Get the substitution for a certain type name.
    fn get_subst(&self, x: &TypeName) -> Option<local::Ty<R>> {
        self.substs.get(&x.0).cloned()
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

impl<R: Reason> Infer<R> for decl::FunType<R, decl::Ty<R>> {
    type Params = LocalizeEnv<R>;
    type Typed = local::FunType<R>;

    fn infer(&self, env: &mut TEnv<R>, localize_env: LocalizeEnv<R>) -> Result<local::FunType<R>> {
        localize_ft(env, &localize_env, self)
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
        Tfun(box ft) => local::Ty::fun(r, localize_ft(env, localize_env, ft)?),
        Tgeneric(box (tparam, hl)) => {
            rupro_todo_assert!(hl.is_empty(), HKD);
            rupro_todo_assert!(
                localize_env.get_subst(tparam).is_none(),
                Localization,
                "type parameter substitution"
            );
            local::Ty::generic(r, tparam.clone(), vec![])
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
    localize_env: &LocalizeEnv<R>,
    ft: &decl::FunType<R, decl::Ty<R>>,
) -> Result<local::FunType<R>> {
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
    let ft = FunType {
        params,
        ret,
        flags: ft.flags.clone(),
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
