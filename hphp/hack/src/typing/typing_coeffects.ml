(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Common
open Typing_defs
open Typing_env_types
module Env = Typing_env
module Phase = Typing_phase
module MakeType = Typing_make_type
module SN = Naming_special_names

let capability_id = Local_id.make_unscoped SN.Coeffects.capability

let local_capability_id = Local_id.make_unscoped SN.Coeffects.local_capability

let register_capabilities env (cap_ty : locl_ty) (unsafe_cap_ty : locl_ty) =
  let cap_pos = Typing_defs.get_pos cap_ty in
  (* Represents the capability for local operations inside a function body, excluding calls. *)
  let env =
    Env.set_local
      ~is_defined:true
      ~bound_ty:None
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
  ( Env.set_local
      ~is_defined:true
      ~bound_ty:None
      env
      capability_id
      ty
      (Pos_or_decl.unsafe_to_raw_pos cap_pos),
    ty )

let get_type capability =
  match capability with
  | CapTy cap -> cap
  | CapDefaults p -> MakeType.default_capability p

let rec validate_capability env pos ty =
  match get_node ty with
  | Tintersection tys -> List.iter ~f:(validate_capability env pos) tys
  | Tapply ((_, n), _) when String.is_prefix ~prefix:SN.Coeffects.contexts n ->
    ()
  | Tapply ((_, n), _) ->
    (match Env.get_class_or_typedef env n with
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      () (* unbound name error *)
    | Decl_entry.Found (Env.TypedefResult { Typing_defs.td_is_ctx = true; _ })
      ->
      ()
    | _ ->
      Errors.add_error
        Nast_check_error.(
          to_user_error
          @@ Illegal_context
               { pos; name = Typing_print.full_decl (Env.get_tcopt env) ty }))
  | Tgeneric (name, []) when SN.Coeffects.is_generated_generic name -> ()
  | Taccess (root, (_p, c)) ->
    let ((env, ty_err_opt), root) =
      Phase.localize_no_subst env ~ignore_errors:false root
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    let (env, candidates) =
      Typing_utils.get_concrete_supertypes ~abstract_enum:false env root
    in
    let check_ctx_const t =
      match get_node t with
      | Tclass ((_, name), _, _) ->
        (match Env.get_class env name with
        | Decl_entry.Found cls ->
          (match Env.get_typeconst env cls c with
          | Some tc ->
            if not tc.Typing_defs.ttc_is_ctx then
              Errors.add_error
                Nast_check_error.(
                  to_user_error
                  @@ Illegal_context
                       {
                         pos;
                         name = Typing_print.full_decl (Env.get_tcopt env) ty;
                       })
          | None -> () (* typeconst not found *))
        | Decl_entry.DoesNotExist
        | Decl_entry.NotYetAvailable ->
          () (* unbound name error *))
      | _ -> ()
    in
    List.iter ~f:check_ctx_const candidates
  | _ ->
    Errors.add_error
      Nast_check_error.(
        to_user_error
        @@ Illegal_context
             { pos; name = Typing_print.full_decl (Env.get_tcopt env) ty })

let pretty env ty =
  lazy
    (let (env, ty) = Typing_intersection.simplify_intersections env ty in
     Typing_print.coeffects env ty)

let is_generated_generic =
  String.is_prefix ~prefix:SN.Coeffects.generated_generic_prefix

let type_capability env ctxs unsafe_ctxs default_pos =
  (* No need to repeat the following check (saves time) for unsafe_ctx
     because it's synthetic and well-kinded by construction *)
  Option.iter ctxs ~f:(fun (_pos, hl) ->
      List.iter
        hl
        ~f:
          (Typing_kinding.Simple.check_well_kinded_context_hint
             ~in_signature:false
             env));

  let cc = Decl_hint.aast_contexts_to_decl_capability in
  let (decl_pos, cap) = cc env.decl_env ctxs default_pos in
  let ((env, ty_err_opt1), cap_ty) =
    match cap with
    | CapTy ty ->
      if TypecheckerOptions.strict_contexts (Env.get_tcopt env) then
        validate_capability env decl_pos ty;
      Phase.localize_no_subst env ~ignore_errors:false ty
    | CapDefaults p -> ((env, None), MakeType.default_capability p)
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt1;
  let ((env, ty_err_opt2), unsafe_cap_ty) =
    match snd @@ cc env.decl_env unsafe_ctxs default_pos with
    | CapTy ty -> Phase.localize_no_subst env ~ignore_errors:false ty
    | CapDefaults p -> ((env, None), MakeType.default_capability_unsafe p)
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt2;
  (env, cap_ty, unsafe_cap_ty)

(* Checking this with List.exists will be a single op in the vast majority of cases (empty) *)
let get_ctx_vars ctxs =
  Option.value_map
    ~f:(fun (_, cs) ->
      List.filter_map cs ~f:(function
          | (_, Haccess ((_, Hvar n), _)) -> Some n
          | _ -> None))
    ~default:[]
    ctxs
