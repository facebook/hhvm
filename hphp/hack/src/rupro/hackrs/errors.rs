// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::parsing_error::ParsingError;
use ty::local_error::TypingError;
use ty::reason::Reason;

#[derive(Debug)]
pub enum HackError<R: Reason> {
    Parsing(ParsingError),
    Typing(TypingError<R>),
}

impl<R: Reason> From<ParsingError> for HackError<R> {
    fn from(e: ParsingError) -> Self {
        Self::Parsing(e)
    }
}

impl<R: Reason> From<TypingError<R>> for HackError<R> {
    fn from(e: TypingError<R>) -> Self {
        Self::Typing(e)
    }
}
