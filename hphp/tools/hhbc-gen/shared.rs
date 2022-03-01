// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(non_camel_case_types)]

// immediates actually contain operand values used by the instruction
// These can be plain aliases to enums & structs from hhvm_hhbc_defs.

pub struct BLA(Box<[BA]>); // Bytecode offset vector immediate
pub struct SLA(Box<[(SA, BA)]>); // String id/offset pair vector
pub struct IVA(u32); // 8 or 32-bit uint
pub struct I64A(i64); // 64-bit Integer
pub struct LA(u32); // Local: 8 or 32-bit uint
pub struct NLA(u32, u32); // NamedLocal: Local w/ name: 2x 8 or 32-bit int
pub struct ILA(u32); // Local w/ id: 8 or 32-bit int
pub struct IA(u32); // Iterator id: 8 or 32-bit uint
pub struct DA(f64); // Double
pub struct SA(u32); // Static string Id
pub struct AA(u32); // Static array Id
pub struct RATA(u8, u32); // Statically inferred RepoAuthType
pub struct BA(i32); // Offset: Bytecode offset (maybe LabelId)
pub struct OA<T>(T); // Marker for 1-byte sub-opcode
pub struct KA(i32); // MemberKey: local, stack, int, str
pub struct LAR(LA, u32); // LocalRange: Contiguous range of locals
pub struct ITA(u8, IA, LA, LA); // IterArgs
pub struct FCA(u16, u32, u32, BA, SA, Box<[bool]>, Box<[bool]>); // FCallArgs
pub struct VSA(Box<[SA]>); // Vector of static string ids

pub struct CollectionType(u8);
pub struct TypeStructResolveOp(u8);
pub struct FatalOp(u8);
pub struct SwitchKind(u8);
pub struct ReadonlyOp(u8);
pub struct IsTypeOp(u8);
pub struct SetOpOp(u8);
pub struct IncDecOp(u8);
pub struct SpecialClsRef(u8);
pub struct IsLogAsDynamicCallOp(u8);
pub struct ObjMethodOp(u8);
pub struct BareThisOp(u8);
pub struct OODeclExistsOp(u8);
pub struct ContCheckOp(u8);
pub struct MOpMode(u8);
pub struct InitPropOp(u8);
pub struct SilenceOp(u8);
pub struct QueryMOp(u8);
pub struct SetRangeOp(u8);

// stack flavors as zero-sized marker types used by stack effect description.
// The effect is expressed as a zero sized array holding a plain function type.
// e.g.:
//   [f(CV,CV) -> CV; 0]
//
// We only need these if we plan to generate the OPCODES macro,
// or if a rust use case needs (say) push/pop counts.

pub struct CV; // Cell Value (TypedValue)
pub struct UV; // Uninit
pub struct CUV; // TypedValue, or Uninit argument
pub struct SMANY; // a variadic number of CVs
pub struct CMANY; // a variadic number of CVs
pub struct CUMANY; // a variadic number of CVs
pub struct MFINAL; // a variadic number of CVs
pub struct C_MFINAL<const N: u8>;
pub struct FCIN<const NIN: u8, const NOBJ: u8>; // input FCALL(nin, nobj)
pub struct FCOUT; // output FCALL

// InstrFlags as zero-sized marker types

pub struct TF; // Terminal
pub struct CF; // Control Flow
pub struct CF_TF; // Terminal and Control Flow
