(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_env_types
module TL = Typing_logic

let with_error fail ((env, p) : env * TL.subtype_prop) : env * TL.subtype_prop =
  match (TL.get_error_if_unsat p, fail) with
  | (Some ty_err1, Some ty_err2) ->
    (env, TL.Disj (Some (Typing_error.both ty_err1 ty_err2), []))
  | _ -> (env, TL.conj p (TL.invalid ~fail))

let check_with b f r =
  if not b then
    with_error f r
  else
    r

let valid env : env * TL.subtype_prop = (env, TL.valid)

let ( &&& ) (env, p1) (f : env -> env * TL.subtype_prop) =
  match TL.get_error_if_unsat p1 with
  | Some ty_err1 ->
    let (env, p2) = f env in
    begin
      match TL.get_error_if_unsat p2 with
      | Some ty_err2 ->
        (env, TL.Disj (Some (Typing_error.both ty_err1 ty_err2), []))
      | None -> (env, p1)
    end
  | None ->
    let (env, p2) = f env in
    (env, TL.conj p1 p2)

let if_unsat (f : env -> env * TL.subtype_prop) (env, p) =
  if TL.is_unsat p then
    f env
  else
    (env, p)

let ( ||| ) ~fail (env, p1) (f : env -> env * TL.subtype_prop) =
  if TL.is_valid p1 then
    (env, p1)
  else
    let (env, p2) = f env in
    (env, TL.disj ~fail p1 p2)

(* We *know* that the assertion is unsatisfiable *)
let invalid ~fail env = (env, TL.invalid ~fail)
