// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::collections::Vec as BVec;

use crate::typing_env_types::Env;
use decl_rust::decl_subst as subst;
use naming_special_names_rust::typehints;
use oxidized::aast::{Hint, Hint_};
use oxidized::ast;
use oxidized::ast_defs::Id as OwnedId;
use oxidized::pos::Pos as OwnedPos;
use oxidized::ToOxidized;
use oxidized_by_ref::ast::ClassId;
use oxidized_by_ref::ast_defs::Id;
use oxidized_by_ref::pos::Pos;
use typing_defs_rust::tast;
use typing_defs_rust::typing_defs::ExpandEnv;
use typing_defs_rust::typing_defs_core::{FunParam, FunParams, FunType, Tparam, Ty, Ty_};

/// Transforms a declaration phase type into a localized type. This performs
/// common operations that are necessary for this operation, specifically:
///   > Expand newtype/types
///   > Resolves the "this" type
///   > Instantiate generics
///   > ...
///
/// When keep track of additional information while localizing a type such as
/// what type defs were expanded to detect potentially recursive definitions..

pub fn localize<'a>(ety_env: &mut ExpandEnv<'a>, env: &mut Env<'a>, dty: Ty<'a>) -> Ty<'a> {
    let bld = env.builder();
    let Ty(r, dty) = dty;
    match dty {
        Ty_::Tprim(p) => bld.prim(r, (*p).clone()),
        // TODO(hrust) missing matches
        Ty_::Tgeneric(name) => match ety_env.substs.find(&&name[..]) {
            None => bld.generic(r, &name),
            Some(ty) => {
                let ty = env.inference_env.expand_type(ty);
                let r = bld.mk_rinstantiate(ty.get_reason(), &name, r);
                Ty::mk(r, ty.get_node())
            }
        },
        // TODO(hrust) missing matches
        Ty_::Tapply(&(cls, tys)) => {
            let Id(_, cls_name) = cls;
            let tys = match env.genv.provider.get_class(&cls_name) {
                None => {
                    bld.slice_from_iter(tys.iter().copied().map(|ty| localize(ety_env, env, ty)))
                }
                Some(class_info) => {
                    // TODO(hrust) global inference logic
                    // TODO(hrust) will the `unwrap` below panic?
                    // Assumption: r is always Rhint or at least always has a pos
                    localize_tparams(ety_env, env, r.pos.unwrap(), tys, class_info.tparams)
                }
            };
            bld.class(r, cls, tys)
        }
        Ty_::Tfun(ft) => {
            let instantiation = None;
            let ft = localize_ft(ety_env, env, ft, instantiation);
            bld.fun(r, ft)
        }
        _ => {
            // TODO(hrust) missing matches
            unimplemented!("{:#?}", dty)
        }
    }
}

/// Declare and localize the type arguments to a constructor or function, given
/// information about the declared type parameters in `decl_tparam list`. If no
/// explicit type arguments are given, generate fresh type variables in their
/// place; do the same for any wildcard explicit type arguments.
/// Report arity errors using `def_pos` (for the declared parameters), `use_pos`
/// (for the use-site) and `use_name` (the name of the constructor or function).
pub fn localize_targs<'a>(
    env: &mut Env<'a>,
    use_pos: &'a Pos,
    use_name: &'a str,
    tparams: &'a [Tparam<'a>],
    targs: &[ast::Targ],
) -> Vec<tast::Targ<'a>> {
    if targs.len() != 0 {
        unimplemented!("Explicit type arguments not supported")
    }
    // TODO(hrust) localize explicit type arguments
    let explicit_targs = std::iter::empty();
    // Generate fresh type variables for the remainder
    let implicit_targs = tparams[targs.len()..].iter().map(|tparam| {
        let tvar = env
            .inference_env
            .fresh_type_reason(env.bld().mk_rtype_variable_generics(
                use_pos,
                tparam.name.name(),
                use_name,
            ));
        // TODO(hrust) logging
        tast::Targ(
            tvar,
            Hint(
                use_pos.to_oxidized(),
                Box::new(Hint_::Happly(
                    OwnedId(OwnedPos::make_none(), typehints::WILDCARD.to_string()),
                    vec![],
                )),
            ),
        )
    });

    explicit_targs.chain(implicit_targs).collect()
}

