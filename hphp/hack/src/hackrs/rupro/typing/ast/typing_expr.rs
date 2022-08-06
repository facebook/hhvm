// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use pos::FunName;
use ty::decl;
use ty::local::FunParam;
use ty::local::Ty;
use ty::local::Ty_;
use ty::reason::Reason;
use utils::core::LocalId;

use crate::tast;
use crate::typing::ast::typing_localize::LocalizeEnv;
use crate::typing::ast::typing_localize::LocalizeFunTypeParams;
use crate::typing::ast::typing_obj_get::TCObjGet;
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;

/// Parameters that affect the typing of expressions.
#[derive(Default)]
pub struct TCExprParams {
    /// Type check the expression with an empty variable environment.
    ///
    /// The variable environment will be emptied before type checking the
    /// expression.
    empty_locals: bool,
}

impl TCExprParams {
    pub fn empty_locals() -> Self {
        Self { empty_locals: true }
    }
}

impl<R: Reason> Infer<R> for oxidized::aast::Expr<(), ()> {
    type Params = TCExprParams;
    type Typed = tast::Expr<R>;

    fn infer(&self, env: &mut TEnv<R>, params: TCExprParams) -> Result<Self::Typed> {
        rupro_todo_mark!(BidirectionalTC);
        rupro_todo_mark!(Coeffects);

        if params.empty_locals {
            env.reinitialize_locals();
        }

        use oxidized::aast::Expr_::*;
        let p = R::Pos::from(&self.1);
        let (e, ty) = match &self.2 {
            Int(s) => (Int(s.clone()), Ty::int(R::witness(p))),
            String(s) => (String(s.clone()), Ty::string(R::witness(p))),
            Binop(box (op, e1, e2)) => infer_binop(env, p, op, e1, e2)?,
            Lvar(box id) => infer_lvar(env, id)?,
            Call(box (e, explicit_targs, el, unpacked_element)) => {
                infer_call(env, p, e, explicit_targs, el, unpacked_element)?
            }
            h => rupro_todo!(AST, "{:?}", h),
        };
        Ok(oxidized::aast::Expr(ty, self.1.clone(), e))
    }
}

fn infer_lvar<R: Reason>(
    env: &TEnv<R>,
    id: &oxidized::aast_defs::Lid,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    rupro_todo_mark!(UsingVar);
    rupro_todo_mark!(CheckLocalDefined);
    let pos = R::Pos::from(&id.0);
    let lid = LocalId::from(&id.1);
    let te = oxidized::aast::Expr_::Lvar(Box::new(id.clone()));
    let ty = env.get_local_check_defined(pos, &lid);
    Ok((te, ty))
}

fn infer_binop<R: Reason>(
    env: &mut TEnv<R>,
    pos: R::Pos,
    bop: &oxidized::ast_defs::Bop,
    e1: &oxidized::aast::Expr<(), ()>,
    e2: &oxidized::aast::Expr<(), ()>,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    rupro_todo_mark!(Coeffects, "Typing_local_ops.check_assignment");
    use oxidized::ast_defs::Bop::*;
    match bop {
        Eq(None) => infer_binop_assign(env, pos, e1, e2),
        Eq(Some(..)) => rupro_todo!(AST, "Compound assignment operators"),
        _ => rupro_todo!(AST),
    }
}

fn infer_binop_assign<R: Reason>(
    env: &mut TEnv<R>,
    pos: R::Pos,
    lhs: &oxidized::aast::Expr<(), ()>,
    rhs: &oxidized::aast::Expr<(), ()>,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    let trhs = rhs.infer(env, TCExprParams::default())?;
    let tlhs = infer_assign(env, pos, lhs, &trhs)?;
    rupro_todo_mark!(Holes);
    let ty = trhs.0.clone();
    let res =
        oxidized::aast::Expr_::Binop(Box::new((oxidized::ast_defs::Bop::Eq(None), tlhs, trhs)));
    Ok((res, ty))
}

