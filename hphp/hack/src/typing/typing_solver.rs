// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::typing_union;
use crate::Env;
use oxidized_by_ref::ident::Ident;
use typing_collections_rust::Set;
use typing_defs_rust::{InternalType, Ty};

pub fn solve_all_unsolved_tyvars(env: &mut Env) {
    env.inference_env
        .iter()
        .for_each(|v| always_solve_tyvar(env, *v))
}

fn always_solve_tyvar(env: &mut Env, v: Ident) {
    solve_to_equal_bound_or_wrt_variance(env, v);
    always_solve_tyvar_down(env, v);
}

fn solve_to_equal_bound_or_wrt_variance(env: &mut Env, v: Ident) {
    // TODO(hrust) solve until concrete type
    try_bind_to_equal_bound(env, v);
    solve_tyvar_wrt_variance(env, v);
}

fn always_solve_tyvar_down(env: &mut Env, v: Ident) {
    // TODO(hrust) check if global
    if !(env.inference_env.tyvar_is_solved(v)) {
        unimplemented!()
    }
}

fn try_bind_to_equal_bound(env: &mut Env, v: Ident) {
    if !(env.inference_env.tyvar_is_solved(v)) {
        let bld = env.bld();
        // TODO(hrust) remove tyvar from bounds
        let lower_bounds = {
            let bounds = env.inference_env.get_lower_bounds(v).iter().copied();
            let bounds = bounds.map(|ty| env.inference_env.expand_internal_type(ty));
            Set::from(bld, bounds)
        };
        let upper_bounds = {
            let bounds = env.inference_env.get_upper_bounds(v).iter().copied();
            let bounds = bounds.map(|ty| env.inference_env.expand_internal_type(ty));
            Set::from(bld, bounds)
        };
        let mut equal_bounds = lower_bounds.intersection(upper_bounds).copied();
        // TODO(hrust) remove Terr and Tany from equal bounds
        match equal_bounds.next() {
            Some(ty) => {
                match ty {
                    InternalType::LoclType(ty) => {
                        // TODO union with any if any in lower bounds
                        bind(env, v, ty)
                    }
                    InternalType::ConstraintType(_) => {
                        // TODO(hrust)
                    }
                }
            }
            None => {
                // TODO(hrust)
            }
        }
    }
}

fn solve_tyvar_wrt_variance(env: &mut Env, v: Ident) {
    if !(env.inference_env.tyvar_is_solved(v)) {
        // TODO(hrust) check global, update Rnone
        let appears_covariantly = env.inference_env.get_appears_covariantly(v);
        let appears_contravariantly = env.inference_env.get_appears_contravariantly(v);
        match (appears_covariantly, appears_contravariantly) {
            (true, false) | (false, false) => {
                // As in Local Type Inference by Pierce & Turner, if type variable does
                // not appear at all, or only appears covariantly, solve to lower bound
                bind_to_lower_bound(env, v)
            }
            (false, true) => {
                // As in Local Type Inference by Pierce & Turner, if type variable
                // appears only contravariantly, solve to upper bound
                bind_to_upper_bounds(env, v)
            }
            (true, true) => {}
        }
    }
}

fn bind_to_lower_bound(env: &mut Env, v: Ident) {
    let lower_bounds = {
        let lower_bounds = env.inference_env.get_lower_bounds(v);
        // TODO(hrust) abort if constraint type in lower bounds
        // TODO(hrust) remove type var itself from its lower bounds
        lower_bounds
            .iter()
            .copied()
            .filter_map(InternalType::get_locl_type_opt)
    };
    let ty = typing_union::union_list(env, lower_bounds);
    // TODO(hrust) freshen
    // TODO(hrust) remove var from the upper bounds of its lower bounds
    bind(env, v, ty)
}

fn bind_to_upper_bounds(env: &mut Env, v: Ident) {
    let _upper_bounds = env.inference_env.get_upper_bounds(v);
    // TODO(hrust)
    unimplemented!()
}

fn bind<'a>(env: &mut Env<'a>, v: Ident, ty: Ty<'a>) {
    // TODO(hrust) logging, update reason, update variance, error if cycling binding
    env.inference_env.add(v, ty)
}
