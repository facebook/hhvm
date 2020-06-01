// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//use crate::typing_env_types::Ty_::*;
use crate::typing_env_types::*;
use crate::typing_phase::localize;
use arena_trait::Arena;
use bumpalo::collections::Vec as BVec;
use itertools::*;
use oxidized_by_ref::ast;
use oxidized_by_ref::ident::Ident;
use typing_collections_rust::{SMap, Vec};
use typing_defs_rust::{avec, Ty};

pub fn is_sub_type_for_union<'a>(env: &mut Env<'a>, ty_sub: Ty<'a>, ty_super: Ty<'a>) -> bool {
    match is_sub_type(env, ty_sub, ty_super) {
        Some(b) => b,
        None => false,
    }
}

fn is_sub_type<'a>(env: &mut Env<'a>, ty_sub: Ty<'a>, ty_super: Ty<'a>) -> Option<bool> {
    let bld = env.bld();
    let ty_sub = bld.loclty(ty_sub);
    let ty_super = bld.loclty(ty_super);
    is_sub_type_i(env, ty_sub, ty_super)
}

fn is_sub_type_i<'a>(
    env: &mut Env<'a>,
    ty_sub: InternalType<'a>,
    ty_super: InternalType<'a>,
) -> Option<bool> {
    let prop = simplify_subtype_i(env, ty_sub, ty_super);
    if prop.is_valid() {
        Some(true)
    } else if prop.is_unsat() {
        Some(false)
    } else {
        None
    }
}