fn infer_assign<R: Reason>(
    env: &mut TEnv<R>,
    pos: R::Pos,
    lhs: &oxidized::aast::Expr<(), ()>,
    trhs: &tast::Expr<R>,
) -> Result<tast::Expr<R>> {
    rupro_todo_mark!(Holes);
    rupro_todo_mark!(AwaitableAsync);
    rupro_todo_mark!(FakeMembersAndRefinement);
    use oxidized::aast::Expr_::*;
    let (te, ty) = match &lhs.2 {
        Lvar(x) => {
            env.set_local_new_value(false, LocalId::from(&x.1), trhs.0.clone(), pos);
            (Lvar(x.clone()), trhs.0.clone())
        }
        _ => rupro_todo!(AST),
    };
    Ok(oxidized::aast::Expr(ty, lhs.1.clone(), te))
}

fn infer_call<R: Reason>(
    env: &mut TEnv<R>,
    pos: R::Pos,
    e: &oxidized::aast::Expr<(), ()>,
    explicit_targs: &[oxidized::aast::Targ<()>],
    el: &[(oxidized::ast_defs::ParamKind, oxidized::aast::Expr<(), ()>)],
    unpacked_element: &Option<oxidized::aast::Expr<(), ()>>,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    rupro_todo_mark!(MightThrow);
    rupro_todo_mark!(FakeMembersAndRefinement);
    dispatch_call(env, pos, e, explicit_targs, el, unpacked_element)
}

fn dispatch_call<R: Reason>(
    env: &mut TEnv<R>,
    _pos: R::Pos,
    e: &oxidized::aast::Expr<(), ()>,
    explicit_targs: &[oxidized::aast::Targ<()>],
    el: &[(oxidized::ast_defs::ParamKind, oxidized::aast::Expr<(), ()>)],
    unpacked_element: &Option<oxidized::aast::Expr<(), ()>>,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    rupro_todo_mark!(Disposable);

    rupro_todo_mark!(AwaitableAsync, "~allow_awaitable:false");

    use oxidized::aast::Expr_::*;
    match &e.2 {
        Id(id) => {
            rupro_todo_mark!(SpecialFunctions, "needs_special_dispatch");
            dispatch_call_id(env, &e.1, explicit_targs, el, unpacked_element.as_ref(), id)
        }
        ObjGet(box (
            e1,
            oxidized::aast::Expr(_, _, Id(box m)),
            nullflavor,
            oxidized::ast_defs::PropOrMethod::IsMethod,
        )) => dispatch_obj_get_method_id(
            env,
            &e.1,
            e1,
            m,
            nullflavor,
            explicit_targs,
            el,
            unpacked_element,
        ),
        e => rupro_todo!(AST, "{:?}", e),
    }
}

fn dispatch_call_id<R: Reason>(
    env: &mut TEnv<R>,
    expr_pos: &oxidized::pos::Pos,
    explicit_targs: &[oxidized::aast::Targ<()>],
    el: &[(oxidized::ast_defs::ParamKind, oxidized::aast::Expr<(), ()>)],
    unpacked_element: Option<&oxidized::aast::Expr<(), ()>>,
    id: &oxidized::ast_defs::Id,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    rupro_todo_mark!(Disposable);
    let (fty, tal) = fun_type_of_id(env, id, explicit_targs, el)?;
    let (tel, tunpacked_element, ty, _should_forget_fake_members) =
        call(env, fty.clone(), el, unpacked_element)?;

    let te = oxidized::aast::Expr(
        fty,
        expr_pos.clone(),
        oxidized::aast::Expr_::Id(Box::new(id.clone())),
    );
    Ok((
        oxidized::aast::Expr_::Call(Box::new((te, tal, tel, tunpacked_element))),
        ty,
    ))
}

