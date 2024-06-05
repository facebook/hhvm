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

let valid env : env * TL.subtype_prop = (env, TL.valid)

let ( &&& ) (env, p1) (f : env -> env * TL.subtype_prop) =
  match TL.get_error_if_unsat p1 with
  | Some ty_err_opt1 ->
    let (env, p2) = f env in
    begin
      match TL.get_error_if_unsat p2 with
      | Some ty_err_opt2 ->
        let ty_err_opt =
          Typing_error.multiple_opt
          @@ List.filter_opt [ty_err_opt1; ty_err_opt2]
        in
        (env, TL.Disj (ty_err_opt, []))
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
