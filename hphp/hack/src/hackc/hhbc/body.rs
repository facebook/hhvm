// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ffi::Maybe;
use ffi::Vector;
pub use hhvm_types_ffi::ffi::Attr;
use serde::Serialize;

use crate::Attribute;
use crate::ClassName;
use crate::Coeffects;
use crate::Instruct;
use crate::Label;
use crate::Param;
use crate::Span;
use crate::StringId;
use crate::TypeInfo;
use crate::UpperBound;

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct Body {
    pub attributes: Vector<Attribute>,
    pub attrs: Attr,
    pub coeffects: Coeffects,
    pub num_iters: usize,
    pub is_memoize_wrapper: bool,
    pub is_memoize_wrapper_lsb: bool,
    pub upper_bounds: Vector<UpperBound>,
    /// shadowed_tparams are the set of tparams on a method which shadow a
    /// tparam on the containing class.
    pub shadowed_tparams: Vector<ClassName>,
    pub return_type: Maybe<TypeInfo>,
    pub doc_comment: Maybe<Vector<u8>>,
    pub span: Span,
    pub repr: BcRepr,
}

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct BcRepr {
    /// Must have been compacted with InstrSeq::compact_iter().
    pub instrs: Vector<Instruct>,
    pub decl_vars: Vector<StringId>,

    /// The statically computed stack depth for this Body. This can be computed
    /// using the hhbc::compute_stack_depth() function.
    pub stack_depth: usize,
    pub params: Vector<ParamEntry>,
}

#[derive(Debug, Serialize)]
#[repr(C)]
pub struct ParamEntry {
    pub param: Param,
    pub dv: Maybe<DefaultValue>,
}

#[derive(Clone, Debug, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct DefaultValue {
    pub label: Label,
    pub expr: Vector<u8>,
}
