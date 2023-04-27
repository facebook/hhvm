// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(dead_code)]

use utils::Lazy;

use crate::local_error::Primary;
use crate::local_error::TypingError;
use crate::local_error::TypingErrorCode;
use crate::reason::Reason;

#[derive(Clone, Debug)]
enum Component {
    Code,
    Reasons,
    Quickfixes,
}

#[derive(Clone, Debug)]
enum Enum<R: Reason> {
    Ignore,
    Always(TypingError<R>),
    OfError(TypingError<R>),
    WithCode(Box<Inner<R>>, TypingErrorCode),
    Retain(Box<Inner<R>>, Component),
}

// Wrapping here allows Enum variants and their fields to remain private.
#[derive(Clone, Debug)]
pub struct Inner<R: Reason>(Enum<R>);

#[derive(Clone, Debug)]
pub struct ReasonsCallback<'a, R: Reason>(Lazy<'a, Inner<R>>);

impl<R: Reason> From<Enum<R>> for Inner<R> {
    fn from(other: Enum<R>) -> Self {
        Self(other)
    }
}

impl<R: Reason> From<Inner<R>> for Enum<R> {
    fn from(other: Inner<R>) -> Self {
        other.0
    }
}

impl<'a, R: Reason + 'a> ReasonsCallback<'a, R> {
    pub fn new(mk: &'a dyn Fn() -> Inner<R>) -> Self {
        Self(Lazy::new(mk))
    }

    pub fn ignore() -> Inner<R> {
        Enum::Ignore.into()
    }

    pub fn invalid_type_hint(pos: R::Pos) -> Inner<R> {
        Self::retain_quickfixes(Self::of_primary_error(Primary::InvalidTypeHint(pos)))
    }

    fn of_primary_error(prim_err: Primary<R>) -> Inner<R> {
        Enum::OfError(TypingError::primary(prim_err)).into()
    }

    fn retain_quickfixes(cb: Inner<R>) -> Inner<R> {
        Enum::Retain(Box::new(cb), Component::Quickfixes).into()
    }
}
