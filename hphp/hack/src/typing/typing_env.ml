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
open Hh_core
open Decl_env
open Typing_defs
open Nast

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

let make_ft p reactivity is_coroutine params ret_ty =
  let arity = List.length params in
  {
    ft_pos      = p;
    ft_deprecated = None;
    ft_abstract = false;
    ft_is_coroutine = is_coroutine;
    ft_arity    = Fstandard (arity, arity);
    ft_tparams  = [];
    ft_where_constraints = [];
    ft_params   = params;
    ft_ret      = ret_ty;
    ft_ret_by_ref = false;
    ft_reactive = reactivity;
    ft_return_disposable = false;
    ft_returns_mutable = false;
    ft_mutable = false;
  }

let get_shape_field_name = function
  | Ast.SFlit (_, s) -> s
  | Ast.SFclass_const ((_, s1), (_, s2)) -> s1^"::"^s2

let empty_bounds = TySet.empty
let singleton_bound ty = TySet.singleton ty

let get_tpenv_lower_bounds tpenv name =
match SMap.get name tpenv with
| None -> empty_bounds
| Some {lower_bounds; _} -> lower_bounds

let get_tpenv_upper_bounds tpenv name =
match SMap.get name tpenv with
| None -> empty_bounds
| Some {upper_bounds; _} -> upper_bounds

let get_lower_bounds env name =
  let local = get_tpenv_lower_bounds env.lenv.tpenv name in
  let global = get_tpenv_lower_bounds env.global_tpenv name in
  TySet.union local global

let get_upper_bounds env name =
  let local = get_tpenv_upper_bounds env.lenv.tpenv name in
  let global = get_tpenv_upper_bounds env.global_tpenv name in
  TySet.union local global

(* Get bounds that are both an upper and lower of a given generic *)
let get_equal_bounds env name =
  let lower = get_lower_bounds env name in
  let upper = get_upper_bounds env name in
  TySet.inter lower upper

