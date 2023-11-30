(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Hashtbl = Stdlib.Hashtbl
open Aast
open Nast

(****************************************************************************)
(* Attention for lint rule writers: This file contains a fair bit of
 * complexity that you probably don't need to bother with. For a simple
 * example linter that applies to all files in www, look at linter_js.ml. If
 * your lint rule should only apply to specific files / classes / methods,
 * look at linter_of_xcontrollers.ml. *)
(****************************************************************************)

type lint_env = {
  ctx: Provider_context.t;
  (* The file we are currently in *)
  cfile: Relative_path.t;
  (* The class we are currently in *)
  cclass: class_ option;
  (* The method we are currently in *)
  cmethod: method_ option;
  (* The function we are currently in *)
  cfun: fun_def option;
}

let default_env ctx cfile =
  { ctx; cfile; cclass = None; cmethod = None; cfun = None }

class body_visitor =
  object (this)
    inherit [unit] Nast.Visitor_DEPRECATED.visitor

    (* This allows lint rules to distinguish between the top-level block (which
     * is the function / method body) and the inner if-else / loop blocks *)
    method on_body = this#on_block
  end
  [@alert "-deprecated"]

module type BodyVisitorModule = sig
  (* each on_* method in the visitor should call its counterpart in the parent
   * visitor, so that all linters can be run in one single pass *)
  class visitor : lint_env -> body_visitor
end

(* A mapping of lint rule visitors to functions and methods, using the
 * position of the function / method as the map key. the various
 * file_visiters will add to this mapping. Once that's done,
 * body_visitor_invoker is used for running all the lint rules, and it will
 * do so in one pass over the named AST. *)
let (body_visitors : (Pos.t, (module BodyVisitorModule)) Hashtbl.t) =
  Hashtbl.create 0

let reset () = Hashtbl.reset body_visitors

module type BodyVisitorFunctor = functor (_ : BodyVisitorModule) ->
  BodyVisitorModule

module GenericBodyVisitor : BodyVisitorModule = struct
  class visitor _env : body_visitor =
    object
      inherit body_visitor
    end
end

let add_visitor (p, _) v =
  let module VisitorFunctor = (val v : BodyVisitorFunctor) in
  match Hashtbl.find_opt body_visitors p with
  | None ->
    let module M = VisitorFunctor (GenericBodyVisitor) in
    Hashtbl.add body_visitors p (module M : BodyVisitorModule)
  | Some v' ->
    let module M' = (val v') in
    let module M = VisitorFunctor (M') in
    Hashtbl.replace body_visitors p (module M : BodyVisitorModule)

class virtual ['a] abstract_file_visitor :
  object
    method on_file :
      'a -> Provider_context.t -> Relative_path.t -> Parser_return.t -> 'a

    method on_class : 'a -> lint_env -> Nast.class_ -> 'a

    method on_fun_def : 'a -> lint_env -> Nast.fun_def -> 'a

    method on_method : 'a -> lint_env -> Nast.class_ -> Nast.method_ -> 'a
  end =
  object (this)
    method on_file context ctx fn pr =
      let ast = pr.Parser_return.ast in
      let env = default_env ctx fn in
      List.fold_left ast ~init:context ~f:(fun context def ->
          match def with
          | Fun f ->
            let f = Errors.ignore_ (fun () -> Naming.fun_def ctx f) in
            this#on_fun_def context env f
          | Class c ->
            let c = Errors.ignore_ (fun () -> Naming.class_ ctx c) in
            this#on_class context env c
          | _ -> context)

    method on_class context env class_ =
      let on_method context meth = this#on_method context env class_ meth in
      List.fold_left class_.c_methods ~init:context ~f:on_method

    method on_fun_def context _env _fun = context

    method on_method context _env _class _method = context
  end

class type ['a] file_visitor =
  object
    method on_file :
      'a -> Provider_context.t -> Relative_path.t -> Parser_return.t -> 'a

    method on_class : 'a -> lint_env -> Nast.class_ -> 'a

    method on_fun_def : 'a -> lint_env -> Nast.fun_def -> 'a

    method on_method : 'a -> lint_env -> Nast.class_ -> Nast.method_ -> 'a
  end

class body_visitor_adder body_visitor =
  object
    inherit [unit] abstract_file_visitor

    method! on_fun_def () _env fd = add_visitor fd.fd_name body_visitor

    method! on_method () _env _class meth = add_visitor meth.m_name body_visitor
  end

let body_visitor_invoker =
  object
    inherit [unit] abstract_file_visitor

    method! on_fun_def () env fd =
      let fun_ = fd.fd_fun in
      let module Visitor = (val Hashtbl.find body_visitors (fst fd.fd_name)) in
      let nb = fun_.f_body in
      let env = { env with cfun = Some fd } in
      (new Visitor.visitor env)#on_body () nb.fb_ast

    method! on_method () env class_ meth =
      let module Visitor = (val Hashtbl.find body_visitors (fst meth.m_name)) in
      let nb = meth.m_body in
      let env = { env with cclass = Some class_; cmethod = Some meth } in
      (new Visitor.visitor env)#on_body () nb.fb_ast
  end

(****************************************************************************)
(* Convenience functions *)
(****************************************************************************)

let lint_all_bodies m = (new body_visitor_adder m)#on_file ()
