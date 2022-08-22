// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hash::HashMap;
use hhbc::Adata;
use hhbc::TypedValue;

#[derive(Debug, Default)]
pub struct AdataState<'arena> {
    pub array_identifier_counter: usize,
    pub array_identifier_map: HashMap<TypedValue<'arena>, &'arena str>,
    pub adata: Vec<Adata<'arena>>,
}
