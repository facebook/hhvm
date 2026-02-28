(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

(* this should never be exposed / thrown outside of this module; translate
 * it into a result type first *)
exception Not_static_exn of Pos.t

let rec static_string_exn = function
  | (_, _, Binop { bop = Ast_defs.Dot; lhs; rhs }) ->
    let s1 = static_string_exn lhs in
    let s2 = static_string_exn rhs in
    s1 ^ s2
  | (_, _, String s) -> s
  | (_, p, _) -> raise (Not_static_exn p)

let static_string (expr : Nast.expr) =
  try Ok (static_string_exn expr) with
  | Not_static_exn p -> Error p
