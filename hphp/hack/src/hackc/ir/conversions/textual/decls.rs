// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// This is emitted with every SIL file to declare the "standard" definitions
/// that we use.
pub const BUILTIN_DECLS: &str = r#"
// ----- BUILTIN DELS STARTS HERE -----

declare copy(*string) : *HackMixed
declare hack_add(*HackMixed, *HackMixed) : *HackMixed
declare hack_modulo(*HackMixed, *HackMixed) : *HackMixed
declare hack_bool(bool) : *HackMixed
declare hack_cmp_eq(*HackMixed, *HackMixed) : *HackMixed
declare hack_cmp_gt(*HackMixed, *HackMixed) : *HackMixed
declare hack_cmp_gte(*HackMixed, *HackMixed) : *HackMixed
declare hack_cmp_lt(*HackMixed, *HackMixed) : *HackMixed
declare hack_cmp_lte(*HackMixed, *HackMixed) : *HackMixed
declare hack_cmp_nsame(*HackMixed, *HackMixed) : *HackMixed
declare hack_cmp_neq(*HackMixed, *HackMixed) : *HackMixed
declare hack_cmp_same(*HackMixed, *HackMixed) : *HackMixed
declare hack_int(int) : *HackMixed
declare hack_is_true(*HackMixed): bool
declare hack_null() : *HackMixed
declare hack_print(*HackMixed) : void
declare hack_string(*string) : *HackMixed

declare arg_pack_0() : *HackParams
declare arg_pack_1() : *HackParams
declare arg_pack_2() : *HackParams
declare arg_pack_3() : *HackParams
declare arg_pack_4() : *HackParams
declare arg_pack_5() : *HackParams

"#;
