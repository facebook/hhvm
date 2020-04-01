// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//use crate::typing_env_types::Ty_::*;
use crate::typing_env_types::*;
use crate::typing_phase::localize;
use arena_trait::Arena;
use bumpalo::collections::Vec;
use itertools::*;
use oxidized::ast;
use typing_collections_rust::SMap;
use typing_defs_rust::Ty;

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
        (InternalType_::LoclType(ty_sub), InternalType_::LoclType(ty_super)) => {
            let ty_sub = env.inference_env.expand_type(*ty_sub);
            let ty_super = env.inference_env.expand_type(*ty_super);
            match (ty_sub.get_node(), ty_super.get_node()) {
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
                    Ty_::Tclass(x_sub, exact_sub, tyl_sub),
                    Ty_::Tclass(x_super, exact_super, tyl_super),
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
                                    simplify_subtype_variance(env, &cd.tparams, tyl_sub, tyl_super)
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
                                        type_expansions: Vec::new_in(bld.alloc),
                                        substs,
                                    });
                                    let up_obj_opt = env.provider().get_ancestor(cd, cid_super);
                                    match up_obj_opt {
                                        Some(up_obj) => {
                                            let up_obj = localize(ety_env, env, &up_obj).clone();
                                            simplify_subtype(env, up_obj, ty_super)
                                        }
                                        None => invalid(),
                                    }
                                }
                            }
                        }
                    }
                }
                (Ty_::Tclass(_, _, _), Ty_::Tprim(_)) => invalid(),
                (Ty_::Tprim(_), Ty_::Tclass(_, _, _)) => invalid(),
                (Ty_::Tclass(_, _, _), Ty_::Tgeneric(_)) => invalid(),
                (Ty_::Tgeneric(_), Ty_::Tclass(_, _, _)) => invalid(),

                _ => default(),
            }
        }
        _ => default(),
    }
}

fn simplify_subtype_variance<'a>(
    env: &mut Env<'a>,
    tparaml: &std::vec::Vec<oxidized::typing_defs::Tparam>,
    children_tyl: &'a Vec<Ty<'a>>,
    super_tyl: &'a Vec<Ty<'a>>,
) -> SubtypeProp<'a> {
    // Collect propositions in an arena-allocated vector
    let mut props = Vec::new_in(env.bld().alloc);
    for (tparam, &ty_sub, &ty_super) in izip!(tparaml.iter(), children_tyl.iter(), super_tyl.iter())
    {
        // Direct decl parser does not yet support variance, so used naming convention instead
        // Type parameters start "TP" are covariant, "TN" contravariant, all others invariant
        // TODO(hrust): use actual variance when it is implemented
        let variance = if tparam.name.1.len() < 2 {
            ast::Variance::Invariant
        } else {
            match &tparam.name.1[0..2] {
                "TP" => ast::Variance::Covariant,
                "TN" => ast::Variance::Contravariant,
                _ => ast::Variance::Invariant,
            }
        };
        // Apply subtyping according to the variance of the type parameter
        match variance {
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
    process_simplify_subtype_result(env, &prop);
}
