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
module Env = Typing_env
module SN = Naming_special_names
module Reason = Typing_reason

(* Add `dynamic` lower and upper bound to any type parameters that are not marked <<__NoRequireDynamic>> *)
let add_require_dynamic_bounds env cls =
  List.fold_left (Decl_provider.Class.tparams cls) ~init:env ~f:(fun env tp ->
      let require_dynamic =
        not
          (Attributes.mem
             SN.UserAttributes.uaNoRequireDynamic
             tp.tp_user_attributes)
      in
      if require_dynamic then
        let dtype =
          Typing_make_type.dynamic (Reason.Rwitness_from_decl (fst tp.tp_name))
        in
        Env.add_upper_bound
          (Env.add_lower_bound env (snd tp.tp_name) dtype)
          (snd tp.tp_name)
          dtype
      else
        env)

let is_dynamic_decl env ty =
  match get_node ty with
  | Tdynamic -> true
  | Tgeneric (name, _) when Typing_env.get_require_dynamic env name -> true
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
  if not (te_check || is_dynamic_decl env ty) then
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
          Typing_enforceability.is_enforceable env dty
          || is_dynamic_decl env dty
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
  let ety_env = empty_expand_env in
  let ret_locl_ty =
    snd
      (Typing_return.make_return_type
         (Typing_phase.localize ~ety_env)
         env
         fun_ty.ft_ret.et_type)
  in
  sound_dynamic_interface_check env params_decl_ty ret_locl_ty

let build_dyn_fun_ty ft_ty =
  let make_dynamic pos =
    Typing_make_type.dynamic (Reason.Rsupport_dynamic_type pos)
  in
  let make_dyn_fun_param fp =
    {
      fp_pos = fp.fp_pos;
      fp_name = fp.fp_name;
      fp_type = Typing_make_type.unenforced (make_dynamic fp.fp_pos);
      fp_flags = fp.fp_flags;
    }
  in

  {
    ft_arity = ft_ty.ft_arity;
    ft_tparams = [];
    ft_where_constraints = [];
    ft_params = List.map ft_ty.ft_params ~f:make_dyn_fun_param;
    ft_implicit_params = ft_ty.ft_implicit_params;
    ft_ret =
      Typing_make_type.unenforced (make_dynamic (get_pos ft_ty.ft_ret.et_type));
    (* Carries through the sync/async information from the aast *)
    ft_flags = ft_ty.ft_flags;
    ft_ifc_decl = ft_ty.ft_ifc_decl;
  }

let relax_method_type env relax r ft =
  let lr = Typing_reason.localize r in
  if Typing_env_types.(env.in_support_dynamic_type_method_check) && relax then
    mk
      ( Typing_reason.Rsupport_dynamic_type (Typing_reason.to_pos r),
        Tintersection [mk (lr, Tfun ft); mk (lr, Tfun (build_dyn_fun_ty ft))] )
  else
    mk (lr, Tfun ft)
