// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use pos::Symbol;

use crate::reason::Reason;
use crate::tast;
use crate::typing_defs::{ParamMode, Ty};
use crate::typing_env::TEnv;
use crate::typing_return::TypingReturnInfo;
use crate::utils::core::LocalId;

pub struct Typing;

pub struct BindParamFlags {
    pub immutable: bool,
    pub can_read_globals: bool,
}

#[derive(Default)]
pub struct TypingFunFlags {
    pub abstract_: bool,
    pub disable: bool,
}

impl Typing {
    pub fn bind_param<R: Reason>(
        env: &TEnv<R>,
        flags: BindParamFlags,
        ty1: Ty<R>,
        param: &oxidized::aast::FunParam<(), ()>,
    ) -> tast::FunParam<R> {
        let (param_te, ty1) = match param.expr {
            None => (None, ty1),
            Some(_) => unimplemented!(),
        };
        if !param.user_attributes.is_empty() {
            unimplemented!()
        }
        let name = Symbol::new(&param.name);
        let pos = env.ctx.alloc.pos_from_ast(&param.pos);
        let id = LocalId::new_unscoped(name);
        let param_mode = ParamMode::from(&param.callconv);
        env.set_param(id.clone(), ty1.clone(), pos.clone(), param_mode);
        env.set_local(flags.immutable, id, ty1.clone(), pos);
        if !param.user_attributes.is_empty() {
            unimplemented!()
        }
        tast::FunParam {
            annotation: ty1.clone(),
            type_hint: oxidized::aast::TypeHint(ty1, param.type_hint.1.clone()),
            is_variadic: param.is_variadic,
            pos: param.pos.clone(),
            name: param.name.clone(),
            expr: param_te,
            readonly: param.readonly,
            callconv: param.callconv.clone(),
            user_attributes: vec![],
            visibility: param.visibility,
        }
    }

    fn stmt_<R: Reason>(
        _env: &TEnv<R>,
        _pos: &oxidized::pos::Pos,
        stmt: &oxidized::aast::Stmt_<(), ()>,
    ) -> tast::Stmt_<R> {
        use oxidized::aast::Stmt_ as OS;
        use tast::Stmt_ as TS;
        match stmt {
            OS::Noop => TS::Noop,
            s => unimplemented!("stmt_: {:?}", s),
        }
    }

    fn stmt<R: Reason>(env: &TEnv<R>, stmt: &oxidized::aast::Stmt<(), ()>) -> tast::Stmt<R> {
        let pos = &stmt.0;
        let st = Self::stmt_(env, pos, &stmt.1);
        oxidized::aast::Stmt(pos.clone(), st)
    }

    fn block<R: Reason>(env: &TEnv<R>, block: &[oxidized::aast::Stmt<(), ()>]) -> tast::Block<R> {
        let mut stl = Vec::new();
        for st in block {
            let st = Self::stmt(env, st);
            match st.1 {
                oxidized::aast::Stmt_::Block(new_stl) => stl.extend(new_stl),
                _ => stl.push(st),
            };
        }
        stl
    }


    fn fun_impl<R: Reason>(
        env: &TEnv<R>,
        flags: TypingFunFlags,
        return_: TypingReturnInfo<R>,
        _pos: R::Pos,
        named_body: &oxidized::aast::FuncBody<(), ()>,
        _f_kind: &oxidized::ast_defs::FunKind,
    ) -> tast::Block<R> {
        env.set_return(return_);
        let tb = if flags.disable {
            unimplemented!()
        } else {
            Self::block(env, &named_body.fb_ast)
        };
        let has_implicit_return = env.has_next();
        // TODO(hrust): hhi
        if has_implicit_return && !flags.abstract_ {
            // TODO(hrust): fun_implicit_return
            unimplemented!()
        }
        // TODO(hrust): set_fun_tast_info
        tb
    }

    pub fn fun_<R: Reason>(
        env: &TEnv<R>,
        flags: TypingFunFlags,
        return_: TypingReturnInfo<R>,
        pos: R::Pos,
        named_body: &oxidized::aast::FuncBody<(), ()>,
        f_kind: &oxidized::ast_defs::FunKind,
    ) -> tast::Block<R> {
        let ret = env.get_return();
        let params = env.get_params();
        let res = Self::fun_impl(env, flags, return_, pos, named_body, f_kind);
        env.set_params(params);
        env.set_return(ret);
        res
    }
}
