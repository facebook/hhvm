(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Aast = Ast_to_nast.Aast

(** Module "naming" a program.
 * Transform all the local names into a unique identifier
 *)

val program: TypecheckerOptions.t -> Ast.program -> Nast.program

(* Solves the local names within a function *)
val fun_: TypecheckerOptions.t -> Ast.fun_ -> Nast.fun_

(* Solves the local names of a class *)
val class_: TypecheckerOptions.t -> Ast.class_ -> Nast.class_

(* Solves the local names in an typedef *)
val typedef: TypecheckerOptions.t -> Ast.typedef -> Nast.typedef

(* Solves the local names in a global constant definition *)
val global_const: TypecheckerOptions.t -> Ast.gconst -> Nast.gconst

module type GetLocals = sig
  val stmt :
    TypecheckerOptions.t -> Namespace_env.env * Pos.t SMap.t ->
      Ast.stmt -> Namespace_env.env * Pos.t SMap.t
  val lvalue :
    TypecheckerOptions.t -> Namespace_env.env * Pos.t SMap.t ->
      Ast.expr -> Namespace_env.env * Pos.t SMap.t

  val aast_lvalue : TypecheckerOptions.t -> Namespace_env.env * Pos.t SMap.t ->
    Aast.expr -> Namespace_env.env * Pos.t SMap.t
  val aast_stmt : TypecheckerOptions.t -> Namespace_env.env * Pos.t SMap.t ->
    Aast.stmt -> Namespace_env.env * Pos.t SMap.t
end

module Make : functor (GetLocals : GetLocals) -> sig
  (* Solves the local names in a function body *)
  val func_body: TypecheckerOptions.t -> Nast.fun_ -> Nast.func_body

  (* Solves the local names in class method bodies *)
  val class_meth_bodies: TypecheckerOptions.t -> Nast.class_ -> Nast.class_

end
