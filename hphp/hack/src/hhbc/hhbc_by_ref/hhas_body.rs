// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ffi::{Maybe, Pair, Slice, Str};
use hhbc_by_ref_hhas_param::HhasParam;
use hhbc_by_ref_hhas_type::Info;
use hhbc_by_ref_hhbc_ast::ClassishKind;
use hhbc_by_ref_instruction_sequence::InstrSeq;

#[derive(Default, Debug, Clone)]
#[repr(C)]
pub struct HhasBodyEnv<'arena> {
    pub is_namespaced: bool,
    pub class_info: Maybe<Pair<ClassishKind, Str<'arena>>>,
    pub parent_name: Maybe<Str<'arena>>,
}

#[derive(Debug)] //Cannot derive `Default`...
// TODO(SF, 2021-08-05): Enable this when D30115681 lands.
//#[repr(C)]
pub struct HhasBody<'arena> {
    pub body_instrs: InstrSeq<'arena>, //... because no `Default` instance for `InstrSeq`.
    pub decl_vars: Slice<'arena, Str<'arena>>,
    pub num_iters: usize,
    pub num_closures: u32,
    pub is_memoize_wrapper: bool,
    pub is_memoize_wrapper_lsb: bool,
    pub upper_bounds: Slice<'arena, Pair<Str<'arena>, Slice<'arena, Info<'arena>>>>,
    pub shadowed_tparams: Slice<'arena, Str<'arena>>,
    pub params: Slice<'arena, HhasParam<'arena>>,
    pub return_type_info: Maybe<Info<'arena>>,
    pub doc_comment: Maybe<Str<'arena>>,
    pub env: Maybe<HhasBodyEnv<'arena>>,
}

pub fn default_with_body_instrs<'arena>(body_instrs: InstrSeq<'arena>) -> HhasBody<'arena> {
    HhasBody {
        body_instrs,
        decl_vars: Slice::<'arena, Str<'arena>>::default(),
        num_iters: usize::default(),
        num_closures: u32::default(),
        is_memoize_wrapper: bool::default(),
        is_memoize_wrapper_lsb: bool::default(),
        upper_bounds: Slice::<'arena, Pair<Str<'arena>, Slice<'arena, Info>>>::default(),
        shadowed_tparams: Slice::<'arena, Str<'arena>>::default(),
        params: Slice::<'arena, HhasParam<'arena>>::default(),
        return_type_info: Maybe::<Info>::default(),
        doc_comment: Maybe::<Str<'arena>>::default(),
        env: Maybe::<HhasBodyEnv<'arena>>::default(),
    }
}

// For cbindgen
#[allow(clippy::needless_lifetimes)]
#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_hhas_body<'arena>(
    _: HhasBodyEnv<'arena>,
    // TODO(SF, 2021-08-05): Enable this and add to cbindgen.sh when D30115681 lands.
    //  _: HhasBody<'arena>,
) {
    unimplemented!()
}
