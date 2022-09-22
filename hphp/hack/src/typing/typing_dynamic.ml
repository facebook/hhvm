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

(* Add `supportdyn<mixed>` lower and upper bound to any type parameters that are marked <<__RequireDynamic>>
 * Just add the upper bound to others. *)
let add_require_dynamic_bounds env cls =
  List.fold_left (Decl_provider.Class.tparams cls) ~init:env ~f:(fun env tp ->
      let require_dynamic =
        Attributes.mem SN.UserAttributes.uaRequireDynamic tp.tp_user_attributes
      in
      let dtype =
        Typing_make_type.supportdyn
          (Reason.Rwitness_from_decl (fst tp.tp_name))
          (Typing_make_type.mixed Reason.Rnone)
      in
      let env = Env.add_upper_bound env (snd tp.tp_name) dtype in
      if
        require_dynamic
        (* Implicit pessimisation should ignore the RequireDynamic attribute
           because everything should be pessimised enough that it isn't necessary. *)
        && not
             (TypecheckerOptions.everything_sdt
                Typing_env_types.(env.genv.tcopt))
      then
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
  if not (Typing_utils.is_supportdyn env ty) then (
    let pos = get_pos ty in
    Typing_log.log_pessimise_prop
      env
      (Pos_or_decl.unsafe_to_raw_pos pos)
      (snd id);
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
  if not te_check then (
    let ((env, ty_err_opt), ty) =
      match ty with
      | Some ty -> ((env, None), ty)
      | None -> Typing_phase.localize_no_subst ~ignore_errors:true env decl_ty
    in
    Option.iter ~f:Errors.add_typing_error ty_err_opt;
    if
      not
        (Typing_utils.is_sub_type_for_union
           env
           (Typing_make_type.dynamic Reason.Rnone)
           ty)
    then (
      let pos = get_pos decl_ty in
      Typing_log.log_pessimise_prop
        env
        (Pos_or_decl.unsafe_to_raw_pos pos)
        (snd id);
      Some
        (on_error
           (fst id)
           (snd id)
           classname
           (pos, Typing_print.full_strip_ns_decl env decl_ty))
    ) else
      None
  ) else
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
    Typing_utils.is_supportdyn env ret_locl_ty
  in
  enforceable_params && coercible_return_type

let sound_dynamic_interface_check_from_fun_ty env fun_ty =
  let params_decl_ty =
    List.map fun_ty.ft_params ~f:(fun fun_param ->
        Some fun_param.fp_type.et_type)
  in
  let ret_locl_ty =
    snd
      (Typing_return.make_return_type
         ~ety_env:empty_expand_env
         env
         ~hint_pos:Pos.none
         ~explicit:(Some fun_ty.ft_ret.et_type)
         ~default:None)
  in
  sound_dynamic_interface_check env params_decl_ty ret_locl_ty.et_type

(* Given t, construct ~t.
 * acc is a boolean that remains false if no change was made (e.g. t = dynamic)
 *)
let make_like env changed ty =
  if
    Typing_defs.is_dynamic ty
    || Option.is_some (Typing_utils.try_strip_dynamic env ty)
  then
    (changed, ty)
  else
    let r = get_reason ty in
    (true, Typing_make_type.locl_like r ty)

let maybe_wrap_with_supportdyn ~should_wrap locl_r ft =
  if should_wrap then
    let r = Typing_reason.Rsupport_dynamic_type (Typing_reason.to_pos locl_r) in
    let ft =
      {
        ft with
        ft_flags =
          Typing_defs_flags.(
            set_bit ft_flags_support_dynamic_type false ft.ft_flags);
      }
    in
    Typing_make_type.supportdyn r (mk (locl_r, Tfun ft))
  else
    mk (locl_r, Tfun ft)

let push_like_tyargs env tyl tparams =
  if List.length tyl <> List.length tparams then
    (false, tyl)
  else
    let make_like changed ty tp =
      (* Don't both pushing through a contravariant parameter, because
       * Contra<~t> <: Contra<t> <: ~Contra<t> by variance and union
       * subtyping rules already *)
      match tp.tp_variance with
      | Ast_defs.Contravariant -> (changed, ty)
      | _ ->
        let (changed', ty') = make_like env changed ty in
        (* Push like onto type argument; if the resulting type is not a subtype of
         * the the upper bound on the type parameter, then intersect it with the
         * upper bonud so that the resulting type is well-formed.
         * For example, dict<~string & arraykey,bool> <: ~dict<string,bool>
         * because the first type parameter has an upper bound of arraykey.
         *)
        let upper_bounds =
          List.filter_map tp.tp_constraints ~f:(fun (c, cty) ->
              match c with
              | Ast_defs.Constraint_as ->
                let (_env, cty) =
                  Typing_phase.localize_no_subst env ~ignore_errors:true cty
                in
                Some cty
              | _ -> None)
        in
        (* Type meets all bounds, so leave alone *)
        if
          List.for_all upper_bounds ~f:(fun bound ->
              Typing_utils.is_sub_type_for_union env ty' bound)
        then
          (changed', ty')
        else
          (changed', mk (get_reason ty, Tintersection (ty' :: upper_bounds)))
    in

    List.map2_env false tyl tparams ~f:make_like

let rec try_push_like env ty =
  match deref ty with
  | (r, Ttuple tyl) ->
    let (changed, tyl) = List.map_env false tyl ~f:(make_like env) in
    ( env,
      if changed then
        Some (mk (r, Ttuple tyl))
      else
        None )
  | (r, Tfun ft) ->
    let (changed, ret_ty) = make_like env false ft.ft_ret.et_type in
    ( env,
      if changed then
        Some
          (mk
             (r, Tfun { ft with ft_ret = { ft.ft_ret with et_type = ret_ty } }))
      else
        None )
  | (r, Tshape (kind, fields)) ->
    let add_like_to_shape_field changed _name { sft_optional; sft_ty } =
      let (changed, sft_ty) = make_like env changed sft_ty in
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
  | (r, Toption ty) ->
    begin
      match try_push_like env ty with
      | (env, Some ty) -> (env, Some (mk (r, Toption ty)))
      | (env, None) -> (env, None)
    end
  | (r, Tvec_or_dict (tk, tv)) ->
    let (changed, tyl) = List.map_env false [tk; tv] ~f:(make_like env) in
    if changed then
      match tyl with
      | [tk; tv] -> (env, Some (mk (r, Tvec_or_dict (tk, tv))))
      | _ -> assert false
    else
      (env, None)
  | _ -> (env, None)
