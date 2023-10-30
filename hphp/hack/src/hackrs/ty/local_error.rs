// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod error_code;
mod error_primary;
mod error_reason;

use eq_modulo_pos::EqModuloPos;
pub use error_code::TypingErrorCode;
pub use error_primary::Primary;
pub use error_reason::ReasonsCallback;
use serde::Deserialize;
use serde::Serialize;

use crate::decl_error::DeclError;
use crate::reason::Reason;

#[derive(Clone, Debug)]
pub struct ErrorMessage<P>(P, String);

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[serde(bound = "R: Reason")]
pub enum TypingError<R: Reason> {
    Primary(Primary<R>),
}

impl<R: Reason> TypingError<R> {
    pub fn primary(primary: Primary<R>) -> Self {
        TypingError::Primary(primary)
    }
}

impl<R: Reason> From<&DeclError<R::Pos>> for TypingError<R> {
    fn from(decl_error: &DeclError<R::Pos>) -> Self {
        match decl_error {
            &DeclError::WrongExtendKind {
                ref pos,
                kind,
                name,
                ref parent_pos,
                parent_kind,
                parent_name,
            } => Self::Primary(Primary::WrongExtendKind {
                pos: pos.clone(),
                kind,
                name,
                parent_pos: parent_pos.clone(),
                parent_kind,
                parent_name,
            }),
            &DeclError::WrongUseKind {
                ref pos,
                name,
                ref parent_pos,
                parent_name,
            } => Self::Primary(Primary::WrongUseKind {
                pos: pos.clone(),
                name,
                parent_pos: parent_pos.clone(),
                parent_name,
            }),
            DeclError::CyclicClassDef(pos, stack) => {
                Self::Primary(Primary::CyclicClassDef(pos.clone(), stack.clone()))
            }
        }
    }
}
