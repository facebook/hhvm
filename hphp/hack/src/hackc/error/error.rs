// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod assertion_utils;

pub use assertion_utils::*;
use hhvm_hhbc_defs_ffi::ffi::FatalOp;
use oxidized::ast_defs::Pos;
use thiserror::Error;

pub type Result<T, E = Error> = std::result::Result<T, E>;

#[derive(Error, Debug)]
#[error(transparent)]
pub struct Error(Box<ErrorKind>);

impl Error {
    pub fn unrecoverable(msg: impl Into<String>) -> Self {
        Self(Box::new(ErrorKind::Unrecoverable(msg.into())))
    }

    pub fn fatal_runtime(pos: &Pos, msg: impl Into<String>) -> Self {
        Self(Box::new(ErrorKind::IncludeTimeFatalException(
            FatalOp::Runtime,
            pos.clone(),
            msg.into(),
        )))
    }

    pub fn fatal_parse(pos: &Pos, msg: impl Into<String>) -> Self {
        Self(Box::new(ErrorKind::IncludeTimeFatalException(
            FatalOp::Parse,
            pos.clone(),
            msg.into(),
        )))
    }

    pub fn kind(&self) -> &ErrorKind {
        &self.0
    }

    pub fn into_kind(self) -> ErrorKind {
        *self.0
    }
}

#[derive(Error, Debug)]
pub enum ErrorKind {
    #[error("IncludeTimeFatalException: FatalOp={0:?}, {1}")]
    IncludeTimeFatalException(FatalOp, Pos, std::string::String),

    #[error("Unrecoverable: {0}")]
    Unrecoverable(std::string::String),
}
