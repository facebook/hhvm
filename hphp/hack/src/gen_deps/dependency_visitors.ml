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
open Hh_core


type dep_env = {
  popt: ParserOptions.t;
  (* Current toplevel entity we are in *)
  top : Dep.variant option;

  (* The namespace we are currently in *)
  nsenv: Namespace_env.env option;

  (* Whether to add extends dependencies *)
  extends: bool
}


let default_env popt = {
  popt;
  top = None;
  nsenv = None;
  extends = false;
}

let debug_mode = ref false
let dbg_dep_set = HashSet.create 0

(* Just print it out for now *)
let add_dep root obj =
  match root with
  | None -> ()
  (* The root is already namespace elaborated,
  so does not need to be canonicalized *)
  | Some r ->
    if !debug_mode then
    HashSet.add
      dbg_dep_set
      ((Dep.to_string r) ^ " -> " ^ (Dep.to_string obj))
    else
    Typing_deps.add_idep r obj

(* Check if the hint refers to a class of some sort,
  and then add a dependency if it's the case *)
let add_class_dep dep_env id =
  let {popt; top; nsenv; extends} = dep_env in
  match nsenv with
  | None -> assert (top = None)
  | Some nsenv ->
  let open Namespaces in
  let (_, ty_name) = elaborate_id nsenv ElaborateClass id in
  match NamingGlobal.GEnv.type_info popt ty_name with
  | Some _ ->
    if extends then
      add_dep top (Dep.Extends ty_name)
    else
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
    let add_fun_dep x =
      add_dep top (Dep.Fun x) in
    Option.iter (nm fun_name) ~f: add_fun_dep;
    Option.iter (nm global_name) ~f: add_fun_dep


let add_const_dep dep_env id =
  let {top; nsenv; popt; _} = dep_env in
  match nsenv with
  | None -> ()
  | Some nsenv ->
    let open Namespaces in
    let global_name = "\\"^(snd id) in
    let (_, const_name) = elaborate_id nsenv ElaborateConst id in
    let nm = NamingGlobal.GEnv.gconst_pos popt in
    let add_const_dep x =
      add_dep top (Dep.GConst x) in
    if Option.is_some (nm const_name) then add_const_dep const_name;
    if Option.is_some (nm global_name) then add_const_dep global_name;
    ()

let special_function = function
  | "class_meth"
  | "fun"
  | "hh_log_level"
  | "hh_show"
  | "hh_show_env"
  | "inst_meth"
  | "meth_caller"
  | "type_structure" -> true
  | _ -> false

class dependency_visitor = object(this)
  inherit [_] Ast_visitors_iter.iter as super
  method! on_fun_ dep_env f =
    let new_top = if snd f.f_name <> ";anonymous"
      then Some (Dep.Fun (snd f.f_name)) else dep_env.top in
    let dep_env = {
      dep_env with
      top = new_top;
      nsenv = Some f.f_namespace;
    } in
    super#on_fun_ dep_env f

  (* Given a list of hints, add extends deps to all of them *)
  method add_extends_deps dep_env l =
  List.iter l
  ~f:(fun h ->
    this#on_hint { dep_env with extends=true } h
  );
  method! on_catch dep_env (c0, c1, c2) =
    (* Add exceptions as class dependencies *)
    add_class_dep dep_env c0;
    super#on_catch dep_env (c0, c1, c2)

  method! on_Xml dep_env c0 c1 c2 =
    (* Add XHP classes *)
    add_class_dep dep_env c0;
    super#on_Xml dep_env c0 c1 c2

  method! on_ClassUse dep_env h =
    this#add_extends_deps dep_env [h];
    super#on_ClassUse dep_env h

  method! on_class_ dep_env c =
    let dep_env = {
      dep_env with
      top = Some (Dep.Class (snd c.c_name));
      nsenv = Some c.c_namespace;
    } in
    (* Add special enum class *)
    if c.c_kind = Ast_defs.Cenum then
      add_class_dep dep_env
      (Pos.none, Naming_special_names.Classes.cHH_BuiltinEnum);
    this#add_extends_deps dep_env c.c_extends;
    this#add_extends_deps dep_env c.c_implements;
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

  method! on_New dep_env c0 _c1 _c2 =
    match snd c0 with
    | Id id ->
      add_class_dep dep_env id
    | _ -> ();
    super#on_New dep_env c0 _c1 _c2

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
    | _, Id (_, "type_structure") ->
      (* Type structure is special and matches special classes *)
      (* We can just put the position as none here since
         we don't actually use the position to elaborate the namespaces
      *)
      add_class_dep dep_env (Pos.none, Naming_special_names.FB.cTypeStructure);
      add_class_dep dep_env (Pos.none, Naming_special_names.Classes.cTypename)
    (* Match on a direct call on a name(not a Lvar) *)
    (* Special function class_meth and "meth_caller" take class names *)
    | _, Id (_, "meth_caller")
    | _, Id (_, "class_meth") ->
      begin match el with
      (* String literals are valid as a classname for some reason *)
      | (_, String (p, s))::_ ->
        add_class_dep dep_env (p, s)
      (* Here, it could be a classname of the form Test::class, but that
        gets caught by on_hint. *)
      | _ -> ()
      end
    (* Special function fun *)
    | _, Id (_, "fun") ->
      begin
        match el with
        | (_, String (p, s))::[] ->
          add_fun_dep dep_env (p,s)
          (* Anything but a string here and it won't typecheck *)
        | _ -> ()
      end
    (* These special functions don't take any toplevel entities,
    but also aren't real functions, so we don't add their dependencies *)
    | _, Id (_, s) when special_function s -> ()
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

  method handle_class_const dep_env c0 c1 =
    begin match c0 with
    | _, (Id c0 | Lvar c0) -> add_class_dep dep_env c0;
    | _ -> ();
    end;
    begin match c1 with
    (* Special "::class" constant. Add the dependency to \\classname *)
    | (_, "class") ->
      add_class_dep dep_env (Pos.none, Naming_special_names.Classes.cClassname)
    | _ -> ()
    end

  method! on_Class_const dep_env c0 c1 =
    this#handle_class_const dep_env c0 c1;
    super#on_Class_const dep_env c0 c1

  method! on_SFclass_const dep_env c0 c1 =
    this#handle_class_const dep_env (Pos.none, Id c0) c1;
    super#on_SFclass_const dep_env c0 c1

end

let gen_deps popt ast =
  let env = default_env popt in
  let _ = (new dependency_visitor)#on_program env ast in
  ()

let print_deps popt ast =
  debug_mode := true;
  gen_deps popt ast;
  Typing_deps.print_string_hash_set dbg_dep_set
