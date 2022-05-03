// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(unused_imports, unused_variables, dead_code)]
use crate::inference_env::InferenceEnv;
use std::ops::Deref;
use ty::local::{Ty, Ty_, Tyvar};
use ty::reason::Reason;

pub fn remove_tyvar_from_upper_bound<R: Reason>(
    env: &mut InferenceEnv<R>,
    tv: Tyvar,
    ty: &Ty<R>,
) -> Ty<R> {
    remove_tyvar_from_upper_bound_help(env, tv, ty).unwrap_or_else(|| Ty::mixed(R::none()))
}

fn remove_tyvar_from_upper_bound_help<R: Reason>(
    env: &mut InferenceEnv<R>,
    tv: Tyvar,
    ty: &Ty<R>,
) -> Option<Ty<R>> {
    use Ty_::*;
    let ty = env.resolve_ty(ty);
    match ty.deref() {
        Tvar(tv2) if tv == *tv2 => None,
        Toption(ty2) => {
            let ety2 = remove_tyvar_from_upper_bound_help(env, tv, ty2)?;
            Some(Ty::option(ty.reason().clone(), ety2))
        }
        Tunion(tys) => {
            let tys_out = tys
                .iter()
                .map(|ty| remove_tyvar_from_upper_bound_help(env, tv, ty))
                .collect::<Option<_>>()?;
            Some(Ty::union(ty.reason().clone(), tys_out))
        }
        _ => Some(ty),
    }
}

pub fn remove_tyvar_from_lower_bound<R: Reason>(
    env: &mut InferenceEnv<R>,
    tv: Tyvar,
    ty: &Ty<R>,
) -> Ty<R> {
    remove_tyvar_from_lower_bound_help(env, tv, ty).unwrap_or_else(|| Ty::nothing(R::none()))
}

fn remove_tyvar_from_lower_bound_help<R: Reason>(
    env: &mut InferenceEnv<R>,
    tv: Tyvar,
    ty: &Ty<R>,
) -> Option<Ty<R>> {
    use Ty_::*;
    let ty = env.resolve_ty(ty);
    match ty.deref() {
        Tvar(tv2) if tv == *tv2 => None,
        Tunion(tys) => {
            let tys_out = tys
                .iter()
                .filter_map(|ty| remove_tyvar_from_lower_bound_help(env, tv, ty))
                .collect();
            Some(Ty::union(ty.reason().clone(), tys_out))
        }
        _ => Some(ty),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use pos::NPos;
    use ty::prop::PropF;
    use ty::reason::NReason;
    use utils::core::IdentGen;

    #[test]
    fn test_remove_upper() {
        let mut env = InferenceEnv::default();
        let gen = IdentGen::new();

        let tv_0: Tyvar = gen.make().into();
        let ty_v0 = Ty::var(NReason::none(), tv_0);
        let tv_1: Tyvar = gen.make().into();
        let ty_v1 = Ty::var(NReason::none(), tv_1);
        let ty_no_v0 = Ty::option(
            NReason::none(),
            Ty::union(
                NReason::none(),
                vec![
                    Ty::int(NReason::none()),
                    Ty::union(NReason::none(), vec![Ty::float(NReason::none()), ty_v1]),
                ],
            ),
        );
        let ty_with_v0 = Ty::option(
            NReason::none(),
            Ty::union(
                NReason::none(),
                vec![
                    Ty::int(NReason::none()),
                    Ty::union(NReason::none(), vec![Ty::float(NReason::none()), ty_v0]),
                ],
            ),
        );

        assert!(remove_tyvar_from_upper_bound_help(&mut env, tv_0, &ty_no_v0).is_some());
        assert!(remove_tyvar_from_upper_bound_help(&mut env, tv_0, &ty_with_v0).is_none());
    }
}