fn dispatch_obj_get_method_id<R: Reason>(
    env: &mut TEnv<R>,
    expr_pos: &oxidized::pos::Pos,
    recv_expr: &oxidized::aast::Expr<(), ()>,
    method_id: &oxidized::ast_defs::Id,
    nullflavor: &oxidized::ast_defs::OgNullFlavor,
    explicit_targs: &[oxidized::aast::Targ<()>],
    el: &[(oxidized::ast_defs::ParamKind, oxidized::aast::Expr<(), ()>)],
    unpacked_element: &Option<oxidized::aast::Expr<(), ()>>,
) -> Result<(tast::Expr_<R>, Ty<R>)> {
    rupro_todo_mark!(MissingError);
    rupro_todo_mark!(Disposable);
    rupro_todo_assert!(
        matches!(nullflavor, oxidized::ast_defs::OgNullFlavor::OGNullthrows),
        Nullsafe
    );
    rupro_todo_assert!(explicit_targs.is_empty(), AST);

    let recv_expr = recv_expr.infer(env, Default::default())?;
    let member_ty = TCObjGet {
        receiver_ty: &recv_expr.0,
        member_id: method_id,
        is_method: true,
        explicit_targs,
    }
    .infer(env, ())?;

    let (tel, typed_unpacked_element, ty, _should_forget_fakes) =
        call(env, member_ty.ty.clone(), el, unpacked_element.as_ref())?;
    let te = oxidized::aast::Expr(
        member_ty.ty.clone(),
        expr_pos.clone(),
        oxidized::aast::Expr_::ObjGet(Box::new((
            recv_expr,
            oxidized::aast::Expr(
                member_ty.ty.clone(),
                method_id.0.clone(),
                oxidized::aast::Expr_::Id(Box::new(method_id.clone())),
            ),
            *nullflavor,
            oxidized::ast_defs::PropOrMethod::IsMethod,
        ))),
    );
    Ok((
        oxidized::aast::Expr_::Call(Box::new((te, member_ty.targs, tel, typed_unpacked_element))),
        ty,
    ))
}

/// Typechecks a call.
///
/// Returns (1) the typed arguments (2) the typed variadic argument
/// (3) the return type (4) a boolean indicating whether fake
/// members should be forgotten.
fn call<R: Reason>(
    env: &mut TEnv<R>,
    fty: Ty<R>,
    el: &[(oxidized::ast_defs::ParamKind, oxidized::aast::Expr<(), ()>)],
    unpacked_element: Option<&oxidized::aast::Expr<(), ()>>,
) -> Result<(
    Vec<(oxidized::ast_defs::ParamKind, tast::Expr<R>)>,
    Option<tast::Expr<R>>,
    Ty<R>,
    bool,
)> {
    rupro_todo_mark!(Dynamic);
    rupro_todo_mark!(GenericParameters, "get_concrete_supertypes");
    rupro_todo_mark!(Solving, "intersect_list, expand_type_and_solve");

    use Ty_::*;
    match &**fty.node() {
        Tfun(ft) => {
            rupro_todo_mark!(FormatStrings, "retype_magic_func");
            rupro_todo_mark!(BidirectionalTC);
            rupro_todo_mark!(Variance);
            rupro_todo_mark!(FakeMembersAndRefinement);
            rupro_todo_mark!(AST, "check arity");
            rupro_todo_mark!(Dynamic);
            rupro_todo_mark!(Coeffects);

            let mut typed_args: Vec<_> = el.iter().map(|_| None).collect();
            let (non_variadic_ft_params, variadic_ft_param) =
                ft.non_variadic_and_variadic_arguments();
            call_check_args(
                env,
                false,
                el,
                non_variadic_ft_params,
                variadic_ft_param,
                &mut typed_args,
            )?;
            call_check_args(
                env,
                true,
                el,
                non_variadic_ft_params,
                variadic_ft_param,
                &mut typed_args,
            )?;
            let typed_args = el
                .iter()
                .map(|x| x.0.clone())
                .zip(typed_args.into_iter().map(|x| x.unwrap()))
                .collect();

            rupro_todo_assert!(unpacked_element.is_none(), AST);
            Ok((typed_args, None, ft.ret.clone(), false))
        }
        _ => rupro_todo!(AST),
    }
}

