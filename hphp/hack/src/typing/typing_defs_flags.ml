(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type flags = int

type bit_mask = int

(* Is this bit set in the flags? *)
let is_set (bit : bit_mask) (flags : flags) : bool =
  not (Int.equal 0 (Int.bit_and bit flags))

type fun_type_flags = int

type fun_param_flags = int

module ClassElt = struct
  type t = flags

  module Field = struct
    type t =
      | Abstract
      | Final
      | Override
      | Lsb
      | Synthesized
      | Const
      | Lateinit
      | Dynamicallycallable
      | SupportDynamicType
      | XaHasDefault
      | XaTagRequired
      | XaTagLateinit
      | ReadonlyProp
      | NeedsInit
    [@@deriving enum, show]

    let all : t list =
      let rec f i acc =
        if i < 0 then
          acc
        else
          f (i - 1) (Option.value_exn (of_enum i) :: acc)
      in
      f max []

    let to_bit_mask (field : t) : bit_mask = 1 lsl to_enum field

    let abstract = to_bit_mask Abstract

    let final = to_bit_mask Final

    let override = to_bit_mask Override

    let lsb = to_bit_mask Lsb

    let synthesized = to_bit_mask Synthesized

    let const = to_bit_mask Const

    let lateinit = to_bit_mask Lateinit

    let dynamicallycallable = to_bit_mask Dynamicallycallable

    let support_dynamic_type = to_bit_mask SupportDynamicType

    (* Three bits used to encode optional XHP attr.
     * Set 1<<10 (0x400) if xa_has_default=true
     * Then encode xa_tag as follows:
     *   Some Required: 1<<11 (0x0800)
     *   Some lateinit: 1<<12 (0x1000)
     *   None:          1<<11 | 1<<12 (0x1800)
     * If attr is not present at all, then masking with 0x1800 will produce zero.
     *)
    let xa_has_default = to_bit_mask XaHasDefault

    let xa_tag_required = to_bit_mask XaTagRequired

    let xa_tag_lateinit = to_bit_mask XaTagLateinit

    let xa_tag_none = Int.bit_or xa_tag_required xa_tag_lateinit

    let xa_tag_mask = Int.bit_or xa_tag_required xa_tag_lateinit

    let readonly_prop = to_bit_mask ReadonlyProp

    let needs_init = to_bit_mask NeedsInit
  end

  let to_string_map (flags : t) : bool SMap.t =
    Field.all
    |> List.map ~f:(fun field ->
           (Field.show field, is_set (Field.to_bit_mask field) flags))
    |> SMap.of_list

  let show (flags : t) : string = flags |> to_string_map |> SMap.show Bool.pp

  let pp (fmt : Format.formatter) (flags : t) : unit =
    flags |> to_string_map |> SMap.pp Bool.pp fmt
end

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

let ft_flags_support_dynamic_type = 1 lsl 12

let ft_flags_is_memoized = 1 lsl 13

(* fun_param flags *)
let fp_flags_accept_disposable = 1 lsl 0

let fp_flags_inout             = 1 lsl 1

let fp_flags_has_default       = 1 lsl 2

let fp_flags_ifc_external      = 1 lsl 3

let fp_flags_ifc_can_call      = 1 lsl 4

(* let fp_flags_via_label_DEPRECATED         = 1 lsl 5 *)

(* 6 and 7 are taken by mutability parameters above *)
let fp_flags_readonly          = 1 lsl 8
