(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This module implements the typing.
 *
 * Given an Nast.program, it infers the type of all the local
 * variables, and checks that all the types are correct (aka
 * consistent) *)
open Hh_prelude
open Common
open Typing_defs
open Typing_env_types
open Aast
module Env = Typing_env
module SN = Naming_special_names
module MakeType = Typing_make_type

module ExpectedTy : sig
  [@@@warning "-32"]

  type t = private {
    pos: Pos.t;
    reason: Typing_reason.ureason;
    ty: locl_possibly_enforced_ty;
  }
  [@@deriving show]

  [@@@warning "+32"]

  val make : Pos.t -> Typing_reason.ureason -> locl_ty -> t

  (* We will allow coercion to this expected type, if et_enforced=Enforced *)
  val make_and_allow_coercion :
    Pos.t -> Typing_reason.ureason -> locl_possibly_enforced_ty -> t

  (* If type is an unsolved type variable, don't create an expected type *)
  val make_and_allow_coercion_opt :
    Typing_env_types.env ->
    Pos.t ->
    Typing_reason.ureason ->
    locl_possibly_enforced_ty ->
    t option
end = struct
  (* Some mutually recursive inference functions in typing.ml pass around an ~expected argument that
   * enables bidirectional type checking. This module abstracts away that type so that it can be
   * extended and modified without having to touch every consumer. *)
  type t = {
    pos: Pos.t;
    reason: Typing_reason.ureason;
    ty: locl_possibly_enforced_ty;
  }
  [@@deriving show]

  let make_and_allow_coercion_opt env pos reason ty =
    let (_env, ety) = Env.expand_type env ty.et_type in
    if is_tyvar ety then
      None
    else
      Some { pos; reason; ty }

  let make_and_allow_coercion pos reason ty = { pos; reason; ty }

  let make pos reason locl_ty =
    make_and_allow_coercion pos reason (MakeType.unenforced locl_ty)
end

(* Return a map describing all the fields in this record, including
   inherited fields, and whether they have a default value. *)
let all_record_fields (env : env) (rd : Decl_provider.record_def_decl) :
    (Typing_defs.pos_id * Typing_defs.record_field_req) SMap.t =
  let record_fields rd =
    List.fold
      rd.rdt_fields
      ~init:SMap.empty
      ~f:(fun acc (((_, name), _) as f) -> SMap.add name f acc)
  in
  let rec loop rd fields decls_seen =
    match rd.rdt_extends with
    | Some (_, parent_name) when SSet.mem parent_name decls_seen ->
      (* Inheritance loop, so we've seen all the records. *)
      fields
    | Some (_, parent_name) ->
      (match Decl_provider.get_record_def (Env.get_ctx env) parent_name with
      | Some rd ->
        loop
          rd
          (SMap.union fields (record_fields rd))
          (SSet.add (snd rd.rdt_name) decls_seen)
      | None -> fields)
    | None -> fields
  in
  loop rd (record_fields rd) (SSet.singleton (snd rd.rdt_name))

let add_decl_errors = function
  | None -> ()
  | Some errors -> Errors.merge_into_current errors

(*****************************************************************************)
(* Handling function/method arguments *)
(*****************************************************************************)
let param_has_attribute param attr =
  List.exists param.param_user_attributes (fun { ua_name; _ } ->
      String.equal attr (snd ua_name))

let has_accept_disposable_attribute param =
  param_has_attribute param SN.UserAttributes.uaAcceptDisposable

let with_timeout env fun_name ~(do_ : env -> 'b) : 'b option =
  let timeout = (Env.get_tcopt env).GlobalOptions.tco_timeout in
  if Int.equal timeout 0 then
    Some (do_ env)
  else
    let big_envs = ref [] in
    let env = { env with big_envs } in
    Timeout.with_timeout
      ~timeout
      ~on_timeout:(fun _ ->
        (*List.iter !big_envs (fun (p, env) ->
            Typing_log.log_key "WARN: environment is too big.";
            Typing_log.hh_show_env p env); *)
        Errors.typechecker_timeout fun_name timeout;
        None)
      ~do_:(fun _ -> Some (do_ env))

(* If the localized types of the return type is a tyvar we force it to be covariant.
  The same goes for parameter types, but this time we force them to be contravariant
*)
let set_tyvars_variance_in_callable env return_ty param_tys variadic_param_ty =
  Env.log_env_change "set_tyvars_variance_in_callable" env
  @@
  let set_variance = Env.set_tyvar_variance ~for_all_vars:true in
  let env = set_variance env return_ty in
  let env = List.fold param_tys ~init:env ~f:(set_variance ~flip:true) in
  let env =
    match variadic_param_ty with
    | FVvariadicArg vparam ->
      let (_p, ty) = vparam.param_annotation in
      set_variance env ty ~flip:true
    | FVellipsis _
    | FVnonVariadic ->
      env
  in
  env

let reify_kind = function
  | Erased -> Aast.Erased
  | SoftReified -> Aast.SoftReified
  | Reified -> Aast.Reified
