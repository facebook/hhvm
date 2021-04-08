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

(* See the type system specs for try for what's going on here *)

let get_cont_cont cont_cont_map cont1 cont2 =
  match CMap.find_opt cont1 cont_cont_map with
  | None -> None
  | Some cont_map -> CMap.find_opt cont2 cont_map

let make_new_cont union locals_map env cont =
  let ctx_cont_next = get_cont_cont locals_map cont C.Next in
  let ctxs_x_cont = CMap.map (CMap.find_opt cont) locals_map in
  CMap.fold_env env union ctxs_x_cont ctx_cont_next

let finally_merge union env locals_map all_conts =
  let make_and_add_new_cont env locals cont =
    let (env, ctxopt) = make_new_cont union locals_map env cont in
    let locals =
      match ctxopt with
      | None -> CMap.remove cont locals
      | Some ctx -> CMap.add cont ctx locals
    in
    (env, locals)
  in
  List.fold_left_env env ~f:make_and_add_new_cont ~init:CMap.empty all_conts