fn simplify_subtype_i<'a>(
    env: &mut Env<'a>,
    ty_sub: InternalType<'a>,
    ty_super: InternalType<'a>,
) -> SubtypeProp<'a> {
    let bld = env.bld();
    let valid = || bld.valid();
    let invalid = || bld.invalid();
    let default = || bld.is_subtype(ty_sub, ty_super);

    // This doesn't correspond to the pattern match order used in the OCaml
    // TODO(hrust): when we are dealing with more types, refactor this so that it does correspond
    match (ty_sub, ty_super) {
        (InternalType::LoclType(ty_sub), InternalType::LoclType(ty_super)) => {
            let ty_sub = env.inference_env.expand_type(ty_sub);
            let ty_super = env.inference_env.expand_type(ty_super);
            match (ty_sub.get_node(), ty_super.get_node()) {
                (Ty_::Tvar(_), _) | (_, Ty_::Tvar(_)) => default(),
                (Ty_::Tprim(p1), Ty_::Tprim(p2)) => match (p1, p2) {
                    (PrimKind::Tint, PrimKind::Tnum)
                    | (PrimKind::Tfloat, PrimKind::Tnum)
                    | (PrimKind::Tint, PrimKind::Tarraykey)
                    | (PrimKind::Tstring, PrimKind::Tarraykey) => valid(),
                    _ => {
                        if p1 == p2 {
                            valid()
                        } else {
                            invalid()
                        }
                    }
                },
                // TODO(hrust): bounds on generics!
                (Ty_::Tprim(_), Ty_::Tgeneric(_)) => invalid(),
                (Ty_::Tgeneric(_), Ty_::Tprim(_)) => invalid(),
                (Ty_::Tgeneric(name1), Ty_::Tgeneric(name2)) => {
                    if name1 == name2 {
                        valid()
                    } else {
                        invalid()
                    }
                }
                (
                    Ty_::Tclass((x_sub, exact_sub, tyl_sub)),
                    Ty_::Tclass((x_super, exact_super, tyl_super)),
                ) => {
                    let exact_match = match (exact_sub, exact_super) {
                        (Exact::Nonexact, Exact::Exact) => false,
                        (_, _) => true,
                    };
                    let (cid_super, cid_sub) = (&x_super.1, &x_sub.1);
                    let class_def_sub = env.provider().get_class(cid_sub);
                    if cid_sub == cid_super {
                        // If class is final then exactness is superfluous
                        let is_final = match class_def_sub {
                            Some(cd) => cd.final_,
                            None => false,
                        };
                        if !(exact_match || is_final) {
                            invalid()
                        } else {
                            match class_def_sub {
                                None => invalid(),
                                Some(cd) => {
                                    simplify_subtype_variance(env, cd.tparams, tyl_sub, tyl_super)
                                }
                            }
                        }
                    } else {
                        if !exact_match {
                            invalid()
                        } else {
                            match class_def_sub {
                                // This should have been caught already in the naming phase
                                None => valid(),
                                Some(cd) => {
                                    let bld = env.builder();
                                    let mut substs = SMap::empty();
                                    for (tparam, &ty) in izip!(cd.tparams.iter(), tyl_sub.iter()) {
                                        substs = substs.add(bld, &tparam.name.1, ty)
                                    }
                                    let ety_env = bld.alloc(ExpandEnv {
                                        type_expansions: BVec::new_in(bld.bumpalo()),
                                        substs,
                                    });
                                    match cd.ancestors.get(cid_super) {
                                        Some(up_obj) => {
                                            let up_obj = localize(ety_env, env, *up_obj);
                                            simplify_subtype(env, up_obj, ty_super)
                                        }
                                        None => {
                                            // TODO(hrust): traits and interfaces
                                            invalid()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                (Ty_::Tclass(..), Ty_::Tprim(..)) => invalid(),
                (Ty_::Tprim(..), Ty_::Tclass(..)) => invalid(),
                (Ty_::Tclass(..), Ty_::Tgeneric(..)) => invalid(),
                (Ty_::Tgeneric(..), Ty_::Tclass(..)) => invalid(),

                _ => default(),
            }
        }
        _ => default(),
    }
}

fn simplify_subtype_variance<'a>(
    env: &mut Env<'a>,
    tparaml: &[oxidized_by_ref::typing_defs::Tparam],
    children_tyl: &'a [Ty<'a>],
    super_tyl: &'a [Ty<'a>],
) -> SubtypeProp<'a> {
    // Collect propositions in an arena-allocated vector
    let mut props = BVec::new_in(env.bld().bumpalo());
    for (tparam, &ty_sub, &ty_super) in izip!(tparaml.iter(), children_tyl.iter(), super_tyl.iter())
    {
        // Apply subtyping according to the variance of the type parameter
        match tparam.variance {
            ast::Variance::Covariant => {
                props.push(simplify_subtype(env, ty_sub, ty_super));
            }
            ast::Variance::Contravariant => {
                props.push(simplify_subtype(env, ty_super, ty_sub));
            }
            ast::Variance::Invariant => {
                props.push(simplify_subtype(env, ty_sub, ty_super));
                props.push(simplify_subtype(env, ty_super, ty_sub));
            }
        };
    }
    // Return the conjunction of the collected propositions
    env.bld().conj(props)
}

fn simplify_subtype<'a>(env: &mut Env<'a>, ty_sub: Ty<'a>, ty_super: Ty<'a>) -> SubtypeProp<'a> {
    let ity_sub = env.bld().loclty(ty_sub);
    let ity_super = env.bld().loclty(ty_super);
    simplify_subtype_i(env, ity_sub, ity_super)
}

pub fn sub_type<'a>(env: &mut Env<'a>, ty_sub: Ty<'a>, ty_super: Ty<'a>) {
    let ity_sub = env.bld().loclty(ty_sub);
    let ity_super = env.bld().loclty(ty_super);
    sub_type_i(env, ity_sub, ity_super)
}

fn process_simplify_subtype_result<'a>(env: &Env<'a>, prop: SubtypeProp<'a>) {
    match prop {
        SubtypePropEnum::IsSubtype(_, _) => {
            panic!("unexpected subtype assertion");
        }
        SubtypePropEnum::Coerce(_, _) => {
            panic!("unexpected coercions assertion");
        }
        SubtypePropEnum::Disj(_) => {
            println!("Type error: proper error handling NYI");
        }
        SubtypePropEnum::Conj(props) => {
            for prop in props {
                process_simplify_subtype_result(env, prop);
            }
        }
    }
}
// Merge sub_type_inner and sub_type_i
pub fn sub_type_i<'a>(env: &mut Env<'a>, ty_sub: InternalType<'a>, ty_super: InternalType<'a>) {
    let prop = simplify_subtype_i(env, ty_sub, ty_super);
    let prop = prop_to_env(env, prop);
    process_simplify_subtype_result(env, &prop);
}