pub struct MethodInstantiation<'a: 'b, 'b> {
    /// position of the method call
    #[allow(dead_code)]
    pub use_pos: &'b OwnedPos,
    /// name of the method
    #[allow(dead_code)]
    pub use_name: &'b str,
    pub explicit_targs: &'b Vec<tast::Targ<'a>>,
}

pub fn localize_ft<'a, 'b>(
    ety_env: &mut ExpandEnv<'a>,
    env: &mut Env<'a>,
    ft: &'a FunType,
    instantiation: Option<MethodInstantiation<'a, 'b>>,
) -> &'a FunType<'a> {
    // TODO(hrust) rx stuff
    match instantiation {
        Some(MethodInstantiation {
            use_pos: _,
            use_name: _,
            explicit_targs,
        }) => {
            if !explicit_targs.is_empty() && explicit_targs.len() != ft.tparams.len() {
                unimplemented!("Wrong number of type arguments.")
            }
            let targ_tys = explicit_targs.iter().map(|targ| targ.0);
            let substs = subst::make_locl(env.bld(), ft.tparams.iter(), targ_tys);
            ety_env.substs = ety_env.substs.add_all(env.bld(), substs);
        }
        None => {
            // TODO(hrust)
        }
    };
    /* TODO(hrust):
       - set_env_reactive
       - localize tparams
       - localize where constraints
       - check constraints under substs
       - arity
    */
    let params = localize_funparams(ety_env, env, &ft.params);
    let ret = localize(ety_env, env, ft.ret.type_);
    env.bld().funtype(params, ret)
}

fn localize_funparams<'a>(
    ety_env: &mut ExpandEnv<'a>,
    env: &mut Env<'a>,
    params: &'a FunParams,
) -> &'a [&'a FunParam<'a>] {
    env.bld()
        .slice_from_iter(params.iter().map(|ty| localize_funparam(ety_env, env, ty)))
}

fn localize_funparam<'a>(
    ety_env: &mut ExpandEnv<'a>,
    env: &mut Env<'a>,
    param: &'a FunParam,
) -> &'a FunParam<'a> {
    let type_ = param.type_.type_;
    let type_ = localize(ety_env, env, type_);
    env.bld().funparam(type_)
}

fn localize_tparams<'a>(
    ety_env: &mut ExpandEnv<'a>,
    env: &mut Env<'a>,
    pos: &Pos,
    tys: &[Ty<'a>],
    tparams: &[Tparam<'a>],
) -> &'a [Ty<'a>] {
    let bld = env.builder();
    let length = std::cmp::min(tys.len(), tparams.len());
    bld.slice_from_iter(
        tys.iter()
            .cloned()
            .take(length)
            .zip(tparams.iter().take(length))
            .map(|(ty, tparam)| localize_tparam(ety_env, env, pos, ty, tparam)),
    )
}

fn localize_tparam<'a>(
    ety_env: &mut ExpandEnv<'a>,
    env: &mut Env<'a>,
    _pos: &Pos,
    ty: Ty<'a>,
    _tparam: &Tparam,
) -> Ty<'a> {
    // TODO(hrust) wildcard case
    localize(ety_env, env, ty)
}

pub fn resolve_type_arguments_and_check_constraints<'a>(
    env: &mut Env<'a>,
    check_constraints: bool,
    use_pos: &'a Pos,
    cid: &'a Id<'a>,
    _class_id: &ClassId<'a>,
    tparams: &'a [Tparam<'a>],
    targs: &[ast::Targ],
) -> (Ty<'a>, Vec<tast::Targ<'a>>) {
    // TODO(hrust) strip_ns of cid.name()
    let targs = localize_targs(env, use_pos, cid.name(), tparams, targs);
    let targs_tys = BVec::from_iter_in(
        targs.iter().map(|targ| targ.annot()).copied(),
        env.bld().bumpalo(),
    )
    .into_bump_slice();
    let this_ty = {
        let r = env.bld().mk_rwitness(cid.pos());
        env.bld().class(r, cid.clone(), targs_tys)
    };
    if check_constraints {
        // TODO(hrust) check tparam constraints
    }
    (this_ty, targs)
}
