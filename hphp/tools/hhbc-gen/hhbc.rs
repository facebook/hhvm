// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// NOTE: Most of the types in this file come from runtime/vm/hhbc.h and need to
// be kept in sync.

mod opcodes;

pub use opcodes::opcode_data;

#[derive(Clone, Debug)]
pub enum FlavorDesc {
    CUV,
    CV,
    UV,
}

#[derive(Clone, Debug)]
pub enum ImmType {
    AA,
    BA,
    BLA,
    DA,
    FCA,
    I64A,
    IA,
    ILA,
    ITA,
    IVA,
    KA,
    LA,
    LAR,
    NA,
    NLA,
    OA(&'static str),
    RATA,
    SA,
    SLA,
    VSA,
}

#[derive(Clone, Debug)]
pub enum Inputs {
    NOV,
    Fixed(Box<[FlavorDesc]>),
    SMany,
    CMany,
    CUMany,
    MFinal,
    CMFinal(i64),
    FCall { inp: i64, obj: i64 },
}

#[derive(Clone, Debug)]
pub enum Outputs {
    NOV,
    Fixed(Box<[FlavorDesc]>),
    FCall,
}

#[allow(non_camel_case_types)]
#[derive(Clone, Debug)]
pub enum InstrFlags {
    NF,
    TF,
    CF,
    CF_TF,
}

#[derive(Clone, Debug)]
pub struct OpcodeData {
    pub name: &'static str,
    pub immediates: Vec<(&'static str, ImmType)>,
    pub inputs: Inputs,
    pub outputs: Outputs,
    pub flags: InstrFlags,
}
