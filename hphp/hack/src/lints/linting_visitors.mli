(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type lint_env = {
  ctx: Provider_context.t;
  (* The file we are currently in *)
  cfile: Relative_path.t;
  (* The class we are currently in *)
  cclass: Nast.class_ option;
  (* The method we are currently in *)
  cmethod: Nast.method_ option;
  (* The function we are currently in *)
  cfun: Nast.fun_def option;
}

class type body_visitor =
  object
    inherit [unit] Nast.Visitor_DEPRECATED.visitor

    (* This allows lint rules to distinguish between the top-level block (which
     * is the function / method body) and the inner if-else / loop blocks *)
    method on_body : unit -> Nast.block -> unit
  end[@alert "-deprecated"]

module type BodyVisitorModule = sig
  (* each on_* method in the visitor should call its counterpart in the parent
   * visitor, so that all linters can be run in one single pass *)
  class visitor : lint_env -> body_visitor
end

module type BodyVisitorFunctor = functor (_ : BodyVisitorModule) ->
  BodyVisitorModule

class type ['a] file_visitor =
  object
    method on_file :
      'a -> Provider_context.t -> Relative_path.t -> Parser_return.t -> 'a

    method on_class : 'a -> lint_env -> Nast.class_ -> 'a

    method on_fun_def : 'a -> lint_env -> Nast.fun_def -> 'a

    method on_method : 'a -> lint_env -> Nast.class_ -> Nast.method_ -> 'a
  end

class virtual ['a] abstract_file_visitor :
  object
    method on_file :
      'a -> Provider_context.t -> Relative_path.t -> Parser_return.t -> 'a

    method on_class : 'a -> lint_env -> Nast.class_ -> 'a

    method on_fun_def : 'a -> lint_env -> Nast.fun_def -> 'a

    method on_method : 'a -> lint_env -> Nast.class_ -> Nast.method_ -> 'a
  end

class body_visitor_adder : (module BodyVisitorFunctor) -> [unit] file_visitor

(* Call this before any using add_visitor *)
val reset : unit -> unit

val add_visitor : Aast.sid -> (module BodyVisitorFunctor) -> unit

val body_visitor_invoker : unit file_visitor

val lint_all_bodies :
  (module BodyVisitorFunctor) ->
  Provider_context.t ->
  Relative_path.t ->
  Parser_return.t ->
  unit
