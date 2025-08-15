(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type flags = int [@@deriving eq, hash, ord]

type bit_mask = int

(* Is this bit set in the flags? *)
let is_set (bit : bit_mask) (flags : flags) : bool =
  not (Int.equal 0 (Int.bit_and bit flags))

(* Set a single bit to a boolean value *)
let set_bit bit value flags =
  if value then
    Int.bit_or bit flags
  else
    Int.bit_and (Int.bit_not bit) flags

(* NB: Keep the values of these flags in sync with typing_defs_flags.rs. *)

module Fun = struct
  type t = flags [@@deriving eq, hash, ord]

  type record = {
    return_disposable: bool;
    async: bool;
    generator: bool;
    fun_kind: Ast_defs.fun_kind;
    is_function_pointer: bool;
    returns_readonly: bool;
    readonly_this: bool;
    support_dynamic_type: bool;
    is_memoized: bool;
    variadic: bool;
  }
  [@@deriving show]

  let return_disposable_mask = 1 lsl 0

  let return_disposable = is_set return_disposable_mask

  let set_return_disposable = set_bit return_disposable_mask

  let async_mask = 1 lsl 4

  let async = is_set async_mask

  let set_async = set_bit async_mask

  let generator_mask = 1 lsl 5

  let generator = is_set generator_mask

  let set_generator = set_bit generator_mask

  let fun_kind_to_flags kind =
    match kind with
    | Ast_defs.FSync -> 0
    | Ast_defs.FAsync -> async_mask
    | Ast_defs.FGenerator -> generator_mask
    | Ast_defs.FAsyncGenerator -> Int.bit_or async_mask generator_mask

  let fun_kind t =
    match (async t, generator t) with
    | (false, false) -> Ast_defs.FSync
    | (true, false) -> Ast_defs.FAsync
    | (false, true) -> Ast_defs.FGenerator
    | (true, true) -> Ast_defs.FAsyncGenerator

  let is_function_pointer_mask = 1 lsl 9

  let is_function_pointer = is_set is_function_pointer_mask

  let set_is_function_pointer = set_bit is_function_pointer_mask

  let returns_readonly_mask = 1 lsl 10

  let returns_readonly = is_set returns_readonly_mask

  let set_returns_readonly = set_bit returns_readonly_mask

  let readonly_this_mask = 1 lsl 11

  let readonly_this = is_set readonly_this_mask

  let set_readonly_this = set_bit readonly_this_mask

  let support_dynamic_type_mask = 1 lsl 12

  let support_dynamic_type = is_set support_dynamic_type_mask

  let set_support_dynamic_type = set_bit support_dynamic_type_mask

  let is_memoized_mask = 1 lsl 13

  let is_memoized = is_set is_memoized_mask

  let set_is_memoized = set_bit is_memoized_mask

  let variadic_mask = 1 lsl 14

  let variadic = is_set variadic_mask

  let set_variadic = set_bit variadic_mask

  let make
      kind
      ~return_disposable
      ~returns_readonly
      ~readonly_this
      ~support_dynamic_type
      ~is_memoized
      ~variadic =
    fun_kind_to_flags kind
    |> set_return_disposable return_disposable
    |> set_returns_readonly returns_readonly
    |> set_readonly_this readonly_this
    |> set_support_dynamic_type support_dynamic_type
    |> set_is_memoized is_memoized
    |> set_variadic variadic

  let default = 0

  let as_record t =
    {
      return_disposable = return_disposable t;
      async = async t;
      generator = generator t;
      fun_kind = fun_kind t;
      is_function_pointer = is_function_pointer t;
      returns_readonly = returns_readonly t;
      readonly_this = readonly_this t;
      support_dynamic_type = support_dynamic_type t;
      is_memoized = is_memoized t;
      variadic = variadic t;
    }

  let pp fmt t = pp_record fmt (as_record t)

  let show t = Format.asprintf "%a" pp t
end

module FunParam = struct
  type t = flags [@@deriving eq, hash, ord]

  type record = {
    accept_disposable: bool;
    inout: bool;
    is_optional: bool;
    readonly: bool;
    ignore_readonly_error: bool;
    splat: bool;
    named: bool;
  }
  [@@deriving show]

  let accept_disposable_mask = 1 lsl 0

  let inout_mask = 1 lsl 1

  let is_optional_mask = 1 lsl 2

  let readonly_mask = 1 lsl 8

  let ignore_readonly_error_mask = 1 lsl 3

  let splat_mask = 1 lsl 9

  let named_mask = 1 lsl 4

  let accept_disposable = is_set accept_disposable_mask

  let inout = is_set inout_mask

  let is_optional = is_set is_optional_mask

  let readonly = is_set readonly_mask

  let ignore_readonly_error = is_set ignore_readonly_error_mask

  let splat = is_set splat_mask

  let named = is_set named_mask

  let set_accept_disposable = set_bit accept_disposable_mask

  let set_inout = set_bit inout_mask

  let set_is_optional = set_bit is_optional_mask

  let set_readonly = set_bit readonly_mask

  let set_ignore_readonly_error = set_bit ignore_readonly_error_mask

  let set_splat = set_bit splat_mask

  let set_named = set_bit named_mask

  let make
      ~inout
      ~accept_disposable
      ~is_optional
      ~readonly
      ~ignore_readonly_error
      ~splat
      ~named =
    0x0
    |> set_inout inout
    |> set_accept_disposable accept_disposable
    |> set_is_optional is_optional
    |> set_readonly readonly
    |> set_ignore_readonly_error ignore_readonly_error
    |> set_splat splat
    |> set_named named

  let as_record t =
    {
      accept_disposable = accept_disposable t;
      inout = inout t;
      is_optional = is_optional t;
      readonly = readonly t;
      ignore_readonly_error = ignore_readonly_error t;
      splat = splat t;
      named = named t;
    }

  let default = 0

  let pp fmt t = pp_record fmt (as_record t)

  let show t = Format.asprintf "%a" pp t
end

module ClassElt = struct
  type t = flags

  module Field = struct
    type t =
      | Abstract
      | Final
      | SuperfluousOverride
          (** Whether the __Override attribute is erroneous, i.e. there is nothing in parents to override.
              This is set during decling (because that's the easiest place to spot this error)
              so that an error can be emitted later during typing. *)
      | Lsb
      | Synthesized
      | Const
      | Lateinit
      | Dynamicallycallable
      | SupportDynamicType
      | XaHasDefault
      | XaTagRequired
      | XaTagLateinit
      | ReadonlyPropOrNeedsConcrete
          (**
       * for properties: indicates readonly-ness
       * for methods: indicates presence of <<__NeedsConcrete>> attribute
      *)
      | NeedsInit
      | SafeGlobalVariable
      | NoAutoLikes
    (* NB: Keep these flags in sync with typing_defs_flags.rs. *)
    [@@deriving enum, show { with_path = false }]

    (* [min] is generated by the enum ppx and is unused. This suppresses the warning. *)
    let _ = min

    let all : t list =
      let rec f i acc =
        if i < 0 then
          acc
        else
          f (i - 1) (Option.value_exn (of_enum i) :: acc)
      in
      f max []

    let to_bit_mask (field : t) : bit_mask = 1 lsl to_enum field

    let list_to_bit_mask (fields : t list) : bit_mask =
      List.map fields ~f:to_bit_mask |> List.fold ~init:0 ~f:Int.bit_or

    (* The three XA bits are used to encode optional XHP attr.
     * Set 1<<10 (0x400) if xa_has_default=true
     * Then encode xa_tag as follows:
     *   Some Required: 1<<11 (0x0800)
     *   Some lateinit: 1<<12 (0x1000)
     *   None:          1<<11 | 1<<12 (0x1800)
     * If attr is not present at all, then masking with 0x1800 will produce zero.
     *)

    let xhp_attr_fields : t list = [XaHasDefault; XaTagLateinit; XaTagRequired]

    let of_xhp_tag : Xhp_attribute.tag -> t = function
      | Xhp_attribute.Required -> XaTagRequired
      | Xhp_attribute.LateInit -> XaTagLateinit

    let to_xhp_tag : t -> Xhp_attribute.tag = function
      | XaTagRequired -> Xhp_attribute.Required
      | XaTagLateinit -> Xhp_attribute.LateInit
      | _ -> failwith "cannot convert field to xhp attribute tag"

    let equal_xhp_tag (field : t) (tag : Xhp_attribute.tag) =
      match (field, tag) with
      | (XaTagRequired, Xhp_attribute.Required)
      | (XaTagLateinit, Xhp_attribute.LateInit) ->
        true
      | _ -> false

    let of_xhp_attr (xa : Xhp_attribute.t option) : t list =
      match xa with
      | None -> []
      | Some { Xhp_attribute.xa_has_default; xa_tag } ->
        let tag_fields =
          match xa_tag with
          | None -> [XaTagRequired; XaTagLateinit]
          | Some tag -> [of_xhp_tag tag]
        in
        if xa_has_default then
          XaHasDefault :: tag_fields
        else
          tag_fields

    let to_xhp_attr (fields : t list) : Xhp_attribute.t option =
      match fields with
      | [] -> None
      | _ :: _ ->
        let rec to_xhp_attr fields xa =
          match fields with
          | [] -> xa
          | field :: fields ->
            let xa =
              let update_tag field tag =
                match tag with
                | None -> Some (to_xhp_tag field)
                | Some tag ->
                  if equal_xhp_tag field tag then
                    Some tag
                  else
                    (* If both XaTagRequired and XaTagLateinit bits are set, then that's
                       code for xa_tag = None. *)
                    None
              in
              match field with
              | XaHasDefault -> { xa with Xhp_attribute.xa_has_default = true }
              | XaTagRequired
              | XaTagLateinit ->
                Xhp_attribute.map_tag xa ~f:(update_tag field)
              | _ -> xa
            in
            to_xhp_attr fields xa
        in
        Some (to_xhp_attr fields Xhp_attribute.init)
  end

  let is_set (field : Field.t) (flags : t) : bool =
    is_set (Field.to_bit_mask field) flags

  let set (field : Field.t) (value : bool) (flags : t) : t =
    set_bit (Field.to_bit_mask field) value flags

  let is_abstract = is_set Field.Abstract

  let is_final = is_set Field.Final

  let has_superfluous_override = is_set Field.SuperfluousOverride

  let has_lsb = is_set Field.Lsb

  let is_synthesized = is_set Field.Synthesized

  let is_const = is_set Field.Const

  let has_lateinit = is_set Field.Lateinit

  let is_dynamicallycallable = is_set Field.Dynamicallycallable

  let supports_dynamic_type = is_set Field.SupportDynamicType

  let is_readonly_prop_or_needs_concrete =
    is_set Field.ReadonlyPropOrNeedsConcrete

  let needs_init = is_set Field.NeedsInit

  let is_safe_global_variable = is_set Field.SafeGlobalVariable

  let is_no_auto_likes = is_set Field.NoAutoLikes

  let get_xhp_attr (flags : t) : Xhp_attribute.t option =
    Field.xhp_attr_fields
    |> List.filter ~f:(fun field -> is_set field flags)
    |> Field.to_xhp_attr

  let set_xhp_attr (xa : Xhp_attribute.t option) (flags : t) : t =
    let xhp_attr_as_flags = xa |> Field.of_xhp_attr |> Field.list_to_bit_mask in
    let reset_xhp_attr =
      let xhp_attr_mask = Field.xhp_attr_fields |> Field.list_to_bit_mask in
      Int.bit_and (Int.bit_not xhp_attr_mask)
    in
    flags |> reset_xhp_attr |> Int.bit_or xhp_attr_as_flags

  let to_string_map (flags : t) : bool SMap.t =
    Field.all
    |> List.map ~f:(fun field -> (Field.show field, is_set field flags))
    |> SMap.of_list

  let show (flags : t) : string = flags |> to_string_map |> SMap.show Bool.pp

  let pp (fmt : Format.formatter) (flags : t) : unit =
    flags |> to_string_map |> SMap.pp Bool.pp fmt

  let make
      ~xhp_attr
      ~abstract
      ~final
      ~superfluous_override
      ~lsb
      ~synthesized
      ~const
      ~lateinit
      ~dynamicallycallable
      ~readonly_prop_or_needs_concrete
      ~support_dynamic_type
      ~needs_init
      ~safe_global_variable
      ~no_auto_likes =
    let flags = 0 in
    let flags = set Field.Abstract abstract flags in
    let flags = set Field.Final final flags in
    let flags = set Field.SuperfluousOverride superfluous_override flags in
    let flags = set Field.Lsb lsb flags in
    let flags = set Field.Synthesized synthesized flags in
    let flags = set Field.Const const flags in
    let flags = set Field.Lateinit lateinit flags in
    let flags = set Field.Dynamicallycallable dynamicallycallable flags in
    let flags = set_xhp_attr xhp_attr flags in
    let flags =
      set
        Field.ReadonlyPropOrNeedsConcrete
        readonly_prop_or_needs_concrete
        flags
    in
    let flags = set Field.SupportDynamicType support_dynamic_type flags in
    let flags = set Field.NeedsInit needs_init flags in
    let flags = set Field.SafeGlobalVariable safe_global_variable flags in
    let flags = set Field.NoAutoLikes no_auto_likes flags in
    flags

  let set_synthesized = set Field.Synthesized true

  let reset_superfluous_override = set Field.SuperfluousOverride false
end
