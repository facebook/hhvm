// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod error_code;
mod error_primary;
mod error_reason;

pub use error_code::TypingErrorCode;
pub use error_primary::Primary;
pub use error_reason::ReasonsCallback;

use crate::reason::Reason;
use serde::{Deserialize, Serialize};

#[derive(Clone, Debug)]
pub struct ErrorMessage<P>(P, String);

#[derive(Clone, Debug, Eq, Hash, PartialEq, Serialize, Deserialize)]
#[serde(bound = "R: Reason")]
pub enum TypingError<R: Reason> {
    Primary(Primary<R>),
}

impl<R: Reason> TypingError<R> {
    pub fn primary(primary: Primary<R>) -> Self {
        TypingError::Primary(primary)
    }
}
