(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Reason = Typing_reason

let is_dynamic_decl ty =
  match get_node ty with
  | Tdynamic -> true
  | _ -> false

(* Check that a property type is a subtype of dynamic *)
let check_property_sound_for_dynamic_read ~on_error env classname id ty =
  if
    not
      (Typing_utils.is_sub_type_for_union
         ~coerce:(Some Typing_logic.CoerceToDynamic)
         env
         ty
         (mk (Reason.Rnone, Tdynamic)))
  then
    on_error
      (fst id)
      (snd id)
      classname
      (get_pos ty, Typing_print.full_strip_ns env ty)

let check_property_sound_for_dynamic_write ~on_error env classname id ty =
  let te_check = Typing_enforceability.is_enforceable env ty in
  (* If the property tyoe isn't enforceable, but is just dynamic,
     then the property will still be safe to write via a receiver expression of type
     dynamic. *)
  if not (te_check || is_dynamic_decl ty) then
    on_error
      (fst id)
      (snd id)
      classname
      (get_pos ty, Typing_print.full_strip_ns_decl env ty)

let sound_dynamic_interface_check env params_decl_ty ret_locl_ty =
  (* 1. check if all the parameters of the method are enforceable *)
  let enforceable_params =
    List.for_all params_decl_ty ~f:(fun dtyopt ->
        match dtyopt with
        | Some dty ->
          (* If a parameter isn't enforceable, but is just typed as dynamic,
             the method will still be safe to call via a receiver expression of type
             dynamic. *)
          Typing_enforceability.is_enforceable env dty || is_dynamic_decl dty
        | None -> true)
  in
  let coercible_return_type =
    (* 2. check if the return type is coercible *)
    Typing_utils.is_sub_type_for_union
      ~coerce:(Some Typing_logic.CoerceToDynamic)
      env
      ret_locl_ty
      (mk (Reason.Rnone, Tdynamic))
  in
  enforceable_params && coercible_return_type

let sound_dynamic_interface_check_from_fun_ty env fun_ty =
  let params_decl_ty =
    List.map fun_ty.ft_params ~f:(fun fun_param ->
        Some fun_param.fp_type.et_type)
  in
  let ety_env = Typing_phase.env_with_self env Errors.ignore_error in
  let ret_locl_ty =
    snd
      (Typing_return.make_return_type
         (Typing_phase.localize ~ety_env)
         env
         fun_ty.ft_ret.et_type)
  in
  sound_dynamic_interface_check env params_decl_ty ret_locl_ty
