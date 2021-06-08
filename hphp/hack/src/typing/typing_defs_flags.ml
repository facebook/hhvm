(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type fun_type_flags = int

type fun_param_flags = int

(* Is this bit set in the flags? *)
let is_set bit flags = not (Int.equal 0 (Int.bit_and bit flags))

(* Set a single bit to a boolean value *)
let set_bit bit value flags =
  if value then
    Int.bit_or bit flags
  else
    Int.bit_and (Int.bit_not bit) flags

[@@@ocamlformat "disable"]

(* NB: Keep the values of these flags in sync with typing_defs_flags.rs. *)

(* Function type flags *)
let ft_flags_return_disposable  = 1 lsl 0

let ft_flags_returns_mutable    = 1 lsl 1

let ft_flags_returns_void_to_rx = 1 lsl 2

let ft_flags_async              = 1 lsl 4

let ft_flags_generator          = 1 lsl 5

(* These flags are used for the self type on FunType and the parameter type on FunParam *)
let mutable_flags_owned         = 1 lsl 6

let mutable_flags_borrowed      = 1 lsl 7

let mutable_flags_maybe         = Int.bit_or mutable_flags_owned mutable_flags_borrowed

let mutable_flags_mask          = Int.bit_or mutable_flags_owned mutable_flags_borrowed

let ft_flags_instantiated_targs = 1 lsl 8

let ft_flags_is_function_pointer = 1 lsl 9

let ft_flags_returns_readonly = 1 lsl 10

let ft_flags_readonly_this = 1 lsl 11

(* Class element flags *)
let ce_flags_abstract            = 1 lsl 0

let ce_flags_final               = 1 lsl 1

let ce_flags_override            = 1 lsl 2

let ce_flags_lsb                 = 1 lsl 3

let ce_flags_synthesized         = 1 lsl 5

let ce_flags_const               = 1 lsl 6

let ce_flags_lateinit            = 1 lsl 7

let ce_flags_dynamicallycallable = 1 lsl 8

let ce_flags_support_dynamic_type = 1 lsl 9

(* Three bits used to encode optional XHP attr.
 * Set 1<<10 (0x400) if xa_has_default=true
 * Then encode xa_tag as follows:
 *   Some Required: 1<<11 (0x0800)
 *   Some lateinit: 1<<12 (0x1000)
 *   None:          1<<11 | 1<<12 (0x1800)
 * If attr is not present at all, then masking with 0x1800 will produce zero.
 *)
let ce_flags_xa_has_default      = 1 lsl 10

let ce_flags_xa_tag_required     = 1 lsl 11

let ce_flags_xa_tag_lateinit     = 1 lsl 12

let ce_flags_xa_tag_none         = Int.bit_or ce_flags_xa_tag_required ce_flags_xa_tag_lateinit

let ce_flags_xa_tag_mask         = Int.bit_or ce_flags_xa_tag_required ce_flags_xa_tag_lateinit

let ce_flags_readonly_prop            = 1 lsl 13

(* fun_param flags *)
let fp_flags_accept_disposable = 1 lsl 0

let fp_flags_inout             = 1 lsl 1

let fp_flags_has_default       = 1 lsl 2

let fp_flags_ifc_external      = 1 lsl 3

let fp_flags_ifc_can_call      = 1 lsl 4

let fp_flags_via_label         = 1 lsl 5

(* 6 and 7 are taken by mutability parameters above *)
let fp_flags_readonly          = 1 lsl 8
