// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use bumpalo::collections::Vec as BVec;

use crate::typing_env_types::Env;
use naming_special_names_rust::typehints;
use oxidized::aast::{Hint, Hint_};
use oxidized::aast_defs::Tprim;
use oxidized::ast;
use oxidized::ast_defs::Id;
use oxidized::pos::Pos;
use oxidized::typing_defs_core::{FunType as DFunType, Tparam as DTparam, Ty as DTy, Ty_ as DTy_};
use typing_defs_rust::tast;
use typing_defs_rust::typing_defs::ExpandEnv_;
use typing_defs_rust::typing_defs_core::{FunType, PrimKind, Ty};
use typing_defs_rust::typing_reason::PReason_;

/// Transforms a declaration phase type into a localized type. This performs
/// common operations that are necessary for this operation, specifically:
///   > Expand newtype/types
///   > Resolves the "this" type
///   > Instantiate generics
///   > ...
///
/// When keep track of additional information while localizing a type such as
/// what type defs were expanded to detect potentially recursive definitions..

pub fn localize<'a>(ety_env: &'a ExpandEnv_<'a>, env: &mut Env<'a>, dty: &'a DTy) -> Ty<'a> {
    let bld = env.builder();
    let DTy(r0, dty) = dty;
    let r = env
        .builder()
        .alloc_reason(PReason_::from_decl_provided_reason(&r0));
    match &**dty {
        DTy_::Tprim(p) => bld.prim(r, localize_prim(env, p)),
        // TODO(hrust) missing matches
        DTy_::Tgeneric(name) => match ety_env.substs.find(&&name[..]) {
            None => bld.generic(r, &name),
            Some(ty) => {
                let ty = env.inference_env.expand_type(ty);
                let r = bld.mk_rinstantiate(ty.get_reason(), &name, r);
                Ty::mk(r, ty.get_node())
            }
        },
        // TODO(hrust) missing matches
        DTy_::Tapply(cls, tys) => {
            let Id(_, cls_name) = cls;
            let tys = match env.genv.provider.get_class(&cls_name) {
                None => bld.vec_from_iter(tys.iter().map(|ty: &'a DTy| localize(ety_env, env, ty))),
                Some(class_info) => {
                    // TODO(hrust) global inference logic
                    // TODO(hrust) will the `unwrap` below panic?
                    // Assumption: r is always Rhint or at least always has a pos
                    localize_tparams(ety_env, env, r.pos.unwrap(), tys, &class_info.tparams)
                }
            };
            bld.class(r, &cls, tys)
        }
        DTy_::Tfun(ft) => {
            let ft = localize_ft(ety_env, env, ft);
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
    use_name: &'a String,
    tparams: &'a Vec<DTparam>,
    targs: &Vec<ast::Targ>,
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
                use_pos.clone(),
                Box::new(Hint_::Happly(
                    Id(Pos::make_none(), typehints::WILDCARD.to_string()),
                    vec![],
                )),
            ),
        )
    });

    explicit_targs.chain(implicit_targs).collect()
}

fn localize_prim<'a>(_env: &mut Env<'a>, prim: &Tprim) -> PrimKind<'a> {
    match prim {
        Tprim::Tnull => PrimKind::Tnull,
        Tprim::Tvoid => PrimKind::Tvoid,
        Tprim::Tint => PrimKind::Tint,
        Tprim::Tbool => PrimKind::Tbool,
        Tprim::Tfloat => PrimKind::Tfloat,
        Tprim::Tstring => PrimKind::Tstring,
        Tprim::Tresource => PrimKind::Tresource,
        Tprim::Tnum => PrimKind::Tnum,
        Tprim::Tarraykey => PrimKind::Tarraykey,
        Tprim::Tnoreturn => PrimKind::Tnoreturn,
        Tprim::Tatom(_s) => panic!("Tatom NYI"),
    }
}

fn localize_ft<'a>(
    ety_env: &'a ExpandEnv_<'a>,
    env: &mut Env<'a>,
    ft: &'a DFunType,
) -> FunType<'a> {
    let ret = localize(ety_env, env, &ft.ret.type_);
    env.bld().funtype(ret)
}

fn localize_tparams<'a>(
    ety_env: &'a ExpandEnv_<'a>,
    env: &mut Env<'a>,
    pos: &Pos,
    tys: &'a Vec<DTy>,
    tparams: &Vec<DTparam>,
) -> BVec<'a, Ty<'a>> {
    let bld = env.builder();
    let length = std::cmp::min(tys.len(), tparams.len());
    bld.vec_from_iter(
        tys.iter()
            .take(length)
            .zip(tparams.iter().take(length))
            .map(|(ty, tparam)| localize_tparam(ety_env, env, pos, ty, tparam)),
    )
}

fn localize_tparam<'a>(
    ety_env: &'a ExpandEnv_<'a>,
    env: &mut Env<'a>,
    _pos: &Pos,
    ty: &'a DTy,
    _tparam: &DTparam,
) -> Ty<'a> {
    // TODO(hrust) wildcard case
    localize(ety_env, env, ty)
}
