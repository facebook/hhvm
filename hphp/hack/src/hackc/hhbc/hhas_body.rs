// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ffi::{Maybe, Pair, Slice, Str};
use hhas_param::HhasParam;
use hhas_type::HhasTypeInfo;
use hhbc_ast::{ClassishKind, Instruct};

#[derive(Default, Debug, Clone)]
#[repr(C)]
pub struct HhasBodyEnv<'arena> {
    pub is_namespaced: bool,
    pub class_info: Maybe<Pair<ClassishKind, Str<'arena>>>,
    pub parent_name: Maybe<Str<'arena>>,
}

#[derive(Debug, Default)]
#[repr(C)]
pub struct HhasBody<'arena> {
    /// Must have been compacted with InstrSeq::compact_iter().
    pub body_instrs: Slice<'arena, Instruct<'arena>>,
    pub decl_vars: Slice<'arena, Str<'arena>>,
    pub num_iters: usize,
    pub is_memoize_wrapper: bool,
    pub is_memoize_wrapper_lsb: bool,
    pub upper_bounds: Slice<'arena, Pair<Str<'arena>, Slice<'arena, HhasTypeInfo<'arena>>>>,
    pub shadowed_tparams: Slice<'arena, Str<'arena>>,
    pub params: Slice<'arena, HhasParam<'arena>>,
    pub return_type_info: Maybe<HhasTypeInfo<'arena>>,
    pub doc_comment: Maybe<Str<'arena>>,
    pub env: Maybe<HhasBodyEnv<'arena>>,
}
