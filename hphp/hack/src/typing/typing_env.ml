(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

include Typing_env_types
open Core
open Decl_env
open Typing_defs
open Nast

module SN = Naming_special_names
module Dep = Typing_deps.Dep
module TLazyHeap = Typing_lazy_heap
module LEnvC = Typing_lenv_cont
module Cont = Typing_continuations


let ( ++ ) x y = Typing_set.add x y

let get_tcopt env = env.genv.tcopt
let fresh () =
  Ident.tmp()

let fresh_type () =
  Reason.none, Tvar (Ident.tmp())

let add_subst env x x' =
  if x <> x'
  then { env with subst = IMap.add x x' env.subst }
  else env

(* Apply variable-to-variable substitution from environment. Update environment
   if we ended up iterating (cf path compression in union-find) *)
let rec get_var env x =
  let x' = IMap.get x env.subst in
  (match x' with
  | None -> env, x
  | Some x' ->
      let env, x' = get_var env x' in
      let env = add_subst env x x' in
      env, x'
  )

(* This is basically union from union-find, but without balancing
 *  (linking the smaller tree to the larger tree). In practice this
 * isn't important: path compression is much more significant. *)
let rename env x x' =
  let env, x = get_var env x in
  let env, x' = get_var env x' in
  let env = add_subst env x x' in
  env

let add env x ty =
  let env, x = get_var env x in
  { env with tenv = IMap.add x ty env.tenv }

let fresh_unresolved_type env =
  let v = Ident.tmp () in
  add env v (Reason.Rnone, Tunresolved []), (Reason.Rnone, Tvar v)

let get_type env x_reason x =
  let env, x = get_var env x in
  let ty = IMap.get x env.tenv in
  match ty with
  | None -> env, (x_reason, Tany)
  | Some ty -> env, ty

let get_type_unsafe env x =
  let ty = IMap.get x env.tenv in
  match ty with
  | None ->
      env, (Reason.none, Tany)
  | Some ty -> env, ty

let expand_type env x =
  match x with
  | r, Tvar x -> get_type env r x
  | x -> env, x

let make_ft p params ret_ty =
  let arity = List.length params in
  {
    ft_pos      = p;
    ft_deprecated = None;
    ft_abstract = false;
    ft_arity    = Fstandard (arity, arity);
    ft_tparams  = [];
    ft_where_constraints = [];
    ft_params   = params;
    ft_ret      = ret_ty;
  }

let get_shape_field_name = function
  | Ast.SFlit (_, s) -> s
  | Ast.SFclass_const ((_, s1), (_, s2)) -> s1^"::"^s2

let empty_bounds = TySet.empty
let singleton_bound ty = TySet.singleton ty

let get_lower_bounds env name =
  let local =
  match SMap.get name env.lenv.tpenv with
  | None -> empty_bounds
  | Some {lower_bounds; _} -> lower_bounds in
  let global =
  match SMap.get name env.global_tpenv with
  | None -> empty_bounds
  | Some {lower_bounds; _} -> lower_bounds in
  TySet.elements (TySet.union local global)

let get_upper_bounds env name =
  let local =
  match SMap.get name env.lenv.tpenv with
  | None -> empty_bounds
  | Some {upper_bounds; _} -> upper_bounds in
  let global =
  match SMap.get name env.global_tpenv with
  | None -> empty_bounds
  | Some {upper_bounds; _} -> upper_bounds in
  TySet.elements (TySet.union local global)

(* Get bounds that are both an upper and lower of a given generic *)
let get_equal_bounds env name =
  let lower = TySet.of_list (get_lower_bounds env name) in
  let upper = TySet.of_list (get_upper_bounds env name) in
  TySet.elements (TySet.inter lower upper)

(* Add a single new upper bound [ty] to generic parameter [name] in [tpenv] *)
let add_upper_bound_ tpenv name ty =
  match SMap.get name tpenv with
  | None ->
    SMap.add name
      {lower_bounds = empty_bounds; upper_bounds = singleton_bound ty} tpenv
  | Some {lower_bounds; upper_bounds} ->
    SMap.add name
      {lower_bounds; upper_bounds = ty++upper_bounds} tpenv

(* Add a single new lower bound [ty] to generic parameter [name] in [tpenv] *)
let add_lower_bound_ tpenv name ty =
  match SMap.get name tpenv with
  | None ->
    SMap.add name
      {lower_bounds = singleton_bound ty; upper_bounds = empty_bounds} tpenv
  | Some {lower_bounds; upper_bounds} ->
    SMap.add name
      {lower_bounds = ty++lower_bounds; upper_bounds} tpenv

let env_with_tpenv env tpenv =
  { env with lenv = { env.lenv with tpenv = tpenv } }

let union_global_tpenv tp1 tp2 =
  SMap.union tp1 tp2 ~combine:(fun _ v1 v2 ->
    let {lower_bounds=low1; upper_bounds=up1} = v1 in
    let {lower_bounds=low2; upper_bounds=up2} = v2 in
    Some  {lower_bounds=TySet.union low1 low2; upper_bounds=TySet.union up1 up2}
  )

let add_upper_bound_global env name ty =
  let tpenv =
    begin match ty with
    | (r, Tabstract (AKgeneric formal_super, _)) ->
      add_lower_bound_ env.global_tpenv formal_super
        (r, Tabstract (AKgeneric name, None))
    | _ -> env.global_tpenv
    end in
   { env with global_tpenv=(add_upper_bound_ tpenv name ty) }

let add_lower_bound_global env name ty =
 let tpenv =
   begin match ty with
   | (r, Tabstract (AKgeneric formal_sub, _)) ->
     add_upper_bound_ env.global_tpenv formal_sub
       (r, Tabstract (AKgeneric name, None))
   | _ -> env.global_tpenv
   end in
  { env with global_tpenv=(add_lower_bound_ tpenv name ty) }

let add_upper_bound env name ty =
  let tpenv =
    begin match ty with
    | (r, Tabstract (AKgeneric formal_super, _)) ->
      add_lower_bound_ env.lenv.tpenv formal_super
        (r, Tabstract (AKgeneric name, None))
    | _ -> env.lenv.tpenv
    end in
  env_with_tpenv env (add_upper_bound_ tpenv name ty)

let add_lower_bound env name ty =
  let tpenv =
    begin match ty with
    | (r, Tabstract (AKgeneric formal_sub, _)) ->
      add_upper_bound_ env.lenv.tpenv formal_sub
        (r, Tabstract (AKgeneric name, None))
    | _ -> env.lenv.tpenv
    end in
  env_with_tpenv env (add_lower_bound_ tpenv name ty)

(* Add type parameters to environment, initially with no bounds.
 * Existing type parameters with the same name will be overridden. *)
let add_generic_parameters env tparaml =
  let add_empty_bounds tpenv (_, (_, name), _) =
    SMap.add name {lower_bounds = empty_bounds;
                   upper_bounds = empty_bounds} tpenv in
  env_with_tpenv env
    (List.fold_left tparaml ~f:add_empty_bounds ~init:env.lenv.tpenv)

let is_generic_parameter env name =
  SMap.mem name env.lenv.tpenv

let get_generic_parameters env =
  SMap.keys (SMap.union env.lenv.tpenv env.global_tpenv)

let get_tpenv_size env =
  let local = SMap.fold (fun _x { lower_bounds; upper_bounds } count ->
    count + TySet.cardinal lower_bounds + TySet.cardinal upper_bounds)
    env.lenv.tpenv 0 in
    SMap.fold (fun _x { lower_bounds; upper_bounds } count ->
      count + TySet.cardinal lower_bounds + TySet.cardinal upper_bounds)
      env.global_tpenv local

(* Generate a fresh generic parameter with a specified prefix but distinct
 * from all generic parameters in the environment *)
let add_fresh_generic_parameter env prefix =
  let rec iterate i =
    let name = Printf.sprintf "%s#%d" prefix i in
    if is_generic_parameter env name then iterate (i+1) else name in
  let name = iterate 1 in
  let env =
    env_with_tpenv env
      (SMap.add name {lower_bounds = empty_bounds;
                      upper_bounds = empty_bounds} env.lenv.tpenv) in
  env, name


(* Replace types for locals with empty environment *)
let env_with_locals env locals history =
  { env with lenv = {
      env.lenv with local_types = locals; local_type_history = history;
    }
  }

let empty_fake_members = {
  last_call = None;
  invalid   = SSet.empty;
  valid     = SSet.empty;
}

let empty_local tpenv = {
  tpenv = tpenv;
  fake_members = empty_fake_members;
  local_types = Typing_continuations.Map.empty;
  local_type_history = Local_id.Map.empty;
}

let empty tcopt file ~droot = {
  pos     = Pos.none;
  outer_pos = Pos.none;
  outer_reason = Reason.URnone;
  tenv    = IMap.empty;
  subst   = IMap.empty;
  lenv    = empty_local SMap.empty;
  todo    = [];
  in_loop = false;
  decl_env = {
    mode = FileInfo.Mstrict;
    droot;
    decl_tcopt = tcopt;
  };
  genv    = {
    tcopt   = tcopt;
    return  = fresh_type();
    self_id = "";
    self    = Reason.none, Tany;
    static  = false;
    parent_id = "";
    parent  = Reason.none, Tany;
    fun_kind = Ast.FSync;
    anons   = IMap.empty;
    file    = file;
  };
  global_tpenv = SMap.empty
}

let add_wclass env x =
  let dep = Dep.Class x in
  Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep);
  ()

let get_typedef env x =
  add_wclass env x;
  TLazyHeap.get_typedef env.genv.tcopt x

let is_typedef x =
  match Naming_heap.TypeIdHeap.get x with
  | Some (_p, `Typedef) -> true
  | _ -> false

let get_class env x =
  add_wclass env x;
  TLazyHeap.get_class env.genv.tcopt x

let get_enum_constraint env x =
  match get_class env x with
  | None -> None
  | Some tc ->
    match tc.tc_enum_type with
    | None -> None
    | Some e -> e.te_constraint

let add_wclass env x =
  let dep = Dep.Class x in
  Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep);
  ()

(* When we want to type something with a fresh typing environment *)
let fresh_tenv env f =
  f { env with
      todo = [];
      lenv = empty_local env.lenv.tpenv;
      tenv = IMap.empty;
      in_loop = false
    }

let get_enum env x =
  match TLazyHeap.get_class env.genv.tcopt x with
  | Some tc when tc.tc_enum_type <> None -> Some tc
  | _ -> None

let is_enum env x = get_enum env x <> None

let get_typeconst env class_ mid =
  add_wclass env class_.tc_name;
  let dep = Dep.Const (class_.tc_name, mid) in
  Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep);
  SMap.get mid class_.tc_typeconsts

(* Used to access class constants. *)
let get_const env class_ mid =
  add_wclass env class_.tc_name;
  let dep = Dep.Const (class_.tc_name, mid) in
  Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep);
  SMap.get mid class_.tc_consts

(* Used to access "global constants". That is constants that were
 * introduced with "const X = ...;" at topelevel, or "define('X', ...);"
 *)
let get_gconst env cst_name =
  let dep = Dep.GConst cst_name in
  Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep);
  TLazyHeap.get_gconst env.genv.tcopt cst_name

let get_static_member is_method env class_ mid =
  add_wclass env class_.tc_name;
  let add_dep x =
    let dep = if is_method then Dep.SMethod (x, mid)
      else Dep.SProp (x, mid) in
    Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep);
  in
  add_dep class_.tc_name;
  (* The type of a member is stored separately in the heap. This means that
   * any user of the member also has a dependency on the class where the member
   * originated.
   *)
  let ce_opt = if is_method then SMap.get mid class_.tc_smethods
    else SMap.get mid class_.tc_sprops in
  Option.iter ce_opt (fun ce -> add_dep ce.ce_origin);
  ce_opt

let suggest_member members mid =
  let members = SMap.fold begin fun x {ce_type = lazy (r, _); _} acc ->
    let pos = Reason.to_pos r in
    SMap.add (String.lowercase x) (pos, x) acc
  end members SMap.empty
  in
  SMap.get mid members

let suggest_static_member is_method class_ mid =
  let mid = String.lowercase mid in
  let members = if is_method then class_.tc_smethods else class_.tc_sprops in
  suggest_member members mid

let get_member is_method env class_ mid =
  add_wclass env class_.tc_name;
  let add_dep x =
    let dep = if is_method then Dep.Method (x, mid)
      else Dep.Prop (x, mid) in
    Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep)
  in
  add_dep class_.tc_name;
  (* The type of a member is stored separately in the heap. This means that
   * any user of the member also has a dependency on the class where the member
   * originated.
   *)
  let ce_opt = if is_method then (SMap.get mid class_.tc_methods)
    else SMap.get mid class_.tc_props in
  Option.iter ce_opt (fun ce -> add_dep ce.ce_origin);
  ce_opt

let suggest_member is_method class_ mid =
  let mid = String.lowercase mid in
  let members = if is_method then class_.tc_methods else class_.tc_props in
  suggest_member members mid

let get_construct env class_ =
  add_wclass env class_.tc_name;
  let add_dep x =
    let dep = Dep.Cstr (x) in
    Option.iter env.decl_env.Decl_env.droot
      (fun root -> Typing_deps.add_idep root dep);
  in
  add_dep class_.tc_name;
  Option.iter (fst class_.tc_construct) (fun ce -> add_dep ce.ce_origin);
  class_.tc_construct

let check_todo env =
  let env, remaining =
    List.fold_left env.todo ~f:(fun (env, remaining) f ->
      let env, remove = f env in
      if remove then env, remaining else env, f::remaining)
      ~init:(env, []) in
  { env with todo = List.rev remaining }

let get_return env =
  env.genv.return

let set_return env x =
  let genv = env.genv in
  let genv = { genv with return = x } in
  { env with genv = genv }

let with_return env f =
  let ret = get_return env in
  let env, result = f env in
  set_return env ret, result

let is_static env = env.genv.static
let get_self env = env.genv.self
let get_self_id env = env.genv.self_id
let is_outside_class env = (env.genv.self_id = "")
let get_parent env = env.genv.parent
let get_parent_id env = env.genv.parent_id

let get_fn_kind env = env.genv.fun_kind

let get_file env = env.genv.file

let get_fun env x =
  let dep = Dep.Fun x in
  Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep);
  TLazyHeap.get_fun env.genv.tcopt x

let set_fn_kind env fn_type =
  let genv = env.genv in
  let genv = { genv with fun_kind = fn_type } in
  { env with genv = genv }

(* Add a function on environments that gets run at some later stage to check
 * constraints, by which time unresolved type variables may be resolved.
 * Because the validity of the constraint might depend on tpenv
 * at the point that the `add_todo` is called, we extend the environment at
 * the point that the function gets run with `tpenv` captured at the point
 * that `add_todo` gets called.
 * Typical examples are `instanceof` tests that introduce bounds on fresh
 * type parameters (e.g. named T#1) or on existing type parameters, which
 * are removed after the end of the `instanceof` conditional block. e.g.
 *   function foo<T as arraykey>(T $x): void { }
 *   class C<+T> { }
 *   class D extends C<arraykey> { }
 *   function test<Tu>(C<Tu> $x, Tu $y): void {
 *   if ($x instanceof D) {
 *     // Here we know Tu <: arraykey but the constraint is checked later
 *     foo($y);
 *   }
 *)
 let add_todo env f =
  let tpenv_now = env.lenv.tpenv in
  let f' env =
    let old_tpenv = env.lenv.tpenv in
    let env, remove = f (env_with_tpenv env tpenv_now) in
    env_with_tpenv env old_tpenv, remove in
  { env with todo = f' :: env.todo }

let add_anonymous env x =
  let genv = env.genv in
  let anon_id = Ident.tmp() in
  let genv = { genv with anons = IMap.add anon_id x genv.anons } in
  { env with genv = genv }, anon_id

let set_anonymous env anon_id x =
  let genv = env.genv in
  let genv = { genv with anons = IMap.add anon_id x genv.anons } in
  { env with genv = genv }

let get_anonymous env x =
  IMap.get x env.genv.anons

let set_self_id env x =
  let genv = env.genv in
  let genv = { genv with self_id = x } in
  { env with genv = genv }

let set_self env x =
  let genv = env.genv in
  let genv = { genv with self = x } in
  { env with genv = genv }

let set_parent_id env x =
  let genv = env.genv in
  let genv = { genv with parent_id = x } in
  { env with genv = genv }

let set_parent env x =
  let genv = env.genv in
  let genv = { genv with parent = x } in
  { env with genv = genv }

let set_static env =
  let genv = env.genv in
  let genv = { genv with static = true } in
  { env with genv = genv }

let set_mode env mode =
  let decl_env = env.decl_env in
  let decl_env = { decl_env with mode } in
  { env with decl_env }

let get_mode env = env.decl_env.mode

let is_strict env = get_mode env = FileInfo.Mstrict
let is_decl env = get_mode env = FileInfo.Mdecl

let get_options env = env.genv.tcopt

(*
let debug_env env =
  Classes.iter begin fun cid class_ ->
    Printf.printf "Type of class %s:" cid;
    Printf.printf "{ ";
    SMap.iter begin fun m _ ->
      Printf.printf "%s " m;
    end class_.tc_methods;
    Printf.printf "}\n"
  end env.genv.classes
*)
(*****************************************************************************)
(* This is used when we want member variables to be treated like locals
 * We want to handle the following:
 * if($this->x) {
 *   ... $this->x ...
 * }
 * The trick consists in replacing $this->x with a "fake" local. So that
 * all the logic that normally applies to locals is applied in cases like
 * this. Hence the name: FakeMembers.
 * All the fake members are thrown away at the first call.
 * We keep the invalidated fake members for better error messages.
 *)
(*****************************************************************************)

let get_last_call env =
  match (env.lenv.fake_members).last_call with
  | None -> assert false
  | Some pos -> pos

let rec lost_info fake_name env ty =
  let info r = Reason.Rlost_info (fake_name, r, get_last_call env) in
  match ty with
  | _, Tvar v ->
      let env, v' = get_var env v in
      (match IMap.get v' env.tenv with
      | None ->
          env, ty
      | Some ty ->
          let env, ty = lost_info fake_name env ty in
          let env = add env v ty in
          env, ty
      )
  | r, Tunresolved tyl ->
      let env, tyl = List.map_env env tyl (lost_info fake_name) in
      env, (info r, Tunresolved tyl)
  | r, ty ->
      env, (info r, ty)

let forget_members env call_pos =
  let {fake_members; local_types; local_type_history; tpenv} = env.lenv in
  let old_invalid = fake_members.invalid in
  let new_invalid = fake_members.valid in
  let new_invalid = SSet.union new_invalid old_invalid in
  let fake_members = {
    last_call = Some call_pos;
    invalid = new_invalid;
    valid = SSet.empty;
  } in
  { env with lenv = {fake_members; local_types; local_type_history; tpenv } }

module FakeMembers = struct

  let make_id obj_name member_name =
    let obj_name =
      match obj_name with
      | _, This -> this
      | _, Lvar (_, x) -> x
      | _ -> assert false
    in
    Local_id.to_string obj_name^"->"^member_name

  let make_static_id cid member_name =
    let class_name = class_id_to_str cid in
    class_name^"::"^member_name

  let get env obj member_name =
    match obj with
    | _, This
    | _, Lvar _ ->
        let id = make_id obj member_name in
        if SSet.mem id env.lenv.fake_members.valid
        then Some (Hashtbl.hash id)
        else None
    | _ -> None

  let is_invalid env obj member_name =
    match obj with
    | _, This
    | _, Lvar _ ->
        SSet.mem (make_id obj member_name) env.lenv.fake_members.invalid
    | _ -> false

  let get_static env cid member_name =
    let name = make_static_id cid member_name in
    if SSet.mem name env.lenv.fake_members.valid
    then Some (Hashtbl.hash name)
    else None

  let is_static_invalid env cid member_name =
    SSet.mem (make_static_id cid member_name) env.lenv.fake_members.invalid

  let add_member env fake_id =
    let {fake_members; local_types; local_type_history; tpenv} = env.lenv in
    let valid = SSet.add fake_id fake_members.valid in
    let fake_members = { fake_members with valid = valid } in
    { env with lenv = {fake_members; local_types; local_type_history; tpenv } }

  let make _ env obj_name member_name =
    let my_fake_local_id = make_id obj_name member_name in
    let env = add_member env my_fake_local_id in
    env, Local_id.get my_fake_local_id

 let make_static _ env class_name member_name =
   let my_fake_local_id = make_static_id class_name member_name in
   let env = add_member env my_fake_local_id in
   env, Local_id.get my_fake_local_id

end


(*****************************************************************************)
(* Locals *)
(*****************************************************************************)

(* We want to "take a picture" of the current type
 * that is, the current type shouldn't be affected by a
 * future unification.
 *)

let rec unbind seen env ty =
  let env, ty = expand_type env ty in
  if List.exists seen (fun ty' ->
    let _, ty' = expand_type env ty' in Typing_defs.ty_equal ty ty')
  then env, ty
  else
    let seen = ty :: seen in
    match ty with
    | r, Tunresolved tyl ->
        let env, tyl = List.map_env env tyl (unbind seen) in
        env, (r, Tunresolved tyl)
    | ty -> env, ty

let unbind = unbind []

(* We maintain 3 states for a local, all the types that the
 * local ever had (cf integrate in typing.ml), the type
 * that the local currently has, and an expression_id generated from
 * the last assignment to this local.
 *)
let set_local env x new_type =
  let {fake_members; local_types ; local_type_history; tpenv} = env.lenv in
  let env, new_type = unbind env new_type in
  let next_cont = LEnvC.get_cont Cont.Next local_types in
  let all_types, expr_id =
    match
      (Local_id.Map.get x next_cont, Local_id.Map.get x local_type_history)
      with
    | None, None -> [], Ident.tmp()
    | Some (_, y), Some x -> x, y
    | _ -> Exit_status.(exit Local_type_env_stale)
  in
  let all_types =
    if List.exists all_types (fun ty' ->
      let _, ty' = expand_type env ty' in Typing_defs.ty_equal new_type ty')
    then all_types
    else new_type :: all_types
  in
  let local = new_type, expr_id in
  let local_types = LEnvC.add_to_cont Cont.Next x local local_types in
  let local_type_history = Local_id.Map.add x all_types local_type_history in
  let env = { env with
    lenv = {fake_members; local_types; local_type_history; tpenv} }
  in
  env

let get_local env x =
  let next_cont = LEnvC.get_cont Cont.Next env.lenv.local_types in
  let lcl = Local_id.Map.get x next_cont in
  match lcl with
  | None -> env, (Reason.Rnone, Tany)
  | Some (x, _) -> env, x

let set_local_expr_id env x new_eid =
  let {fake_members; local_types; local_type_history; tpenv} = env.lenv in
  let next_cont = LEnvC.get_cont Cont.Next local_types in
  match Local_id.Map.get x next_cont with
  | Some (type_, eid) when eid <> new_eid ->
      let local = type_, new_eid in
      let local_types = LEnvC.add_to_cont Cont.Next x local local_types in
      let env ={ env with
        lenv = {fake_members; local_types; local_type_history; tpenv} }
      in
      env
  | _ -> env

let get_local_expr_id env x =
  let next_cont = LEnvC.get_cont Cont.Next env.lenv.local_types in
  let lcl = Local_id.Map.get x next_cont in
  Option.map lcl ~f:(fun (_, x) -> x)

(*****************************************************************************)
(* This function is called when we are about to type-check a block that will
 * later be fully_integrated (cf Typing.fully_integrate).
 * Integration is about keeping track of all the types that a local had in
 * its lifetime. It's necessary to correctly type-check catch blocks.
 * After we type-check a block, we want to take all the types that the local
 * had in this block, and add it to the list of possible types.
 *
 * However, we are not interested in the types that the local had *before*
 * we started typing the block.
 *
 * A concrete example:
 *
 * $x = null;
 *
 * $x = 'hello'; // the type of $x is string
 *
 * while (...) {
 *   $x = 0;
 * }
 *
 * The type of $x is string or int, NOT string or int or ?_.
 * We don't really care about the fact that $x could be null before the
 * block.
 *
 * This is what freeze_local does, just before we start type-checking the
 * while loop, we "freeze" the type of locals to the current environment.
 *)
(*****************************************************************************)

let freeze_local_env env =
  let local_types = env.lenv.local_types in
  let next_cont = LEnvC.get_cont Cont.Next local_types in
  let local_type_history = Local_id.Map.map
    (fun (type_, _) -> [type_])
    next_cont
  in
  env_with_locals env local_types local_type_history

(*****************************************************************************)
(* Sets up/cleans up the environment when typing an anonymous function. *)
(*****************************************************************************)

let anon anon_lenv env f =
  (* Setting up the environment. *)
  let old_lenv = env.lenv in
  let old_return = get_return env in
  let outer_fun_kind = get_fn_kind env in
  let env = { env with lenv = anon_lenv } in
  (* Typing *)
  let env, tfun, result = f env in
  (* Cleaning up the environment. *)
  let env = { env with lenv = old_lenv } in
  let env = set_return env old_return in
  let env = set_fn_kind env outer_fun_kind in
  env, tfun, result

let in_loop env f =
  let old_in_loop = env.in_loop in
  let env = { env with in_loop = true } in
  let env, result = f env in
  { env with in_loop = old_in_loop }, result

(*****************************************************************************)
(* Merge and un-merge locals *)
(*****************************************************************************)

let merge_locals_and_history lenv =
  let merge_fn _key locals history =
    match locals, history with
      | None, None -> None
      | Some (type_, exp_id), Some hist -> Some (hist, type_, exp_id)
      | _ -> Exit_status.(exit Local_type_env_stale)
  in
  let next_cont = LEnvC.get_cont Cont.Next lenv.local_types in
  Local_id.Map.merge
    merge_fn next_cont lenv.local_type_history

(* TODO: Right now the only continuation we have is next
 * so I'm putting everything in next *)
let separate_locals_and_history locals_and_history =
  let conts = Typing_continuations.Map.empty in
  let next_cont = Local_id.Map.map
    (fun (_, type_, exp_id) -> type_, exp_id) locals_and_history
  in
  let locals =
    Typing_continuations.Map.add
      Cont.Next
      next_cont
      conts
  in
  let history = Local_id.Map.map
    (fun (hist, _, _) -> hist) locals_and_history
  in
  locals, history
