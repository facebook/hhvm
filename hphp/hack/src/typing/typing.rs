// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::typing_phase::{self, MethodInstantiation};
use crate::typing_subtype;
use crate::{Env, LocalId, ParamMode};
use arena_trait::Arena;
use oxidized::typing_defs_core::Ty_ as DTy_;
use oxidized::{ast, pos::Pos};
use typing_defs_rust::typing_reason::PReason_;
use typing_defs_rust::{tast, FunParam, FuncBodyAnn, SavedEnv, Ty, Ty_};

pub fn fun<'a>(env: &mut Env<'a>, f: &'a ast::Fun_) -> tast::Fun_<'a> {
    let ast = f.body.ast.iter().map(|x| stmt(env, x)).collect();

    // We put empty vec below for all of those, since real conversion is unimplemented
    assert!(f.user_attributes.is_empty());
    assert!(f.file_attributes.is_empty());

    let ret = tast::TypeHint(env.builder().null(env.builder().mk_rnone()), None);

    tast::Fun_ {
        span: f.span.clone(),
        annotation: SavedEnv,
        mode: f.mode,
        ret,
        name: f.name.clone(),
        tparams: vec![],
        where_constraints: f.where_constraints.clone(),
        body: tast::FuncBody {
            ast,
            annotation: FuncBodyAnn,
        },
        fun_kind: f.fun_kind,
        variadic: tast::FunVariadicity::FVnonVariadic,
        params: vec![],
        user_attributes: vec![],
        file_attributes: vec![],
        external: f.external,
        namespace: f.namespace.clone(),
        doc_comment: f.doc_comment.clone(),
        static_: f.static_,
    }
}

pub fn stmt<'a>(env: &mut Env<'a>, ast::Stmt(pos, s): &'a ast::Stmt) -> tast::Stmt<'a> {
    tast::Stmt(pos.clone(), stmt_(env, s))
}

fn stmt_<'a>(env: &mut Env<'a>, s: &'a ast::Stmt_) -> tast::Stmt_<'a> {
    match s {
        ast::Stmt_::Noop => tast::Stmt_::Noop,
        ast::Stmt_::Markup(x) => markup(&x),
        ast::Stmt_::Expr(x) => tast::Stmt_::mk_expr(expr(env, x)),
        ast::Stmt_::Return(e) => match e.as_ref() {
            None => unimplemented!("return; not yet implemented"),
            Some(e) => {
                let te = expr(env, e);
                typing_subtype::sub_type(env, (te.0).1, env.genv.return_info.type_.type_);
                tast::Stmt_::mk_return(Some(te))
            }
        },
        x => unimplemented!("{:#?}", x),
    }
}

fn markup<'a>(s: &ast::Pstring) -> tast::Stmt_<'a> {
    tast::Stmt_::mk_markup(s.clone())
}

fn expr<'a>(env: &mut Env<'a>, ast::Expr(pos, e): &'a ast::Expr) -> tast::Expr<'a> {
    let (ty, e) = match e {
        ast::Expr_::True => {
            let ty = env.bld().bool(env.bld().mk_rwitness(pos));
            let e = tast::Expr_::True;
            (ty, e)
        }
        ast::Expr_::False => {
            let ty = env.bld().bool(env.bld().mk_rwitness(pos));
            let e = tast::Expr_::False;
            (ty, e)
        }
        ast::Expr_::Int(s) => {
            let ty = env.bld().int(env.bld().mk_rwitness(pos));
            let e = tast::Expr_::Int(s.clone());
            (ty, e)
        }
        ast::Expr_::Float(s) => {
            let ty = env.bld().float(env.bld().mk_rwitness(pos));
            let e = tast::Expr_::Float(s.clone());
            (ty, e)
        }
        ast::Expr_::Null => {
            let ty = env.bld().null(env.bld().mk_rwitness(pos));
            let e = tast::Expr_::Null;
            (ty, e)
        }
        ast::Expr_::String(s) => {
            let ty = env.bld().string(env.bld().mk_rwitness(pos));
            let e = tast::Expr_::String(s.clone());
            (ty, e)
        }
        ast::Expr_::Call(x) => {
            // TODO(hrust) pseudo functions, might_throw
            let (call_type, e, explicit_targs, el, unpacked_element) = &**x;
            let in_suspend = false;
            check_call(
                env,
                pos,
                call_type,
                e,
                explicit_targs,
                el,
                unpacked_element,
                in_suspend,
            )
        }
        x => unimplemented!("{:#?}", x),
    };
    // TODO(hrust) set_tyvar_variance
    ast::Expr((&pos, ty), e)
}

fn check_call<'a>(
    env: &mut Env<'a>,
    pos: &Pos,
    call_type: &ast::CallType,
    e: &'a ast::Expr,
    explicit_targs: &Vec<ast::Targ>,
    el: &'a Vec<ast::Expr>,
    unpacked_element: &Option<ast::Expr>,
    in_suspend: bool,
) -> (Ty<'a>, tast::Expr_<'a>) {
    dispatch_call(
        env,
        pos,
        call_type,
        e,
        explicit_targs,
        el,
        unpacked_element,
        in_suspend,
    )
    // TODO(hrust) forget_fake_members
}

