(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Typing_env_types
open Hh_core
open Decl_env
open Typing_defs
open Nast
open Typing_env_return_info
open Type_parameter_env

module Dep = Typing_deps.Dep
module TLazyHeap = Typing_lazy_heap
module LEnvC = Typing_lenv_cont
module C = Typing_continuations

let show_env _ = "<env>"
let pp_env _ _ = Printf.printf "%s\n" "<env>"

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
  match ty with
  | _, Tvar x' -> add_subst env x x'
  | _ -> { env with tenv = IMap.add x ty env.tenv }

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
    ft_mutability = None;
    ft_decl_errors = None;
    ft_returns_void_to_rx = false;
  }

let get_shape_field_name = function
  | Ast.SFlit_int (_, s)
  | Ast.SFlit_str (_, s) -> s
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

let rec is_generic_param ~elide_nullable ty name =
  match ty with
  | (_, Tabstract (AKgeneric name', None)) -> name = name'
  | (_, Toption ty) when elide_nullable -> is_generic_param ~elide_nullable ty name
  | _ -> false

(* Add a single new upper bound [ty] to generic parameter [name] in [tpenv] *)
let add_upper_bound_ tpenv name ty =
  (* Don't add superfluous T <: T or T <: ?T to environment *)
  if is_generic_param ~elide_nullable:true ty name
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
  (* Don't add superfluous T <: T to environment *)
  if is_generic_param ~elide_nullable:false ty name
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

 (* Add a single new upper bound [ty] to generic parameter [name] in the local
  * type parameter environment of [env].
  * If the optional [intersect] operation is supplied, then use this to avoid
  * adding redundant bounds by merging the type with existing bounds. This makes
  * sense because a conjunction of upper bounds
  *   (T <: t1) /\ ... /\ (T <: tn)
  * is equivalent to a single upper bound
  *   T <: (t1 & ... & tn)
  *)
 let add_upper_bound ?intersect env name ty =
   let tpenv =
     begin match ty with
     | (r, Tabstract (AKgeneric formal_super, _)) ->
       add_lower_bound_ env.lenv.tpenv formal_super
         (r, Tabstract (AKgeneric name, None))
     | _ -> env.lenv.tpenv
     end in
   match intersect with
   | None -> env_with_tpenv env (add_upper_bound_ tpenv name ty)
   | Some intersect ->
     let tyl = intersect ty (TySet.elements (get_upper_bounds env name)) in
     let add ty tys =
       if is_generic_param ~elide_nullable:true ty name
       then tys else TySet.add ty tys in
     let upper_bounds = List.fold_right ~init:TySet.empty ~f:add tyl in
     let lower_bounds = get_tpenv_lower_bounds env.lenv.tpenv name in
     env_with_tpenv env (SMap.add name {lower_bounds; upper_bounds} tpenv)

(* Add a single new upper lower [ty] to generic parameter [name] in the
 * local type parameter environment [env].
 * If the optional [union] operation is supplied, then use this to avoid
 * adding redundant bounds by merging the type with existing bounds. This makes
 * sense because a conjunction of lower bounds
 *   (t1 <: T) /\ ... /\ (tn <: T)
 * is equivalent to a single lower bound
 *   (t1 | ... | tn) <: T
 *)
let add_lower_bound ?union env name ty =
  let tpenv =
    begin match ty with
    | (r, Tabstract (AKgeneric formal_sub, _)) ->
      add_upper_bound_ env.lenv.tpenv formal_sub
        (r, Tabstract (AKgeneric name, None))
    | _ -> env.lenv.tpenv
    end in
  match union with
  | None -> env_with_tpenv env (add_lower_bound_ tpenv name ty)
  | Some union ->
    let tyl = union ty (TySet.elements (get_lower_bounds env name)) in
    let lower_bounds = List.fold_right ~init:TySet.empty ~f:TySet.add tyl in
    let upper_bounds = get_tpenv_upper_bounds env.lenv.tpenv name in
    env_with_tpenv env (SMap.add name {lower_bounds; upper_bounds} tpenv)

(* Add type parameters to environment, initially with no bounds.
 * Existing type parameters with the same name will be overridden. *)
let add_generic_parameters env tparaml =
  let add_empty_bounds tpenv (_, (_, name), _, _) =
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

let is_fresh_generic_parameter name =
  String.contains name '#' && not (AbstractKind.is_generic_dep_ty name)

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
let env_with_locals env locals =
  { env with lenv =
    { env.lenv with local_types = locals; }
  }

let reinitialize_locals env =
  env_with_locals env LEnvC.initial_locals

let empty_fake_members = {
  last_call = None;
  invalid   = SSet.empty;
  valid     = SSet.empty;
}

let empty_local tpenv local_reactive = {
  tpenv = tpenv;
  fake_members = empty_fake_members;
  local_types = LEnvC.empty_locals;
  local_using_vars = Local_id.Set.empty;
  local_mutability = Local_id.Map.empty;
  local_reactive = local_reactive;
}

let initial_local tpenv local_reactive = {
  tpenv = tpenv;
  fake_members = empty_fake_members;
  local_types = LEnvC.initial_locals;
  local_using_vars = Local_id.Set.empty;
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
  lenv    = initial_local SMap.empty Nonreactive;
  todo    = [];
  checking_todos = false;
  in_loop = false;
  in_try  = false;
  in_case  = false;
  inside_constructor = false;
  inside_ppl_class = false;
  decl_env = {
    mode = FileInfo.Mstrict;
    droot;
    decl_tcopt = tcopt;
  };
  genv    = {
    tcopt   = tcopt;
    return  = {
      return_type = fresh_type ();
      return_disposable = false;
      return_mutable = false;
      return_explicit = false;
      return_by_ref = false;
      return_void_to_rx = false;
    };
    params  = Local_id.Map.empty;
    condition_types = SMap.empty;
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

let set_condition_type env n ty =
  { env with genv = {
    env.genv with condition_types = SMap.add n ty env.genv.condition_types }
  }

let get_condition_type env n =
  SMap.get n env.genv.condition_types

let env_reactivity env =
  env.lenv.local_reactive

(* Some form (strict/shallow/local) of reactivity *)
let env_local_reactive env =
  env_reactivity env <> Nonreactive

let function_is_mutable env =
  env.genv.fun_mutable

let set_fun_mutable env mut =
  { env with genv = {env.genv with fun_mutable = mut }}

let error_if_reactive_context env f =
  if env_local_reactive env && not (TypecheckerOptions.unsafe_rx env.genv.tcopt) then f ()

let error_if_shallow_reactive_context env f =
  match env_reactivity env with
  | Reactive _ | Shallow _ when not (TypecheckerOptions.unsafe_rx env.genv.tcopt) -> f ()
  | _ -> ()

let forward_compat_ge env min =
  let fcl = TypecheckerOptions.forward_compatibility_level (get_tcopt env) in
  ForwardCompatibilityLevel.as_int fcl >= min

let error_if_forward_compat_ge env min f =
  if forward_compat_ge env min
  then f ()
  else ()

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
      lenv = initial_local env.lenv.tpenv env.lenv.local_reactive;
      tenv = IMap.empty;
      in_loop = false;
      in_try = false;
      in_case = false;
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
  let env = { env with checking_todos = true } in
  let env, remaining =
    List.fold_left env.todo ~f:(fun (env, remaining) f ->
      let env, remove = f env in
      if remove then env, remaining else env, f::remaining)
      ~init:(env, []) in
  { env with todo = List.rev remaining; checking_todos = false }

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

let set_inside_ppl_class env inside_ppl_class =
  { env with inside_ppl_class }

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

let is_strict env = let mode = get_mode env in
                    mode = FileInfo.Mstrict || mode = FileInfo.Mexperimental
let is_decl env = get_mode env = FileInfo.Mdecl

let get_options env = env.genv.tcopt

let iter_anonymous env f =
  IMap.iter (fun _id (_, _, ftys, pos, _) ->
    let (untyped,typed) = !ftys in f pos (untyped @ typed)) env.genv.anons

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

(* We maintain 2 states for a local: the type
 * that the local currently has, and an expression_id generated from
 * the last assignment to this local.
 *)
let set_local env x new_type =
  let {fake_members; local_types; local_using_vars;
       tpenv; local_mutability; local_reactive} = env.lenv in
  let env, new_type = unbind env new_type in
  let new_type = match new_type with
    | _, Tunresolved [ty] -> ty
    | _ -> new_type in
  let next_cont = LEnvC.get_cont C.Next local_types in
  let expr_id = match Local_id.Map.get x next_cont with
    | None -> Ident.tmp()
    | Some (_, y) -> y in
  let local = new_type, expr_id in
  let local_types = LEnvC.add_to_cont C.Next x local local_types in
  let env = { env with
    lenv = {fake_members; local_types; local_using_vars;
            tpenv; local_mutability; local_reactive; } }
  in
  env

let is_using_var env x =
  Local_id.Set.mem x env.lenv.local_using_vars

let set_using_var env x =
  { env with lenv = {
    env.lenv with local_using_vars = Local_id.Set.add x env.lenv.local_using_vars } }

let unset_local env local =
  let {fake_members; local_types; local_using_vars; tpenv; local_mutability;
    local_reactive; } = env.lenv in
  let local_types = LEnvC.remove_from_cont C.Next local local_types in
  let local_using_vars = Local_id.Set.remove local local_using_vars in
  let local_mutability = Local_id.Map.remove local local_mutability in
  let env = { env with
    lenv = {fake_members; local_types; local_using_vars;
            tpenv; local_mutability; local_reactive} }
  in
  env


let is_mutable env local =
  let module TME = Typing_mutability_env in
  match Local_id.Map.get local env.lenv.local_mutability with
  | Some (_, (TME.Mutable | TME.Borrowed)) -> true
  | _ -> false

let add_mutable_var env local mutability_type =
env_with_mut
  env
  (Local_id.Map.add local mutability_type env.lenv.local_mutability)

let get_locals env =
  LEnvC.get_cont C.Next env.lenv.local_types

let get_local_in_ctx x ctx =
  let lcl = Local_id.Map.get x ctx in
  match lcl with
  | None -> (Reason.Rnone, Tany)
  | Some (x, _) -> x

let get_local_ env x =
  let next_cont = get_locals env in
  get_local_in_ctx x next_cont

(* While checking todos at the end of a function body, the Next continuation
 * might have been moved to the 'Exit' (if there is a `return` statement)
 * or the 'Catch' continuation (if the function always throws). So we find
 * which continuation is still present and get the local from there. *)
let get_local_for_todo env x =
  let local_types = env.lenv.local_types in
  let ctx = LEnvC.try_get_conts [C.Next; C.Exit; C.Catch] local_types in
  get_local_in_ctx x ctx

let get_local env x =
  if env.checking_todos then get_local_for_todo env x else get_local_ env x

let set_local_expr_id env x new_eid =
  let local_types = env.lenv.local_types in
  let next_cont = LEnvC.get_cont C.Next local_types in
  match Local_id.Map.get x next_cont with
  | Some (type_, eid) when eid <> new_eid ->
      let local = type_, new_eid in
      let local_types = LEnvC.add_to_cont C.Next x local local_types in
      let env ={ env with lenv = { env.lenv with local_types } }
      in
      env
  | _ -> env

let get_local_expr_id env x =
  let next_cont = LEnvC.get_cont C.Next env.lenv.local_types in
  let lcl = Local_id.Map.get x next_cont in
  Option.map lcl ~f:(fun (_, x) -> x)

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

let in_try env f =
  let old_in_try = env.in_try in
  let env = { env with in_try = true } in
  let env, result = f env in
  { env with in_try = old_in_try }, result

let in_case env f =
  let old_in_case = env.in_case in
  let env = { env with in_case = true } in
  let env, result = f env in
  { env with in_case = old_in_case }, result

(* Return the subset of env which is saved in the Typed AST's EnvAnnotation. *)
let save local_tpenv env =
  {
    Tast.tcopt = get_tcopt env;
    Tast.tenv = env.tenv;
    Tast.subst = env.subst;
    Tast.tpenv = SMap.union local_tpenv env.global_tpenv;
  }
