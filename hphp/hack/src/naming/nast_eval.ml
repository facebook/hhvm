(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast

(* this should never be exposed / thrown outside of this module; translate
 * it into a result type first *)
exception Not_static_exn of Pos.t

let rec static_string_exn = function
  | (_, Binop (Ast_defs.Dot, s1, s2)) ->
    let s1 = static_string_exn s1 in
    let s2 = static_string_exn s2 in
    s1 ^ s2
  | (_, String s) -> s
  | (p, _) -> raise (Not_static_exn p)

let static_string expr =
  (try Ok (static_string_exn expr) with Not_static_exn p -> Error p)