fn dispatch_call<'a>(
    env: &mut Env<'a>,
    pos: &Pos,
    _call_type: &ast::CallType,
    ast::Expr(fpos, fun_expr): &'a ast::Expr,
    explicit_targs: &Vec<ast::Targ>,
    el: &'a Vec<ast::Expr>,
    unpacked_element: &Option<ast::Expr>,
    _in_suspend: bool,
) -> (Ty<'a>, tast::Expr_<'a>) {
    match fun_expr {
        ast::Expr_::Id(x) => {
            let (fty, targs) = fun_type_of_id(env, x, explicit_targs, el);
            let (tel, rty) = call(env, pos, fty, el, unpacked_element);
            // TODO(hrust) reactivity stuff
            let fte = tast::Expr((fpos, fty), tast::Expr_::Id(x.clone()));
            (
                rty,
                tast::Expr_::mk_call(tast::CallType::Cnormal, fte, targs, tel, None),
            )
        }
        x => unimplemented!("{:#?}", x),
    }
}

fn fun_type_of_id<'a>(
    env: &mut Env<'a>,
    ast::Id(pos, id): &'a ast::Sid,
    targs: &Vec<ast::Targ>,
    _el: &'a Vec<ast::Expr>,
) -> (Ty<'a>, Vec<tast::Targ<'a>>) {
    let bld = env.builder();
    match env.provider().get_fun(id) {
        None => unimplemented!(),
        Some(f) => {
            match f.type_.get_node() {
                DTy_::Tfun(ft) => {
                    // TODO(hrust) transform_special_fun_ty
                    let ety_env = bld.env_with_self();
                    // TODO(hrust) below: strip_ns id
                    let targs = typing_phase::localize_targs(env, pos, id, &ft.tparams, targs);
                    // TODO(hrust) pessimize
                    let instantiation = MethodInstantiation {
                        use_pos: pos,
                        use_name: id,
                        explicit_targs: &targs,
                    };
                    let ft = typing_phase::localize_ft(ety_env, env, ft, Some(instantiation));
                    let fty = bld.fun(bld.alloc(PReason_::from(f.type_.get_reason())), ft);
                    // TODO(hrust) check deprecated
                    (fty, targs)
                }
                _ => panic!("Expected function type"),
            }
        }
    }
}

fn call<'a>(
    env: &mut Env<'a>,
    _pos: &Pos,
    fty: Ty<'a>,
    el: &'a Vec<ast::Expr>,
    _unpacked_element: &Option<ast::Expr>,
) -> (Vec<tast::Expr<'a>>, Ty<'a>) {
    // TODO(hrust) missing bits
    match fty.get_node() {
        Ty_::Tfun(ft) => {
            // TODO(hrust) retype magic fun, variadic param, expected ty, set tyvar variance
            let tel = el
                .iter()
                .zip(&ft.params)
                .map(|(e, param)| check_arg(env, e, param))
                .collect();
            // TODO(hrust) rx check_call, unpacked element, arity, inout, rx adjusted return
            (tel, ft.return_)
        }
        x => unimplemented!("{:#?}", x),
    }
}

fn check_arg<'a>(env: &mut Env<'a>, e: &'a ast::Expr, param: &FunParam<'a>) -> tast::Expr<'a> {
    // TODO(hrust) derive expected arg
    let te = expr(env, e);
    call_param(env, &te, param);
    te
}

fn call_param<'a>(_env: &mut Env<'a>, _te: &tast::Expr, _param: &FunParam<'a>) -> () {
    // TODO(hrust) param_modes, dep_ty, coercion
    // TODO(hrust) call into subtyping here
}

#[allow(dead_code)]
fn bind_param<'a>(env: &mut Env<'a>, ty1: Ty<'a>, param: &'a ast::FunParam) -> tast::FunParam<'a> {
    // TODO(hrust): if parameter has a default initializer, check its type
    let param_te: Option<tast::Expr<'a>> = match &param.expr {
        None => None,
        Some(e) => unimplemented!("{:?}", e),
    };
    // TODO(hrust): process user attributes
    let user_attributes: Vec<tast::UserAttribute<'a>> = if param.user_attributes.len() > 0 {
        unimplemented!("{:?}", param.user_attributes)
    } else {
        vec![]
    };
    let tparam = tast::FunParam {
        annotation: (&param.pos, ty1),
        type_hint: ast::TypeHint(ty1, param.type_hint.get_hint().clone()),
        is_variadic: param.is_variadic,
        pos: param.pos.clone(),
        name: param.name.clone(),
        expr: param_te,
        callconv: param.callconv,
        user_attributes,
        visibility: param.visibility,
    };
    // TODO(hrust): add type to locals env
    let mode = ParamMode::from(param.callconv);
    let id = LocalId::make_unscoped(&param.name);
    env.set_param(id, (ty1, mode));
    // TODO(hrust): has_accept_disposable_attribute
    // TODO(hrust): mutability
    tparam
}
