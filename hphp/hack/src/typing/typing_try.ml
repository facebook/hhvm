(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

[@@@warning "-33"] (* in OCaml 4.06.0, this can be inlined *)

open Hh_prelude
open Common

[@@@warning "+33"]

module C = Typing_continuations
module CMap = C.Map
module Env = Typing_env
module LEnv = Typing_lenv
module LEnvC = Typing_per_cont_env

(* See the type system specs for try for what's going on here *)

let get_cont_cont cont_cont_map cont1 cont2 =
  match CMap.find_opt cont1 cont_cont_map with
  | None -> None
  | Some cont_map -> CMap.find_opt cont2 cont_map

let make_new_cont locals_map env cont =
  match cont with
  | C.Next -> (env, get_cont_cont locals_map C.Finally C.Next)
  | _ ->
    let ctx_cont_cont = get_cont_cont locals_map cont cont in
    let ctxs_x_cont = CMap.map (CMap.find_opt cont) locals_map in
    let union env _key = LEnv.union_contextopts env in
    CMap.fold_env env union ctxs_x_cont ctx_cont_cont

let finally_merge env locals_map =
  let make_and_add_new_cont env locals cont =
    let (env, ctxopt) = make_new_cont locals_map env cont in
    (env, LEnvC.replace_cont cont ctxopt locals)
  in
  Env.all_continuations env
  |> List.fold_left_env env ~f:make_and_add_new_cont ~init:CMap.empty
