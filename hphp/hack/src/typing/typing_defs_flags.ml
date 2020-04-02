(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* Is this bit set in the flags? *)
let is_set bit flags = not (Int.equal 0 (Int.bit_and bit flags))

(* Set a single bit to a boolean value *)
let set_bit bit value flags =
  if value then
    Int.bit_or bit flags
  else
    Int.bit_and (Int.bit_not bit) flags

(* Function type flags *)
let ft_flags_return_disposable = 0x1

let ft_flags_returns_mutable = 0x2

let ft_flags_returns_void_to_rx = 0x4

let ft_flags_is_coroutine = 0x8

let ft_flags_async = 0x10

let ft_flags_generator = 0x20

let ft_flags_param_owned_mutable = 0x40

let ft_flags_param_borrowed_mutable = 0x80

let ft_flags_param_maybe_mutable = 0xC0

let ft_flags_instantiated_targs = 0x100
