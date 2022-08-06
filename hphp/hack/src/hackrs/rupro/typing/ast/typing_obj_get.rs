// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use pos::MethodName;
use pos::Symbol;
use pos::TypeName;
use ty::decl;
use ty::local::Ty;
use ty::local::Ty_;
use ty::reason::Reason;

use crate::tast;
use crate::typing::ast::typing_localize::LocalizeEnv;
use crate::typing::ast::typing_localize::LocalizeFunTypeParams;
use crate::typing::ast::typing_tparam::TCTargs;
use crate::typing::ast::typing_tparam::TCTargsParams;
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;
use crate::typing_decl_provider::Class;
use crate::typing_decl_provider::ClassElt;

/// This struct provides typing for member access, both static and instance.
pub struct TCObjGet<'a, R: Reason> {
    /// Type of the object/receiver, i.e. type of the `expr` in `expr->get()`.
    pub receiver_ty: &'a Ty<R>,

    /// Member name
    pub member_id: &'a oxidized::ast_defs::Id,

    /// Is the member access a method?
    pub is_method: bool,

    /// Explicit arguments provided to this method call.
    pub explicit_targs: &'a [oxidized::aast::Targ<()>],
}

/// The result of typing a member access.
#[derive(Debug)]
pub struct TCObjGetResult<R: Reason> {
    /// The return type, i.e. the type of the full `expr->get()` expression.
    pub ty: Ty<R>,

    /// The types of the type arguments.
    pub targs: Vec<tast::Targ<R>>,

    /// An optional error based on the receiver's type, if encountered.
    pub lval_err: Option<()>,

    /// An optional error if the coercion to the object type failed.
    pub rval_err: Option<()>,
}

/// Internal struct for shared parameters
struct TCObjGetInternalParams<'a> {
    /// Is the member access for a method?
    is_method: bool,

    /// Explicit arguments provided to this method call.
    pub explicit_targs: &'a [oxidized::aast::Targ<()>],
}

impl<'a, R: Reason> Infer<R> for TCObjGet<'a, R> {
    type Params = ();
    type Typed = TCObjGetResult<R>;

    fn infer(&self, env: &mut TEnv<R>, _params: ()) -> Result<Self::Typed> {
        rupro_todo_mark!(Solving, "solve receiver ty");
        rupro_todo_mark!(DependentType);
        rupro_todo_mark!(ParentCall);
        rupro_todo_mark!(MissingError);

        let args = TCObjGetInternalParams {
            is_method: self.is_method,
            explicit_targs: self.explicit_targs,
        };
        obj_get_inner(env, &args, self.receiver_ty, self.member_id)
    }
}

fn obj_get_inner<R: Reason>(
    env: &mut TEnv<R>,
    args: &TCObjGetInternalParams<'_>,
    receiver_ty: &Ty<R>,
    member_id: &oxidized::ast_defs::Id,
) -> Result<TCObjGetResult<R>> {
    rupro_todo_assert!(args.is_method, MemberAccess);
    let receiver_ty = env.resolve_ty_and_solve(receiver_ty)?;
    obj_get_concrete_ty(env, args, &receiver_ty, member_id)
}

fn obj_get_concrete_ty<R: Reason>(
    env: &mut TEnv<R>,
    args: &TCObjGetInternalParams<'_>,
    receiver_ty: &Ty<R>,
    member_id: &oxidized::ast_defs::Id,
) -> Result<TCObjGetResult<R>> {
    rupro_todo_mark!(Solving, "expand_type");
    match &**receiver_ty {
        Ty_::Tclass(class_name, _, paraml) => obj_get_concrete_class(
            env,
            args,
            receiver_ty,
            member_id,
            class_name.id_ref(),
            paraml,
        ),
        _ => rupro_todo!(MemberAccess),
    }
}

fn obj_get_concrete_class<R: Reason>(
    env: &mut TEnv<R>,
    args: &TCObjGetInternalParams<'_>,
    receiver_ty: &Ty<R>,
    member_id: &oxidized::ast_defs::Id,
    class_name: &TypeName,
    paraml: &[Ty<R>],
) -> Result<TCObjGetResult<R>> {
    match env.decls().get_class(class_name.clone())? {
        None => rupro_todo!(MissingError),
        Some(class_info) => {
            rupro_todo_assert!(
                !paraml.is_empty() || class_info.get_tparams().is_empty(),
                AST
            );
            rupro_todo_mark!(MemberAccess, "private visibility precedence");
            let member_info = env.decls().get_class_member(
                args.is_method,
                &*class_info,
                MethodName::from(&member_id.1),
            )?;
            match member_info {
                None => rupro_todo!(MissingError),
                Some(member_info) => obj_get_concrete_class_with_member_info(
                    env,
                    args,
                    receiver_ty,
                    member_id,
                    class_name,
                    class_info.as_ref(),
                    paraml,
                    &*member_info,
                ),
            }
        }
    }
}

fn obj_get_concrete_class_with_member_info<R: Reason>(
    env: &mut TEnv<R>,
    args: &TCObjGetInternalParams<'_>,
    _receiver_ty: &Ty<R>,
    member_id: &oxidized::ast_defs::Id,
    _class_name: &TypeName,
    class_info: &dyn Class<R>,
    paraml: &[Ty<R>],
    member_info: &ClassElt<R>,
) -> Result<TCObjGetResult<R>> {
    rupro_todo_mark!(MissingError, "Ambiguous_object_access");
    rupro_todo_mark!(Xhp, "Typing_enum.member_type");
    rupro_todo_mark!(Variance, "this_appears_covariantly");
    let member_decl_ty = member_info.ty();
    match member_decl_ty.node_ref() {
        decl::Ty_::Tfun(ft) if args.is_method => {
            rupro_todo_mark!(Dynamic);
            rupro_todo_assert!(args.explicit_targs.is_empty(), AST);
            let targs = TCTargs {
                tparams: &*ft.tparams,
            }
            .infer(
                env,
                TCTargsParams {
                    use_pos: &member_id.0,
                    use_name: Symbol::from(&member_id.1),
                },
            )?;
            let ft = ft.infer(
                env,
                LocalizeFunTypeParams {
                    explicit_targs: targs.clone(),
                    localize_env: LocalizeEnv::with_class_subst(class_info, paraml),
                },
            )?;
            let ty = Ty::fun(member_decl_ty.reason().clone(), ft);
            Ok(TCObjGetResult {
                ty,
                targs,
                lval_err: None,
                rval_err: None,
            })
        }
        _ => rupro_todo!(MemberAccess),
    }
}
