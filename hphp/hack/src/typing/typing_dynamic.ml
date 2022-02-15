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
open Common
module Env = Typing_env
module SN = Naming_special_names
module Reason = Typing_reason
module Cls = Decl_provider.Class

(* Add `dynamic` lower and upper bound to any type parameters that are marked <<__RequireDynamic>>
 * Just add the upper bound to others. *)
let add_require_dynamic_bounds env cls =
  List.fold_left (Decl_provider.Class.tparams cls) ~init:env ~f:(fun env tp ->
      let require_dynamic =
        Attributes.mem SN.UserAttributes.uaRequireDynamic tp.tp_user_attributes
      in
      let dtype =
        Typing_make_type.dynamic (Reason.Rwitness_from_decl (fst tp.tp_name))
      in
      let env = Env.add_upper_bound env (snd tp.tp_name) dtype in
      if require_dynamic then
        Env.add_lower_bound env (snd tp.tp_name) dtype
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
  then (
    let pos = get_pos ty in
    Typing_log.log_pessimise_prop env pos (snd id);
    Some
      (on_error
         (fst id)
         (snd id)
         classname
         (pos, Typing_print.full_strip_ns env ty))
  ) else
    None

(* The optional ty should be (if not None) the localisation of the decl_ty.
   This lets us avoid re-localising the decl type when the caller has already
  localised it *)
let check_property_sound_for_dynamic_write ~on_error env classname id decl_ty ty
    =
  let te_check = Typing_enforceability.is_enforceable env decl_ty in
  (* If the property type isn't enforceable, but is a supertype of dynamic,
     then the property will still be safe to write via a receiver expression of type
     dynamic. *)
  if not te_check then
    let (env, ty) =
      match ty with
      | Some ty -> (env, ty)
      | None -> Typing_phase.localize_no_subst ~ignore_errors:true env decl_ty
    in
    if
      not
        (Typing_utils.is_sub_type_for_union
           ~coerce:(Some Typing_logic.CoerceToDynamic)
           env
           (Typing_make_type.dynamic Reason.Rnone)
           ty)
    then (
      let pos = get_pos decl_ty in
      Typing_log.log_pessimise_prop env pos (snd id);
      Some
        (on_error
           (fst id)
           (snd id)
           classname
           (pos, Typing_print.full_strip_ns_decl env decl_ty))
    ) else
      None
  else
    None

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

(* Given t, construct ~t.
 * acc is a boolean that remains false if no change was made (e.g. t = dynamic)
 *)
let make_like changed ty =
  if Typing_defs.is_dynamic ty then
    (changed, ty)
  else
    let r = get_reason ty in
    (true, Typing_make_type.locl_like r ty)

let push_like_tyargs env tyl tparams =
  if List.length tyl <> List.length tparams then
    (false, tyl)
  else
    (* Only push like onto type argument if it produces a well-formed type
     * i.e. satisfies any as constraints
     *)
    let make_like changed ty tp =
      let (changed', ty') = make_like changed ty in
      if
        List.for_all tp.tp_constraints ~f:(fun (c, cty) ->
            match c with
            | Ast_defs.Constraint_as ->
              let (_env, cty) =
                Typing_phase.localize_no_subst env ~ignore_errors:true cty
              in
              Typing_utils.is_sub_type_for_union
                ~coerce:(Some Typing_logic.CoerceToDynamic)
                env
                ty'
                cty
            | _ -> true)
      then
        (changed', ty')
      else
        (changed, ty)
    in

    List.map2_env false tyl tparams ~f:make_like

let try_push_like env ty =
  match deref ty with
  | (r, Ttuple tyl) ->
    let (changed, tyl) = List.map_env false tyl ~f:make_like in
    ( env,
      if changed then
        Some (mk (r, Ttuple tyl))
      else
        None )
  | (r, Tshape (kind, fields)) ->
    let add_like_to_shape_field changed _name { sft_optional; sft_ty } =
      let (changed, sft_ty) = make_like changed sft_ty in
      (changed, { sft_optional; sft_ty })
    in
    let (changed, fields) =
      TShapeMap.map_env add_like_to_shape_field false fields
    in
    ( env,
      if changed then
        Some (mk (r, Tshape (kind, fields)))
      else
        None )
  | (r, Tnewtype (n, tyl, bound)) ->
    begin
      match Env.get_typedef env n with
      | None -> (env, None)
      | Some td ->
        let (changed, tyl) = push_like_tyargs env tyl td.td_tparams in
        ( env,
          if changed then
            Some (mk (r, Tnewtype (n, tyl, bound)))
          else
            None )
    end
  | (r, Tclass ((p, n), exact, tyl)) ->
    begin
      match Env.get_class env n with
      | None -> (env, None)
      | Some cd ->
        let (changed, tyl) = push_like_tyargs env tyl (Cls.tparams cd) in
        ( env,
          if changed then
            Some (mk (r, Tclass ((p, n), exact, tyl)))
          else
            None )
    end
  | _ -> (env, None)
