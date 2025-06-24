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

/// A hhbc bytecode body
pub type Body = BodyImpl<BcRepr>;

/// Information for template params that are defined for this method.
#[derive(Debug, Clone, Eq, PartialEq, Serialize)]
#[repr(C)]
pub struct TParamInfo {
    pub name: ClassName,
    /// true if this template param shadows a template param from the
    /// containing class.
    pub shadows_class_tparam: bool,
}

/// A BodyImpl represents a body of code. It's used for both Function and Method.
#[derive(Debug, Default, Clone, Serialize)]
#[repr(C)]
pub struct BodyImpl<R> {
    pub attributes: Vector<Attribute>,
    pub attrs: Attr,
    pub coeffects: Coeffects,
    pub num_iters: usize,
    pub is_memoize_wrapper: bool,
    pub is_memoize_wrapper_lsb: bool,
    pub upper_bounds: Vector<UpperBound>,
    pub tparam_info: Vector<TParamInfo>,
    pub return_type: Maybe<TypeInfo>,
    pub doc_comment: Maybe<Vector<u8>>,
    pub span: Span,
    pub repr: R,
}

impl<R> BodyImpl<R> {
    pub fn is_reified(&self) -> bool {
        self.attributes
            .iter()
            .any(|attr| attr.name.as_str() == "__Reified")
    }

    pub fn return_type(&self) -> TypeInfo {
        match &self.return_type {
            Maybe::Just(t) => t.clone(),
            Maybe::Nothing => TypeInfo::empty(),
        }
    }
}

// TODO: Steps to unify BcRepr with IrRepr:
//
// 1. Locations. Add a LocId field to the hhbc::Instruct::Opcode variant
// so every Opcode has a SrcLoc. LocId indexes into a new locs table,
// similar to IrRepr::locs. Assign LocIds in InstrSeq::to_vec() so the results
// does not contain any inline Pseudo::SrcLoc instructions. If this step is
// not bytecode-preserving, dig into how SrcLocs differ and debug; maybe the
// new way is more correct.
//
// 2. Blocks. repurpose make_cfg() in opt_body.rs to create a vector of basic
// blocks of type Vec<InstrId> where instrId is the index into BcRepr::instrs.
// Change BcRepr::instrs to IdVec<InstrId,Instruct> for type safety. Modify
// hackc-translator, bc_to_ir, and bytecode_printer to traverse blocks and
// instructions. The order of instructions in BcRepr::instrs no longer matters.
// If this step isn't bytecode preserving, decide if the new block order is better.
//
// 3. ExFrames. Construct ex_frames during or after constructing blocks by
// processing in-line TryBegin/TryMiddle/TryEnd pseudo instructions.
// Now every block refers to an exception scope. passes that linearize bytecode
// (bytecode_printer, hackc-translator, bc_to_ir) need to first honor exception
// frames, then place linear blocks a local good order (probably RPO) in nested
// try/catch scopes. If this step is not bytecode preserving, make sure blocks
// or instructions haven't moved in/out of their correct try/catch scope.
//
// By this point, enum Pseudo should be totally unused in BcRepr. enum Instruct
// (opcode or pseudo) is now only needed for InstrSeq; BcRepr instrs can be
// reduced to just Vector<(Opcode, LocId)>.

#[derive(Debug, Clone, Serialize)]
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

#[derive(Debug, Clone, Serialize)]
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