/// Given an expected function type, check types for the non-unpacked
/// arguments.
///
/// If `check_lambdas` is `false` only non-lambda arguments will be type checked.
/// If it is `true`, only lambda arguments will be type checked.
fn call_check_args<R: Reason>(
    env: &mut TEnv<R>,
    check_lambdas: bool,
    arg_exprs: &[(oxidized::ast_defs::ParamKind, oxidized::aast::Expr<(), ()>)],
    non_variadic_ft_params: &[FunParam<R>],
    variadic_ft_param: Option<&FunParam<R>>,
    results: &mut [Option<tast::Expr<R>>],
) -> Result<()> {
    let mut non_variadic_ft_params = non_variadic_ft_params.iter();
    for (arg_expr, result) in arg_exprs.iter().zip(results.iter_mut()) {
        let (is_variadic, opt_param) = match non_variadic_ft_params.next() {
            Some(param) => (false, Some(param)),
            None => (true, variadic_ft_param),
        };
        let is_lambda = match arg_expr.1.2 {
            oxidized::aast::Expr_::Efun(..) | oxidized::aast::Expr_::Lfun(..) => true,
            _ => false,
        };
        match (check_lambdas, is_lambda) {
            (false, false) | (true, true) => {
                *result = Some(call_check_arg(
                    env,
                    &arg_expr.0,
                    &arg_expr.1,
                    opt_param,
                    is_variadic,
                )?)
            }
            (false, true) => {
                rupro_todo_mark!(Variance);
            }
            (true, false) => {}
        };
    }
    Ok(())
}

fn call_check_arg<R: Reason>(
    env: &mut TEnv<R>,
    param_kind: &oxidized::ast_defs::ParamKind,
    arg_expr: &oxidized::aast::Expr<(), ()>,
    opt_param: Option<&FunParam<R>>,
    is_variadic: bool,
) -> Result<tast::Expr<R>> {
    match opt_param {
        None => rupro_todo!(MissingError),
        Some(param) => {
            rupro_todo_mark!(EnumClasses);
            rupro_todo_mark!(Dynamic);
            rupro_todo_mark!(UsingVar);
            rupro_todo_mark!(MissingError);
            let te = arg_expr.infer(env, Default::default())?;
            check_param(env, param, param_kind, &te, is_variadic)?;
            rupro_todo_mark!(MissingError, "result of subtype check");
            Ok(te)
        }
    }
}

fn check_param<R: Reason>(
    env: &mut TEnv<R>,
    param: &FunParam<R>,
    _param_kind: &oxidized::ast_defs::ParamKind,
    te: &tast::Expr<R>,
    _is_variadic: bool,
) -> Result<()> {
    rupro_todo_mark!(MissingError, "param_modes");
    rupro_todo_mark!(DependentType);
    rupro_todo_mark!(Dynamic);
    let err = env.subtyper().subtype(&te.0, &param.ty)?;
    rupro_todo_assert!(err.is_none(), MissingError);
    Ok(())
}

/// Given an identifier for a function, return its localized type.
fn fun_type_of_id<R: Reason>(
    env: &mut TEnv<R>,
    id: &oxidized::ast_defs::Id,
    tal: &[oxidized::aast::Targ<()>],
    _el: &[(oxidized::ast_defs::ParamKind, oxidized::aast::Expr<(), ()>)],
) -> Result<(Ty<R>, Vec<tast::Targ<R>>)> {
    let fty = env.decls().get_fun(FunName::new(&id.1))?;
    match fty.as_ref().map(|fty| (&**fty, &**fty.ty.node())) {
        None => rupro_todo!(MissingError),
        Some((fty, decl::Ty_::Tfun(ft))) => {
            rupro_todo_mark!(Dynamic);
            rupro_todo_mark!(Idx);
            rupro_todo_mark!(Modules);
            rupro_todo_mark!(MissingError, "deprecated");
            rupro_todo_assert!(tal.is_empty(), AST);
            let ft = ft.infer(
                env,
                LocalizeFunTypeParams {
                    explicit_targs: vec![],
                    localize_env: LocalizeEnv::no_subst(),
                },
            )?;
            let fty = Ty::fun(fty.ty.reason().clone(), ft);
            Ok((fty, vec![]))
        }
        Some((_, _)) => panic!("expected function type"),
    }
}
