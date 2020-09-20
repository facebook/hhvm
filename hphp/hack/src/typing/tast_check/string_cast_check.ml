(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Env = Tast_env
module TCO = TypecheckerOptions
module SN = Naming_special_names

(** Produce an error on (string) casts of objects. Currently it is allowed in HHVM to
    cast an object if it is Stringish (i.e., has a __toString() method), but all
    (string) casts of objects will be banned in the future. Eventually,
    __toString/(string) casts of objects will be removed from HHVM entirely. *)

let check__toString m =
  let (pos, name) = m.m_name in
  if String.equal name SN.Members.__toString then (
    if (not (Aast.equal_visibility m.m_visibility Public)) || m.m_static then
      Errors.toString_visibility pos;
    match hint_of_type_hint m.m_ret with
    | Some (_, Hprim Tstring) -> ()
    | Some (p, _) -> Errors.toString_returns_string p
    | None -> ()
  )

let rec is_stringish env ty =
  let (env, ety) = Env.expand_type env ty in
  match get_node ety with
  | Toption ty' -> is_stringish env ty'
  | Tunion tyl -> List.for_all ~f:(is_stringish env) tyl
  | Tintersection tyl -> List.exists ~f:(is_stringish env) tyl
  | Tgeneric _
  | Tnewtype _
  | Tdependent _ ->
    let (env, tyl) = Env.get_concrete_supertypes env ty in
    List.for_all ~f:(is_stringish env) tyl
  | Tclass (x, _, _) -> Option.is_none (Env.get_class env (snd x))
  | Tany _
  | Terr
  | Tdynamic
  | Tobject
  | Tnonnull
  | Tprim _
  | Tpu _ ->
    true
  | Tvarray _
  | Tdarray _
  | Tvarray_or_darray _
  | Tvar _
  | Ttuple _
  | Tfun _
  | Tshape _
  | Tpu_type_access _ ->
    false
  | Tunapplied_alias _ ->
    Typing_defs.error_Tunapplied_alias_in_illegal_context ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env ((p, _), expr) =
      match expr with
      | Cast ((_, Hprim Tstring), te) ->
        let ((_, ty), _) = te in
        if not (is_stringish env ty) then
          Errors.string_cast p (Env.print_ty env ty)
      | _ -> ()

    method! at_method_ _ m = check__toString m
  end
