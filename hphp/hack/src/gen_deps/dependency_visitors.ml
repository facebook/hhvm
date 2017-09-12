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
open Typing_deps


type dep_env = {
  popt: ParserOptions.t;
  (* Current toplevel entity we are in *)
  top : Dep.variant option;

  (* The namespace we are currently in *)
  nsenv: Namespace_env.env option;
}


let default_env popt = {
  popt;
  top = None;
  nsenv = None;
}

let dbg_dep_set = HashSet.create 0

(* Just print it out for now *)
let add_dep root obj =
  match root with
  | None -> ()
  (* The root is already namespace elaborated,
  so does not need to be canonicalized *)
  | Some r ->
    HashSet.add
      dbg_dep_set
      ((Dep.to_string r) ^ " -> " ^ (Dep.to_string obj))


(* Check if the hint refers to a class of some sort,
  and then add a dependency if it's the case *)
let add_class_dep dep_env id =
  let {popt; top; nsenv } = dep_env in
  match nsenv with
  | None -> assert (top = None)
  | Some nsenv ->
  let open Namespaces in
  let (_, ty_name) = elaborate_id nsenv ElaborateClass id in
  match NamingGlobal.GEnv.type_info popt ty_name with
  | Some _ ->
    add_dep top (Dep.Class ty_name)
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
      add_dep top (Dep.Fun name)
    | None, None -> ()

let add_const_dep dep_env id =
  let {top; nsenv; popt;} = dep_env in
  match nsenv with
  | None -> ()
  | Some nsenv ->
    let open Namespaces in
    let global_name = "\\"^(snd id) in
    let (_, const_name) = elaborate_id nsenv ElaborateConst id in
    let nm = NamingGlobal.GEnv.gconst_pos popt in
    match nm const_name, nm global_name with
    | Some _, _ ->
      add_dep top (Dep.GConst const_name)
    | _, Some _ ->
      add_dep top (Dep.GConst global_name)
    | None, None -> ()


class dependency_visitor = object
  inherit [_] Ast_visitors_iter.iter as super
  method! on_fun_ dep_env f =
    let dep_env = {
      dep_env with
      top = Some (Dep.Fun (snd f.f_name));
      nsenv = Some f.f_namespace;
    } in
    super#on_fun_ dep_env f

  method! on_class_ dep_env c =
    let dep_env = {
      dep_env with
      top = Some (Dep.Class (snd c.c_name));
      nsenv = Some c.c_namespace;
    } in
    super#on_class_ dep_env c

  method! on_typedef dep_env t =
  let dep_env = {
    dep_env with
    top = Some (Dep.Class (snd t.t_id));
    nsenv = Some t.t_namespace;
  } in
  super#on_typedef dep_env t

  method! on_gconst dep_env c =
  let dep_env = {
    dep_env with
    top = Some (Dep.GConst (snd c.cst_name));
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

  method! on_id dep_env id =
    (* There's a minor bug here where if a global constant
      and a function are named exactly the same thing, we may end up adding
      an extra dependency when calling a function. Since there's
      no good way of telling whether this id is associated with a function call
      or not, we'll have to accept the false positive for now.
    *)
    add_const_dep dep_env id;
    super#on_id dep_env id

  (* Class constants: only care about the toplevel class*)
  method! on_Class_const dep_env c0 _c1 =
    add_class_dep dep_env c0

  method! on_SFclass_const dep_env c0 _c1 =
    add_class_dep dep_env c0

end

let print_deps popt ast =
  (* Elaborate the namespaces away *)
  let env = default_env popt in
  let _ = (new dependency_visitor)#on_program env ast in
  Typing_deps.print_string_hash_set dbg_dep_set
