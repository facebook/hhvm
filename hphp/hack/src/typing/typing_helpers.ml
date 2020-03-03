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
open Typing_defs
open Typing_env_types
module TFTerm = Typing_func_terminality
module TUtils = Typing_utils
module Reason = Typing_reason
module Inst = Decl_instantiate
module Type = Typing_ops
module Env = Typing_env
module Inf = Typing_inference_env
module LEnv = Typing_lenv
module Async = Typing_async
module SubType = Typing_subtype
module Union = Typing_union
module Inter = Typing_intersection
module SN = Naming_special_names
module TVis = Typing_visibility
module TNBody = Typing_naming_body
module Phase = Typing_phase
module TOG = Typing_object_get
module Subst = Decl_subst
module ExprDepTy = Typing_dependent_type.ExprDepTy
module TCO = TypecheckerOptions
module EnvFromDef = Typing_env_from_def
module C = Typing_continuations
module CMap = C.Map
module Try = Typing_try
module TR = Typing_reactivity
module FL = FeatureLogging
module MakeType = Typing_make_type
module Cls = Decl_provider.Class
module Partial = Partial_provider
module Fake = Typing_fake_members
module TySet = Typing_set
module TPEnv = Type_parameter_env

exception InvalidPocketUniverse

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

  (* We will allow coercion to this expected type, if et_enforced=true *)
  val make_and_allow_coercion :
    Pos.t -> Typing_reason.ureason -> locl_possibly_enforced_ty -> t
end = struct
  (* Some mutually recursive inference functions in typing.ml pass around an ~expected argument that
   * enables bidirectional type checking. This module abstracts away that type so that it can be
   * extended and modified without having to touch every consumer. *)
  type t = {
    pos: Pos.t;
    reason: Typing_reason.ureason;
    ty: locl_possibly_enforced_ty;
        [@printer Pp_type.pp_possibly_enforced_ty Pp_type.pp_locl]
  }
  [@@deriving show]

  let make_and_allow_coercion pos reason ty = { pos; reason; ty }

  let make pos reason locl_ty =
    make_and_allow_coercion pos reason (MakeType.unenforced locl_ty)
end

(* Return a map describing all the fields in this record, including
   inherited fields, and whether they have a default value. *)
let all_record_fields (env : env) (rd : Decl_provider.record_def_decl) :
    (Aast.sid * Typing_defs.record_field_req) SMap.t =
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
