(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs_core
open Hh_prelude
module Env = Typing_env
module SN = Naming_special_names

let capability_id = Local_id.make_unscoped SN.Coeffects.capability

let local_capability_id = Local_id.make_unscoped SN.Coeffects.local_capability

let register_capabilities env (cap_ty : locl_ty) (unsafe_cap_ty : locl_ty) =
  let cap_pos = Typing_defs.get_pos cap_ty in
  (* Represents the capability for local operations inside a function body, excluding calls. *)
  let env =
    Env.set_local
      env
      local_capability_id
      cap_ty
      (Pos_or_decl.unsafe_to_raw_pos cap_pos)
  in
  let (env, ty) =
    Typing_intersection.intersect
      env
      ~r:(Reason.Rhint cap_pos)
      cap_ty
      unsafe_cap_ty
  in
  (* The implicit argument for ft_implicit_params.capability *)
  ( Env.set_local env capability_id ty (Pos_or_decl.unsafe_to_raw_pos cap_pos),
    ty )

let get_type capability =
  match capability with
  | CapTy cap -> cap
  | CapDefaults p -> Typing_make_type.default_capability p

let rec validate_capability env pos ty =
  match get_node ty with
  | Tintersection tys -> List.iter ~f:(validate_capability env pos) tys
  | Tapply ((_, n), _) when String.is_prefix ~prefix:SN.Coeffects.contexts n ->
    ()
  | Tapply ((_, n), _) ->
    (match Env.get_class_or_typedef env n with
    | None -> () (* unbound name error *)
    | Some (Typing_env.TypedefResult { Typing_defs.td_is_ctx = true; _ }) -> ()
    | _ -> Errors.illegal_context pos (Typing_print.full_decl ty))
  | Tgeneric (name, []) when SN.Coeffects.is_generated_generic name -> ()
  | Taccess (root, (_p, c)) ->
    let (env, root) =
      Typing_phase.localize_no_subst env ~ignore_errors:false root
    in
    let (env, candidates) =
      Typing_utils.get_concrete_supertypes ~abstract_enum:false env root
    in
    let check_ctx_const t =
      match get_node t with
      | Tclass ((_, name), _, _) ->
        (match Env.get_class env name with
        | Some cls ->
          (match Env.get_typeconst env cls c with
          | Some tc ->
            if not tc.Typing_defs.ttc_is_ctx then
              Errors.illegal_context pos (Typing_print.full_decl ty)
          | None -> () (* typeconst not found *))
        | None -> () (* unbound name error *))
      | _ -> ()
    in
    List.iter ~f:check_ctx_const candidates
  | _ -> Errors.illegal_context pos (Typing_print.full_decl ty)

let pretty env ty =
  let (env, ty) = Typing_intersection.simplify_intersections env ty in
  Typing_print.coeffects env ty

let is_generated_generic =
  String.is_prefix ~prefix:SN.Coeffects.generated_generic_prefix
