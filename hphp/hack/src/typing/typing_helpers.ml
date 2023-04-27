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
    coerce: Typing_logic.coercion_direction option;
  }
  [@@deriving show]

  [@@@warning "+32"]

  val make :
    ?coerce:Typing_logic.coercion_direction option ->
    Pos.t ->
    Typing_reason.ureason ->
    locl_ty ->
    t

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
    coerce: Typing_logic.coercion_direction option;
  }
  [@@deriving show]

  let make_and_allow_coercion_opt env pos reason ty =
    let (_env, ety) = Env.expand_type env ty.et_type in
    if is_tyvar ety then
      None
    else
      Some { pos; reason; ty; coerce = None }

  let make_and_allow_coercion pos reason ty = { pos; reason; ty; coerce = None }

  let make ?coerce pos reason locl_ty =
    let res =
      make_and_allow_coercion pos reason (MakeType.unenforced locl_ty)
    in
    match coerce with
    | None -> res
    | Some coerce -> { res with coerce }
end

let decl_error_to_typing_error decl_error =
  let primary =
    match decl_error with
    | Decl_defs.Wrong_extend_kind
        { pos; kind; name; parent_pos; parent_kind; parent_name } ->
      Typing_error.Primary.Wrong_extend_kind
        { pos; kind; name; parent_pos; parent_kind; parent_name }
    | Decl_defs.Cyclic_class_def { pos; stack } ->
      Typing_error.Primary.Cyclic_class_def { pos; stack }
  in
  Typing_error.primary primary

let add_decl_error decl_error =
  Errors.add_typing_error (decl_error_to_typing_error decl_error)

let add_decl_errors = List.iter ~f:add_decl_error

(*****************************************************************************)
(* Handling function/method arguments *)
(*****************************************************************************)
let param_has_attribute param attr =
  List.exists param.param_user_attributes ~f:(fun { ua_name; _ } ->
      String.equal attr (snd ua_name))

let has_accept_disposable_attribute param =
  param_has_attribute param SN.UserAttributes.uaAcceptDisposable

let with_timeout env fun_name (do_ : env -> 'b) : 'b option =
  let timeout = TypecheckerOptions.(timeout @@ Env.get_tcopt env) in
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
        let (pos, fn_name) = fun_name in
        Errors.add_typing_error
          Typing_error.(
            primary
            @@ Primary.Typechecker_timeout { pos; fn_name; seconds = timeout });
        None)
      ~do_:(fun _ -> Some (do_ env))

(* If the localized types of the return type is a tyvar we force it to be covariant.
   The same goes for parameter types, but this time we force them to be contravariant
*)
let set_tyvars_variance_in_callable env return_ty param_tys =
  Env.log_env_change "set_tyvars_variance_in_callable" env
  @@
  let set_variance = Env.set_tyvar_variance ~for_all_vars:true in
  let env = set_variance env return_ty in
  let env =
    List.fold param_tys ~init:env ~f:(fun env opt_ty ->
        match opt_ty with
        | None -> env
        | Some ty -> set_variance ~flip:true env ty)
  in
  env

let reify_kind = function
  | Erased -> Aast.Erased
  | SoftReified -> Aast.SoftReified
  | Reified -> Aast.Reified

let merge_hint_with_decl_hint env type_hint decl_ty =
  let contains_tvar decl_ty =
    match decl_ty with
    | None -> false
    | Some decl_ty -> Typing_utils.contains_tvar_decl decl_ty
  in
  if contains_tvar decl_ty then
    decl_ty
  else
    Option.map type_hint ~f:(Decl_hint.hint env.decl_env)

let merge_decl_header_with_hints ~params ~ret decl_header env =
  let ret_decl_ty =
    merge_hint_with_decl_hint
      env
      (hint_of_type_hint ret)
      (Option.map
         ~f:(fun { ft_ret = { et_type; _ }; _ } -> et_type)
         decl_header)
  in
  let params_decl_ty =
    match decl_header with
    | None ->
      List.map
        ~f:(fun h ->
          merge_hint_with_decl_hint
            env
            (hint_of_type_hint h.param_type_hint)
            None)
        params
    | Some { ft_params; _ } ->
      List.zip_exn params ft_params
      |> List.map ~f:(fun (h, { fp_type = { et_type; _ }; _ }) ->
             merge_hint_with_decl_hint
               env
               (hint_of_type_hint h.param_type_hint)
               (Some et_type))
  in
  (ret_decl_ty, params_decl_ty)
