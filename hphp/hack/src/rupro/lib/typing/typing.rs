// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::dependency_registrar::DeclName;
use crate::reason::Reason;
use crate::tast;
use crate::typing::typing_trait::TC;
use crate::typing_env::TEnv;
use crate::typing_return::{TypingReturn, TypingReturnInfo};

pub type Result<T, E = Error> = std::result::Result<T, E>;

/// A system error preventing us from proceeding with typechecking. When we
/// encounter one during a bulk typecheck, we should abort the check, report the
/// error to the user, and log the error to Scuba. In some circumstances (e.g.,
/// decl-consistency errors), we might attempt the bulk check again. Includes
/// decl-provider errors like file-not-found (even though it was listed in our
/// global symbol table), decl-consistency errors (i.e., we detected that a
/// source file on disk changed under our feet), etc.
///
/// This type should not be used for internal compiler errors (i.e., invariant
/// violations in our own logic). In OCaml, those are represented as exceptions
/// which are caught at the typing entrypoint and reported as a Hack error
/// (i.e., `Typing_error.invariant_violation`). In Rust, we should represent
/// these with a panic.
#[derive(thiserror::Error, Debug)]
pub enum Error {
    #[error("{0}")]
    DeclProvider(#[from] crate::typing_decl_provider::Error),
    #[error("Decl Not Found: {0:?}")]
    DeclNotFound(DeclName),
}

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

#[derive(Debug)]
pub struct TypingExprFlags<R> {
    _data: std::marker::PhantomData<R>,
}

impl Typing {
    fn block<R: Reason>(
        env: &TEnv<R>,
        block: &[oxidized::aast::Stmt<(), ()>],
    ) -> Result<tast::Block<R>> {
        let mut stl = Vec::new();
        for st in block {
            let st = st.infer(env, ())?;
            match st.1 {
                oxidized::aast::Stmt_::Block(new_stl) => stl.extend(new_stl),
                _ => stl.push(st),
            };
        }
        Ok(stl)
    }

    fn fun_impl<R: Reason>(
        env: &TEnv<R>,
        flags: TypingFunFlags,
        return_: TypingReturnInfo<R>,
        pos: R::Pos,
        named_body: &oxidized::aast::FuncBody<(), ()>,
        f_kind: &oxidized::ast_defs::FunKind,
    ) -> Result<tast::Block<R>> {
        env.set_return(return_);
        let tb = if flags.disable {
            unimplemented!()
        } else {
            Self::block(env, &named_body.fb_ast)?
        };
        let has_implicit_return = env.has_next();
        // TODO(hrust): hhi
        if has_implicit_return && !flags.abstract_ {
            let ret = env.get_return().return_type();
            TypingReturn::fun_implicit_return(env, pos, ret, f_kind);
        }
        // TODO(hrust): set_fun_tast_info
        Ok(tb)
    }

    pub fn fun_<R: Reason>(
        env: &TEnv<R>,
        flags: TypingFunFlags,
        return_: TypingReturnInfo<R>,
        pos: R::Pos,
        named_body: &oxidized::aast::FuncBody<(), ()>,
        f_kind: &oxidized::ast_defs::FunKind,
    ) -> Result<tast::Block<R>> {
        let ret = env.get_return();
        let params = env.get_params();
        let res = Self::fun_impl(env, flags, return_, pos, named_body, f_kind)?;
        env.set_params(params);
        env.set_return(ret);
        Ok(res)
    }
}

impl<R> Default for TypingExprFlags<R> {
    fn default() -> Self {
        Self {
            _data: std::marker::PhantomData,
        }
    }
}