let is_generic_param ty name =
  match ty with
  | (_, Tabstract (AKgeneric name', None)) -> name = name'
  | _ -> false

(* Add a single new upper bound [ty] to generic parameter [name] in [tpenv] *)
let add_upper_bound_ tpenv name ty =
  if is_generic_param ty name
  then tpenv
  else match SMap.get name tpenv with
  | None ->
    SMap.add name
      {lower_bounds = empty_bounds; upper_bounds = singleton_bound ty} tpenv
  | Some {lower_bounds; upper_bounds} ->
    SMap.add name
      {lower_bounds; upper_bounds = ty++upper_bounds} tpenv

(* Add a single new lower bound [ty] to generic parameter [name] in [tpenv] *)
let add_lower_bound_ tpenv name ty =
  if is_generic_param ty name
  then tpenv
  else
  match SMap.get name tpenv with
  | None ->
    SMap.add name
      {lower_bounds = singleton_bound ty; upper_bounds = empty_bounds} tpenv
  | Some {lower_bounds; upper_bounds} ->
    SMap.add name
      {lower_bounds = ty++lower_bounds; upper_bounds} tpenv

let env_with_tpenv env tpenv =
  { env with lenv = { env.lenv with tpenv = tpenv } }

let env_with_global_tpenv env global_tpenv =
  { env with global_tpenv }

let add_upper_bound_global env name ty =
  let tpenv =
    begin match ty with
    | (r, Tabstract (AKgeneric formal_super, _)) ->
      add_lower_bound_ env.global_tpenv formal_super
        (r, Tabstract (AKgeneric name, None))
    | _ -> env.global_tpenv
    end in
   { env with global_tpenv=(add_upper_bound_ tpenv name ty) }

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

let tparams_visitor env =
  object(this)
    inherit [SSet.t] Type_visitor.type_visitor
    method! on_tabstract acc _ ak _ty_opt =
      match ak with
      | AKgeneric s -> SSet.add s acc
      | _ -> acc
    method! on_tvar acc r ix =
      let _env, ty = get_type env r ix in
      this#on_type acc ty
  end
let get_tparams_aux env acc ty = (tparams_visitor env)#on_type acc ty
let get_tparams env ty = get_tparams_aux env SSet.empty ty

let get_tpenv_tparams env =
  SMap.fold begin fun _x { lower_bounds; upper_bounds } acc ->
    let folder ty acc =
      match ty with
      | _, Tabstract (AKgeneric _, _) -> acc
      | _ -> get_tparams_aux env acc ty in
    TySet.fold folder lower_bounds @@
    TySet.fold folder upper_bounds acc
    end
  env.lenv.tpenv SSet.empty

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

let empty_local tpenv local_reactive = {
  tpenv = tpenv;
  fake_members = empty_fake_members;
  local_types = Typing_continuations.Map.empty;
  local_using_vars = Local_id.Set.empty;
  local_type_history = Local_id.Map.empty;
  local_mutability = Local_id.Map.empty;
  local_reactive = local_reactive;
}

let empty tcopt file ~droot = {
  function_pos = Pos.none;
  pos     = Pos.none;
  outer_pos = Pos.none;
  outer_reason = Reason.URnone;
  tenv    = IMap.empty;
  subst   = IMap.empty;
  lenv    = empty_local SMap.empty Nonreactive;
  todo    = [];
  in_loop = false;
  inside_constructor = false;
  decl_env = {
    mode = FileInfo.Mstrict;
    droot;
    decl_tcopt = tcopt;
  };
  genv    = {
    tcopt   = tcopt;
    return  = {
      return_type = fresh_type ();
      return_explicit = false;
      return_disposable = false;
      return_mutable = false };
    params  = Local_id.Map.empty;
    self_id = "";
    self    = Reason.none, Tany;
    static  = false;
    parent_id = "";
    parent  = Reason.none, Tany;
    fun_kind = Ast.FSync;
    fun_mutable = false;
    anons   = IMap.empty;
    file    = file;
  };
  global_tpenv = SMap.empty
}

let set_env_reactive env reactive =
  { env with lenv = {env.lenv with local_reactive = reactive }}

let set_env_function_pos env function_pos =
  { env with function_pos }

let lambda_reactive = ref None

let env_reactivity env =
  Option.value !lambda_reactive ~default:env.lenv.local_reactive

(* Some form (strict/shallow/local) of reactivity *)
let env_local_reactive env =
  env_reactivity env <> Nonreactive

let function_is_mutable env =
  env.genv.fun_mutable

let set_fun_mutable env mut =
  { env with genv = {env.genv with fun_mutable = mut }}

(* Takes in the typechecking function of a lambda
  block and checks if it breaks reactivity rules *)
let check_lambda_reactive f =
  let old_lambda_reactive = !lambda_reactive in
  lambda_reactive := Some Reactive;
  let results = f () in
  let result = !lambda_reactive in
  lambda_reactive := old_lambda_reactive;
  match result with
  | Some c -> c, results
  | None -> assert false

let not_lambda_reactive () =
  lambda_reactive := (match !lambda_reactive with
  | Some _ -> Some Nonreactive
  | None -> None)

let is_checking_lambda () =
  Option.is_some !lambda_reactive

let error_if_reactive_context env f =
  not_lambda_reactive ();
  if env_local_reactive env then f ()

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


let env_with_mut env local_mutability =
  { env with lenv = { env.lenv with local_mutability } }

let get_env_mutability env =
  env.lenv.local_mutability

(* When we want to type something with a fresh typing environment *)
let fresh_tenv env f =
  f { env with
      todo = [];
      lenv = empty_local env.lenv.tpenv env.lenv.local_reactive;
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
    SMap.add (String.lowercase_ascii x) (pos, x) acc
  end members SMap.empty
  in
  SMap.get mid members

let suggest_static_member is_method class_ mid =
  let mid = String.lowercase_ascii mid in
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
  let mid = String.lowercase_ascii mid in
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

let get_params env =
  env.genv.params

let set_params env params =
  { env with genv = { env.genv with params = params } }

let set_param env x param =
  let params = get_params env in
  let params = Local_id.Map.add x param params in
  set_params env params

let clear_params env =
  set_params env Local_id.Map.empty

let with_env env f =
  let ret = get_return env in
  let params = get_params env in
  let env, result = f env in
  let env = set_params env params in
  let env = set_return env ret in
  env, result

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

let log_anonymous env =
  if TypecheckerOptions.disallow_ambiguous_lambda (get_options env)
  then IMap.iter (fun _ (_, _, counter, pos, _) ->
    Errors.ambiguous_lambda pos !counter) env.genv.anons

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
  let fake_members = env.lenv.fake_members in
  let old_invalid = fake_members.invalid in
  let new_invalid = fake_members.valid in
  let new_invalid = SSet.union new_invalid old_invalid in
  let fake_members = {
    last_call = Some call_pos;
    invalid = new_invalid;
    valid = SSet.empty;
  } in
  { env with lenv = { env.lenv with fake_members } }

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
    let fake_members = env.lenv.fake_members in
    let valid = SSet.add fake_id fake_members.valid in
    let fake_members = { fake_members with valid = valid } in
    { env with lenv = { env.lenv with fake_members } }

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
  let {fake_members; local_types; local_type_history; local_using_vars;
       tpenv; local_mutability; local_reactive} = env.lenv in
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
    lenv = {fake_members; local_types; local_type_history; local_using_vars;
            tpenv; local_mutability; local_reactive; } }
  in
  env

let is_using_var env x =
  Local_id.Set.mem x env.lenv.local_using_vars

let set_using_var env x =
  { env with lenv = {
    env.lenv with local_using_vars = Local_id.Set.add x env.lenv.local_using_vars } }

let unset_local env local =
  let {fake_members; local_types ; local_type_history;
       local_using_vars; tpenv; local_mutability;local_reactive; } = env.lenv in
  let local_types = LEnvC.remove_from_cont Cont.Next local local_types in
  let local_using_vars = Local_id.Set.remove local local_using_vars in
  let local_type_history = Local_id.Map.remove local local_type_history in
  let local_mutability = Local_id.Map.remove local local_mutability in
  let env = { env with
    lenv = {fake_members; local_types; local_type_history; local_using_vars;
            tpenv; local_mutability; local_reactive} }
  in
  env


let is_mutable env local =
  Local_id.Map.mem local env.lenv.local_mutability

let add_mutable_var env local mutability_type =
env_with_mut
  env
  (Local_id.Map.add local mutability_type env.lenv.local_mutability)

let get_locals env =
  LEnvC.get_cont Cont.Next env.lenv.local_types

let get_local env x =
  let next_cont = get_locals env in
  let lcl = Local_id.Map.get x next_cont in
  match lcl with
  | None -> (Reason.Rnone, Tany)
  | Some (x, _) -> x

let set_local_expr_id env x new_eid =
  let local_types = env.lenv.local_types in
  let next_cont = LEnvC.get_cont Cont.Next local_types in
  match Local_id.Map.get x next_cont with
  | Some (type_, eid) when eid <> new_eid ->
      let local = type_, new_eid in
      let local_types = LEnvC.add_to_cont Cont.Next x local local_types in
      let env ={ env with lenv = { env.lenv with local_types } }
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
  let old_params = get_params env in
  let outer_fun_kind = get_fn_kind env in
  let env = { env with lenv = anon_lenv } in
  (* Typing *)
  let env, tfun, result = f env in
  (* Cleaning up the environment. *)
  let env = { env with lenv = old_lenv } in
  let env = set_params env old_params in
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
