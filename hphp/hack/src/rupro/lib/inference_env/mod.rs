// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]

mod tyvar_info;
mod tyvar_occurrences;

use crate::reason::Reason;
use crate::typing_defs::Tyvar;
use crate::typing_prop::Prop;
use im::HashMap;
use tyvar_info::TyvarInfo;
use tyvar_occurrences::TyvarOccurrences;

pub struct InferenceEnv<R: Reason> {
    tyvar_info: HashMap<Tyvar, TyvarInfo<R>>,
    occurrences: TyvarOccurrences,
    subtype_prop: Prop<R>,
}
