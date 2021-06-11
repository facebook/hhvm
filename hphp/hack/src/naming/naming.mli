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

val program : Provider_context.t -> Nast.program -> Nast.program

(* Solves the local names within a function *)
val fun_def : Provider_context.t -> Nast.fun_def -> Nast.fun_def

(* Solves the local names of a class *)
val class_ : Provider_context.t -> Nast.class_ -> Nast.class_

val record_def : Provider_context.t -> Nast.record_def -> Nast.record_def

(* Solves the local names in an typedef *)
val typedef : Provider_context.t -> Nast.typedef -> Nast.typedef

(* Solves the local names in a global constant definition *)
val global_const : Provider_context.t -> Nast.gconst -> Nast.gconst

val invalid_expr_ : Pos.t -> Nast.expr_