fn prop_to_env<'a>(env: &mut Env<'a>, prop: SubtypeProp<'a>) -> SubtypeProp<'a> {
    let mut props_acc = avec![in env.bld()];
    prop_to_env_(env, prop, &mut props_acc);
    // TODO(hrust) proper conjunction
    env.bld().conj(props_acc)
}

fn prop_to_env_<'a>(
    env: &mut Env<'a>,
    prop: SubtypeProp<'a>,
    props_acc: &mut BVec<'a, SubtypeProp<'a>>,
) {
    use SubtypePropEnum as SP;
    match prop {
        &SP::IsSubtype(ty_sub, ty_super) => match (ty_sub.get_var(), ty_super.get_var()) {
            (Some(var_sub), Some(var_super)) => {
                let prop = env.bld().valid();
                let prop = add_tyvar_upper_bound_and_close(env, prop, var_sub, ty_super);
                let prop = add_tyvar_lower_bound_and_close(env, prop, var_super, ty_sub);
                prop_to_env_(env, prop, props_acc)
            }
            (Some(var_sub), None) => {
                let prop = env.bld().valid();
                let prop = add_tyvar_upper_bound_and_close(env, prop, var_sub, ty_super);
                prop_to_env_(env, prop, props_acc)
            }
            (None, Some(var_super)) => {
                let prop = env.bld().valid();
                let prop = add_tyvar_lower_bound_and_close(env, prop, var_super, ty_sub);
                prop_to_env_(env, prop, props_acc)
            }
            (None, None) => props_acc.push(prop),
        },
        SP::Coerce(_, _) => unimplemented!("{:?}", prop),
        SP::Disj(props) => {
            if props.is_empty() {
                props_acc.push(prop)
            } else {
                // TODO(hrust) try and find the first prop that works. Requires env backtracking.
                prop_to_env_(env, props[0], props_acc)
            }
        }
        SP::Conj(props) => props_to_env(env, props, props_acc),
    }
}

fn props_to_env<'a>(
    env: &mut Env<'a>,
    props: &Vec<'a, SubtypeProp<'a>>,
    props_acc: &mut BVec<'a, SubtypeProp<'a>>,
) {
    props
        .iter()
        .for_each(|prop| prop_to_env_(env, prop, props_acc))
}

fn add_tyvar_upper_bound_and_close<'a>(
    env: &mut Env<'a>,
    prop: SubtypeProp<'a>,
    v: Ident,
    ty: InternalType<'a>,
) -> SubtypeProp<'a> {
    let upper_bounds_before = env.inference_env.get_upper_bounds(v);
    // TODO(hrust) add_tyvar_upper_bound_and_update_variances instead
    env.inference_env.add_upper_bound(v, ty);
    let upper_bounds_after = env.inference_env.get_upper_bounds(v);
    let added_upper_bounds = upper_bounds_after.diff(env.bld(), upper_bounds_before);
    let lower_bounds = env.inference_env.get_lower_bounds(v);
    added_upper_bounds
        .into_iter()
        .fold(prop, |prop, &upper_bound| {
            // TODO(hrust) make all type consts and pu equal
            lower_bounds.into_iter().fold(prop, |prop, &lower_bound| {
                simplify_subtype_i(env, lower_bound, upper_bound).conj(env.bld().bumpalo(), prop)
            })
        })
}

fn add_tyvar_lower_bound_and_close<'a>(
    env: &mut Env<'a>,
    prop: SubtypeProp<'a>,
    v: Ident,
    ty: InternalType<'a>,
) -> SubtypeProp<'a> {
    let lower_bounds_before = env.inference_env.get_lower_bounds(v);
    // TODO(hrust) add_tyvar_lower_bound_and_update_variances instead
    env.inference_env.add_lower_bound(v, ty);
    let lower_bounds_after = env.inference_env.get_lower_bounds(v);
    let added_lower_bounds = lower_bounds_after.diff(env.bld(), lower_bounds_before);
    let upper_bounds = env.inference_env.get_upper_bounds(v);
    added_lower_bounds
        .into_iter()
        .fold(prop, |prop, &lower_bound| {
            // TODO(hrust) make all type consts and pu equal
            upper_bounds.into_iter().fold(prop, |prop, &upper_bound| {
                simplify_subtype_i(env, lower_bound, upper_bound).conj(env.bld().bumpalo(), prop)
            })
        })
}
