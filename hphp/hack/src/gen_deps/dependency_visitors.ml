(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* This file defines a visitor on the AST which calculates dependencies
 * between toplevel entities in Hack. It grabs toplevel entities from the
 * global naming table and adds the corresponding dependencies.
 *)
open Ast


type toplevel =
| Class of string
| Function of string
| Typedef of string
| Const of string
| Other


type dep_env = {
  popt: ParserOptions.t;
  (* Current toplevel entity we are in *)
  top : toplevel;

  (* The namespace we are currently in *)
  nsenv: Namespace_env.env option;
}


let default_env popt = {
  popt;
  top = Other;
  nsenv = None;
}

let toplevel_to_string = function
| Class s -> "class "^s
| Function f -> "function "^f
| Typedef t -> "typedef "^t
| Const c -> "const "^c
| Other -> "*UNKNOWN*"

(* Just print it out for now *)
let add_dep t1 t2 =
  Printf.printf "%s -> %s\n"
    (toplevel_to_string t1)
    (toplevel_to_string t2)

(* Check if the hint refers to a class of some sort,
  and then add a dependency if it's the case *)
let add_class_dep dep_env id =
  let {popt; top; nsenv } = dep_env in
  match nsenv with
  | None -> assert (top = Other)
  | Some nsenv ->
  let open Namespaces in
  let (_, ty_name) = elaborate_id nsenv ElaborateClass id in
  match NamingGlobal.GEnv.type_info popt ty_name with
  | Some (_, `Class) ->
    add_dep top (Class ty_name)
  | Some (_, `Typedef) ->
    add_dep top (Typedef ty_name)
  | None -> ()

let add_fun_dep dep_env id =
  let {top; nsenv; _} = dep_env in
  match nsenv with
    | None -> ()
    | Some nsenv ->
    let open Namespaces in
    let global_name = "\\"^(snd id) in
    let (_, fun_name) = elaborate_id nsenv ElaborateFun id in
    let nm = NamingGlobal.GEnv.fun_canon_name in
    match nm fun_name, nm global_name with
    | Some name, _
    | _, Some name ->
      add_dep top (Function name)
    | None, None -> ()

class dependency_visitor = object
  inherit [_] Ast_visitors_iter.iter as super
  method! on_fun_ dep_env f =
    let dep_env = {
      dep_env with
      top = Function (snd f.f_name);
      nsenv = Some f.f_namespace;
    } in
    super#on_fun_ dep_env f

  method! on_class_ dep_env c =
    let dep_env = {
      dep_env with
      top = Class (snd c.c_name);
      nsenv = Some c.c_namespace;
    } in
    super#on_class_ dep_env c

  method! on_typedef dep_env t =
  let dep_env = {
    dep_env with
    top = Typedef (snd t.t_id);
    nsenv = Some t.t_namespace;
  } in
  super#on_typedef dep_env t

  method! on_gconst dep_env c =
  let dep_env = {
    dep_env with
    top = Const (snd c.cst_name);
    nsenv = Some c.cst_namespace;
  } in
  super#on_gconst dep_env c

  method! on_hint dep_env hint =
    (match (snd hint) with
    | Happly (id, _)
    (* Only care about the base class's name *)
    | Haccess (id, _, _) ->
      add_class_dep dep_env id
    (* No need to recurse here, since the parent class does it for us  *)
    | Hshape _
    | Hoption _
    | Hsoft _
    | Hfun _
    | Htuple _ -> ());
    super#on_hint dep_env hint

  method! on_Call dep_env exp hl el el2 =
    (match exp with
    (* Match on a direct call on a name(not a Lvar) *)
    | _, Id id ->
      add_fun_dep dep_env id
    | _ -> ());
    super#on_Call dep_env exp hl el el2
end

let print_deps popt ast =
  (* Elaborate the namespaces away *)
  let env = default_env popt in
  let _ = (new dependency_visitor)#on_program env ast in
  ()
