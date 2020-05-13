// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::typing_enum;
use crate::typing_env_types::Env;
use crate::typing_phase::{self, MethodInstantiation};
use arena_trait::Arena;
use decl_rust::decl_subst as subst;
use oxidized::ast;
use oxidized_by_ref::ast::Id;
use oxidized_by_ref::shallow_decl_defs::ShallowClass;
use oxidized_by_ref::typing_defs::Exact;
use oxidized_by_ref::typing_reason::Reason;
use typing_defs_rust::tast;
use typing_defs_rust::{ExpandEnv, Ty, Ty_};

pub fn obj_get<'a>(
    env: &mut Env<'a>,
    is_method: bool,
    nullsafe: Option<()>,
    receiver: &tast::Expr<'a>,
    member_id: &'a ast::Id,
    method_explicit_type_args: &[ast::Targ],
) -> (Ty<'a>, Vec<tast::Targ<'a>>) {
    obj_get_(
        env,
        is_method,
        nullsafe,
        receiver,
        member_id,
        method_explicit_type_args,
    )
}

fn obj_get_<'a>(
    env: &mut Env<'a>,
    is_method: bool,
    nullsafe: Option<()>,
    receiver: &tast::Expr<'a>,
    member_id: &'a ast::Id,
    method_explicit_type_args: &[ast::Targ],
) -> (Ty<'a>, Vec<tast::Targ<'a>>) {
    // TODO(hrust) expand receiver type and solve / narrow
    // TODO(hrust) handle receiver type being union, inter and so on
    obj_get_concrete_ty(
        env,
        is_method,
        nullsafe,
        receiver,
        member_id,
        method_explicit_type_args,
    )
}

fn obj_get_concrete_ty<'a>(
    env: &mut Env<'a>,
    is_method: bool,
    _nullsafe: Option<()>,
    receiver: &tast::Expr<'a>,
    member_id: &'a ast::Id,
    method_explicit_type_args: &[ast::Targ],
) -> (Ty<'a>, Vec<tast::Targ<'a>>) {
    let bld = env.bld();
    let tast::Expr((_p, receiver_ty), _receiver_) = receiver;
    let receiver_ty = env.inference_env.expand_type(*receiver_ty);
    let (_r, receiver_ty_) = receiver_ty.unpack();
    match receiver_ty_ {
        Ty_::Tclass(&(class_id, exact, class_type_args)) => {
            // TODO(hrust): use eager class
            match env.genv.provider.get_shallow_class(class_id.name()) {
                None => unimplemented!(),
                Some(class_info) => {
                    // TODO(hrust) default if not a method in partial mode and
                    // shouldn't check error 4053 and...
                    // TODO(hrust) make type args any if missing (WHY??)
                    let member_info = env.get_member(class_info, member_id.name(), is_method);
                    // TODO(hrust) check ancestors
                    match member_info {
                        None => unimplemented!(),
                        Some(member_info) => {
                            // TODO(hrust) ambiguous object access error
                            // TODO(hrust) check obj access and deprecated
                            // TODO(hrust) parent abstract call error
                            let Ty(r, member_decl_ty) = typing_enum::member_type(env, member_info);
                            let mut ety_env =
                                mk_ety_env(env, r, class_info, class_id, exact, class_type_args);
                            let (member_ty, method_type_args, _et_enforced) = match member_decl_ty {
                                Ty_::Tfun(fun_type) if is_method => {
                                    // TODO(hrust) strip_ns of member_id.name() below
                                    // let type_arg_types = type_args.iter().map(|targ| targ.annot());
                                    let method_type_args = typing_phase::localize_targs(
                                        env,
                                        member_id.pos(),
                                        member_id.name(),
                                        &fun_type.tparams,
                                        method_explicit_type_args,
                                    );
                                    // TODO(hrust) compute enforced and pessimize
                                    let instantiation = Some(MethodInstantiation {
                                        use_name: member_id.name(),
                                        use_pos: member_id.pos(),
                                        explicit_targs: &method_type_args,
                                    });
                                    let fun_type = typing_phase::localize_ft(
                                        &mut ety_env,
                                        env,
                                        &fun_type,
                                        instantiation,
                                    );
                                    (bld.fun(bld.alloc(r), fun_type), method_type_args, false)
                                }
                                _ => unimplemented!(),
                            };
                            // TODO(hrust) check_inst_meth_access
                            // TODO(hrust) case with upper bounds on this
                            // TODO(hrust) coercion
                            (member_ty, method_type_args)
                        }
                    }
                }
            }
        }
        _ => unimplemented!(),
    }
}

fn mk_ety_env<'a>(
    env: &Env<'a>,
    _r: &Reason<'a>,
    class_info: &'a ShallowClass<'a>,
    _class_id: Id,
    _exact: Exact,
    type_args: &[Ty<'a>],
) -> ExpandEnv<'a> {
    ExpandEnv {
        substs: subst::make_locl(
            env.bld(),
            class_info.tparams.iter(),
            type_args.iter().copied(),
        ),
        type_expansions: bumpalo::vec![in env.bld().bumpalo()],
    }
}
