// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::ast;

use crate::Error;
use crate::Result;

pub fn expect_normal_paramkind(arg: &ast::Argument) -> Result<&ast::Expr> {
    match arg {
        ast::Argument::Anormal(e) => Ok(e),
        ast::Argument::Ainout(pk_pos, _) => {
            Err(Error::fatal_parse(pk_pos, "Unexpected `inout` annotation"))
        }
    }
}

pub fn expect_normal_paramkind_mut(arg: &mut ast::Argument) -> Result<&mut ast::Expr> {
    match arg {
        ast::Argument::Anormal(e) => Ok(e),
        ast::Argument::Ainout(pk_pos, _) => {
            Err(Error::fatal_parse(pk_pos, "Unexpected `inout` annotation"))
        }
    }
}

pub fn ensure_normal_paramkind(arg: &ast::Argument) -> Result<()> {
    expect_normal_paramkind(arg).map(|_| ())
}
