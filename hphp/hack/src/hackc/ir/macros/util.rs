// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::Ident;

use crate::simple_type::SimpleType;

pub(crate) enum InterestingFields<'a> {
    None,
    One(usize, Option<&'a Ident>, SimpleType<'a>),
    Many,
}

impl<'a> InterestingFields<'a> {
    pub(crate) fn add(&mut self, idx: usize, ident: Option<&'a Ident>, ty: SimpleType<'a>) {
        *self = match self {
            InterestingFields::None => InterestingFields::One(idx, ident, ty),
            InterestingFields::One(..) | InterestingFields::Many => InterestingFields::Many,
        }
    }
}
