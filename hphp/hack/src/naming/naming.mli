(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Module "naming" a program.
 * Transform all the local names into a unique identifier
 *)

val program : Nast.program -> Nast.program

(* Solves the local names within a function *)
val fun_ : Nast.fun_ -> Nast.fun_

(* Solves the local names of a class *)
val class_ : Nast.class_ -> Nast.class_

val record_def : Nast.record_def -> Nast.record_def

(* Solves the local names in an typedef *)
val typedef : Nast.typedef -> Nast.typedef

(* Solves the local names in a global constant definition *)
val global_const : Nast.gconst -> Nast.gconst

module type GetLocals = sig
  val lvalue :
    Namespace_env.env * Pos.t SMap.t ->
    Nast.expr ->
    Namespace_env.env * Pos.t SMap.t

  val stmt :
    Namespace_env.env * Pos.t SMap.t ->
    Nast.stmt ->
    Namespace_env.env * Pos.t SMap.t
end

module Make (GetLocals : GetLocals) : sig
  (* Solves the local names in a function body *)
  val func_body : Nast.fun_ -> Nast.func_body

  (* Solves the local names in class method bodies *)
  val class_meth_bodies : Nast.class_ -> Nast.class_
end
