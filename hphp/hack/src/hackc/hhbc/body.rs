// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Slice;
use ffi::Str;
use ffi::Vector;
use serde::Serialize;

use crate::Instruct;
use crate::Param;
use crate::TypeInfo;
use crate::UpperBound;

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Body<'arena> {
    /// Must have been compacted with InstrSeq::compact_iter().
    pub body_instrs: Vector<Instruct<'arena>>,
    pub decl_vars: Vector<Str<'arena>>,
    pub num_iters: usize,
    pub is_memoize_wrapper: bool,
    pub is_memoize_wrapper_lsb: bool,
    pub upper_bounds: Slice<'arena, UpperBound<'arena>>,
    pub shadowed_tparams: Vector<Str<'arena>>,
    pub params: Vector<Param<'arena>>,
    pub return_type_info: Maybe<TypeInfo<'arena>>,
    pub doc_comment: Maybe<Str<'arena>>,
    /// The statically computed stack depth for this Body. This can be computed
    /// using the hhbc::compute_stack_depth() function.
    pub stack_depth: usize,
}
