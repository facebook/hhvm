(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Common
include Typing_env_types
open Decl_env
open Typing_defs
open Nast
open Typing_env_return_info
open Type_parameter_env

module Dep = Typing_deps.Dep
module LID = Local_id
module TLazyHeap = Typing_lazy_heap
module SG = SN.Superglobals
module LEnvC = Typing_lenv_cont
module C = Typing_continuations
module TL = Typing_logic
module Cls = Typing_classes_heap

let show_env _ = "<env>"
let pp_env _ _ = Printf.printf "%s\n" "<env>"

let ( ++ ) x y = Typing_set.add x y

let get_tcopt env = env.genv.tcopt

let set_log_level env key log_level =
  {env with log_levels = SMap.add key log_level env.log_levels }

let get_log_level env key =
  Option.value (SMap.get key env.log_levels) ~default:0

let env_log_function = ref (fun _pos _name _old_env _new_env -> ())

let set_env_log_function f = env_log_function := f

let log_env_change name old_env new_env =
  if get_log_level new_env name >= 1 || get_log_level new_env "env" >= 1
  then begin
    let pos =
      match old_env.tyvars_stack with
      | (p,_)::_ -> p
      | _ -> old_env.pos in
    !env_log_function pos name old_env new_env
  end;
  new_env

let fresh () =
  Ident.tmp()

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

let empty_bounds = TySet.empty
let singleton_bound ty = TySet.singleton ty

let env_with_tvenv env tvenv =
  { env with tvenv = tvenv }

let empty_tyvar_info =
  { tyvar_pos = Pos.none;
    eager_solve_fail = false;
    lower_bounds = empty_bounds;
    upper_bounds = empty_bounds;
    appears_covariantly = false;
    appears_contravariantly = false;
    type_constants = SMap.empty;
    }

let add_current_tyvar env p v =
  if TypecheckerOptions.new_inference env.genv.tcopt
  then
    match env.tyvars_stack with
    | (expr_pos, tyvars) :: rest ->
      let env = env_with_tvenv env
        (IMap.add v { empty_tyvar_info with tyvar_pos = p } env.tvenv) in
      { env with tyvars_stack = (expr_pos, (v :: tyvars)) :: rest }
    | _ -> env
  else env

let fresh_unresolved_type env p =
  let v = Ident.tmp () in
  let env =
    if TypecheckerOptions.new_inference env.genv.tcopt
    then
      log_env_change "fresh_unresolved_type" env @@
      add_current_tyvar env p v
    else add env v (Reason.Rnone, Tunresolved []) in
  env, (Reason.Rtype_variable p, Tvar v)

let open_tyvars env p =
  { env with tyvars_stack = (p,[]) :: env.tyvars_stack }

let close_tyvars env =
  match env.tyvars_stack with
  | [] -> failwith "close_tyvars: empty stack"
  | _::rest -> { env with tyvars_stack = rest }

let get_current_tyvars env =
  match env.tyvars_stack with
  | [] -> []
  | (_,tyvars)::_ -> tyvars

let fresh_type env p =
  let v = Ident.tmp () in
  let env =
    log_env_change "fresh_type" env @@
    add_current_tyvar env p v in
  env, (Reason.Rtype_variable p, Tvar v)

let get_type env x_reason x =
  let env, x = get_var env x in
  let ty = IMap.get x env.tenv in
  match ty with
  | None ->
    if TypecheckerOptions.new_inference env.genv.tcopt
    then env, (x_reason, Tvar x)
    else env, (x_reason, Tany)
  | Some ty -> env, ty

let get_type_unsafe env x =
  let ty = IMap.get x env.tenv in
  match ty with
  | None ->
      env, (Reason.none, Tany)
  | Some ty -> env, ty

let get_tyvar_info env var =
  Option.value (IMap.get var env.tvenv) ~default:empty_tyvar_info

let get_tyvar_eager_solve_fail env var =
  let tvinfo = get_tyvar_info env var in
  tvinfo.eager_solve_fail

let expand_var env r v =
  let env, ty = get_type env r v in
  if get_tyvar_eager_solve_fail env v
  then env, (Reason.Rsolve_fail (Reason.to_pos r), snd ty)
  else env, ty

let expand_type env x =
  match x with
  | r, Tvar x -> expand_var env r x
  | x -> env, x

let make_ft p reactivity is_coroutine params ret_ty =
  let arity = List.length params in
  {
    ft_pos      = p;
    ft_deprecated = None;
    ft_abstract = false;
    ft_is_coroutine = is_coroutine;
    ft_arity    = Fstandard (arity, arity);
    ft_tparams  = ([], FTKtparams);
    ft_where_constraints = [];
    ft_params   = params;
    ft_ret      = ret_ty;
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

let get_shape_field_name_pos = function
  | Ast.SFlit_int (p, _)
  | Ast.SFlit_str (p, _)
  | Ast.SFclass_const ((p, _), _) -> p

let get_tpenv_lower_bounds tpenv name =
match SMap.get name tpenv with
| None -> empty_bounds
| Some {lower_bounds; _} -> lower_bounds

let get_tpenv_upper_bounds tpenv name =
match SMap.get name tpenv with
| None -> empty_bounds
| Some {upper_bounds; _} -> upper_bounds


let get_tpenv_reified tpenv name =
match SMap.get name tpenv with
| None -> Erased
| Some {reified; _} -> reified

let get_tpenv_enforceable tpenv name =
match SMap.get name tpenv with
| None -> false
| Some {enforceable; _} -> enforceable

let get_tpenv_newable tpenv name =
match SMap.get name tpenv with
| None -> false
| Some {newable; _} -> newable

let get_lower_bounds env name =
  let local = get_tpenv_lower_bounds env.lenv.tpenv name in
  let global = get_tpenv_lower_bounds env.global_tpenv name in
  TySet.union local global

let get_upper_bounds env name =
  let local = get_tpenv_upper_bounds env.lenv.tpenv name in
  let global = get_tpenv_upper_bounds env.global_tpenv name in
  TySet.union local global

let get_reified env name =
  let local = get_tpenv_reified env.lenv.tpenv name in
  let global = get_tpenv_reified env.global_tpenv name in
  match local, global with
  | Reified, _ | _, Reified -> Reified
  | SoftReified, _ | _, SoftReified -> SoftReified
  | _ -> Erased

let get_enforceable env name =
  let local = get_tpenv_enforceable env.lenv.tpenv name in
  let global = get_tpenv_enforceable env.global_tpenv name in
  local || global

let get_newable env name =
  let local = get_tpenv_newable env.lenv.tpenv name in
  let global = get_tpenv_newable env.global_tpenv name in
  local || global

(* Get bounds that are both an upper and lower of a given generic *)
let get_equal_bounds env name =
  let lower = get_lower_bounds env name in
  let upper = get_upper_bounds env name in
  TySet.inter lower upper

let get_tpenv_equal_bounds env name =
  let lower = get_tpenv_lower_bounds env name in
  let upper = get_tpenv_upper_bounds env name in
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
      { lower_bounds = empty_bounds;
        upper_bounds = singleton_bound ty;
        reified = Erased;
        enforceable = false;
        newable = false } tpenv
  | Some tp ->
    SMap.add name { tp with upper_bounds = ty++tp.upper_bounds; } tpenv

(* Add a single new lower bound [ty] to generic parameter [name] in [tpenv] *)
let add_lower_bound_ tpenv name ty =
  (* Don't add superfluous T <: T to environment *)
  if is_generic_param ~elide_nullable:false ty name
  then tpenv
  else
  match SMap.get name tpenv with
  | None ->
    SMap.add name
      { lower_bounds = singleton_bound ty;
        upper_bounds = empty_bounds;
        reified = Erased;
        enforceable = false;
        newable = false } tpenv
  | Some tp ->
    SMap.add name { tp with lower_bounds = ty++tp.lower_bounds } tpenv

let env_with_tpenv env tpenv =
  { env with lenv = { env.lenv with tpenv = tpenv } }

let env_with_global_tpenv env global_tpenv =
  { env with global_tpenv }

let remove_upper_bound tpenv tparam bound =
  match SMap.get tparam tpenv with
  | None -> tpenv
  | Some tparam_info ->
    let bounds = tparam_info.upper_bounds in
    let bounds = TySet.remove bound bounds in
    let tparam_info = { tparam_info with upper_bounds = bounds } in
    SMap.add tparam tparam_info tpenv

let remove_lower_bound tpenv tparam bound =
  match SMap.get tparam tpenv with
  | None -> tpenv
  | Some tparam_info ->
    let bounds = tparam_info.lower_bounds in
    let bounds = TySet.remove bound bounds in
    let tparam_info = { tparam_info with lower_bounds = bounds } in
    SMap.add tparam tparam_info tpenv

let remove_from_tpenv tpenv tparam =
  let tparam_ty = (Reason.Rnone, Tabstract (AKgeneric tparam, None)) in
  let lower_bounds = get_tpenv_lower_bounds tpenv tparam in
  let remove_tparam_from_upper_bounds_of tparam tpenv =
    match tparam with
    | _, Tabstract (AKgeneric tparam, _) ->
      remove_upper_bound tpenv tparam tparam_ty
    | _ -> tpenv in
  let tpenv = TySet.fold remove_tparam_from_upper_bounds_of lower_bounds tpenv in
  let upper_bounds = get_tpenv_upper_bounds tpenv tparam in
  let remove_tparam_from_lower_bounds_of tparam tpenv =
    match tparam with
    | _, Tabstract (AKgeneric tparam, _) ->
      remove_lower_bound tpenv tparam tparam_ty
    | _ -> tpenv in
  let tpenv = TySet.fold remove_tparam_from_lower_bounds_of upper_bounds tpenv in
  SMap.remove tparam tpenv

let simplify_tpenv env (tparams : (string * Ast.variance) list) r =
  let tpenv = env.lenv.tpenv in
  (* For each tparam, "solve" it if it falls in any of those categories:
   *   - there exists a type ty to which it is equal
   *   - it is covariant and has only one upper bound ty
   *   - it is contravariant and has only one lower bound ty.
   * In which case remove tparam from tpenv and add substitution
   * (tparam -> ty) to substs. *)
  let (tpenv, substs) = List.fold tparams ~init:(tpenv, SMap.empty)
    ~f:(fun (tpenv, substs) (tparam_name, variance) ->
      let equal_bounds = get_tpenv_equal_bounds tpenv tparam_name in
      let lower_bounds = TySet.elements (get_tpenv_lower_bounds tpenv tparam_name) in
      let upper_bounds = TySet.elements (get_tpenv_upper_bounds tpenv tparam_name) in
      match variance, lower_bounds, upper_bounds, TySet.choose_opt equal_bounds with
      (* Special case for mixed=?nonnull as a lower bound *)
      | _, [(_, Toption (_, Tnonnull)) as bound], _, _
      | _, _, _, Some bound
      | Ast.Covariant, _, [bound], _
      | Ast.Contravariant, [bound], _, _->
        (* remove tparam_name from tpenv, and in any lower/upper bound set
         * where it occurs.
         * We don't need to do any merging of lower/upper bounds,
         * because all its lower/upper bounds
         * are already lower/upper bounds of `bound` (and other bounds)
         * thanks to the transitive closure we've done in
         * Typing_subtype.add_constraint. *)
        let tpenv = remove_from_tpenv tpenv tparam_name in
        let substs = SMap.add tparam_name bound substs in
        tpenv, substs
      | _ ->
        let tparam_ty = (r, Tabstract (AKgeneric tparam_name, None)) in
        let substs = SMap.add tparam_name tparam_ty substs in
        tpenv, substs) in
  (* reduce the set of substitutions. For example, for a set of substitutions
   * like
   *   Ta -> Tb
   *   Tb -> int
   * simplify to:
   *   Ta -> int
   *   Tb -> int
   *)
  let rec reduce substs tparam =
    match SMap.find_opt tparam substs with
    | None -> substs, None
    | Some (_, Tabstract (AKgeneric tparam', _) as subst) when tparam' <> tparam ->
      let substs, new_subst_opt = reduce substs tparam' in
      begin match new_subst_opt with
      | None -> substs, Some subst
      | Some new_subst ->
        let substs = SMap.add tparam new_subst substs in
        substs, Some new_subst
      end
    | Some subst ->
      substs, Some subst in
  let reduce substs (tparam, _) = fst (reduce substs tparam) in
  let substs = List.fold tparams ~init:substs ~f:reduce in
  env_with_tpenv env tpenv, substs

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
     let reified = get_tpenv_reified env.lenv.tpenv name in
     let enforceable = get_tpenv_enforceable env.lenv.tpenv name in
     let newable = get_tpenv_newable env.lenv.tpenv name in
     env_with_tpenv env (SMap.add name {lower_bounds; upper_bounds; reified; enforceable; newable} tpenv)

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
    let reified = get_tpenv_reified env.lenv.tpenv name in
    let enforceable = get_tpenv_enforceable env.lenv.tpenv name in
    let newable = get_tpenv_newable env.lenv.tpenv name in
    env_with_tpenv env (SMap.add name {lower_bounds; upper_bounds; reified; enforceable; newable} tpenv)

(* Add type parameters to environment, initially with no bounds.
 * Existing type parameters with the same name will be overridden. *)
let add_generic_parameters env tparaml =
  let add_empty_bounds tpenv { tp_name = (_, name); tp_reified = reified; tp_user_attributes; _ } =
    let enforceable = Attributes.mem SN.UserAttributes.uaEnforceable tp_user_attributes in
    let newable = Attributes.mem SN.UserAttributes.uaNewable tp_user_attributes in
    SMap.add name {lower_bounds = empty_bounds;
                   upper_bounds = empty_bounds;
                   reified;
                   enforceable;
                   newable} tpenv in
  env_with_tpenv env
    (List.fold_left tparaml ~f:add_empty_bounds ~init:env.lenv.tpenv)

let is_generic_parameter env name =
  SMap.mem name env.lenv.tpenv

let get_generic_parameters env =
  SMap.keys (SMap.union env.lenv.tpenv env.global_tpenv)

let get_tpenv_size env =
  let local = SMap.fold (fun _x { lower_bounds; upper_bounds; reified = _; enforceable = _; newable = _ } count ->
    count + TySet.cardinal lower_bounds + TySet.cardinal upper_bounds)
    env.lenv.tpenv 0 in
    SMap.fold (fun _x { lower_bounds; upper_bounds; reified = _; enforceable = _ ; newable = _ } count ->
      count + TySet.cardinal lower_bounds + TySet.cardinal upper_bounds)
      env.global_tpenv local


(*****************************************************************************
 * Operations to get or add bounds to type variables.
 * There is a lot of code duplication from the tpenv code here, which we
 * should consider sharing in future.
 *****************************************************************************)

let get_tyvar_lower_bounds env var =
  match IMap.get var env.tvenv with
  | None -> empty_bounds
  | Some {lower_bounds; _} -> lower_bounds

let get_tyvar_upper_bounds env var =
  match IMap.get var env.tvenv with
  | None -> empty_bounds
  | Some {upper_bounds; _} -> upper_bounds

let rec is_tvar ~elide_nullable ty var =
  match ty with
  | (_, Tvar var') -> var = var'
  | (_, Toption ty) when elide_nullable -> is_tvar ~elide_nullable ty var
  | _ -> false

let set_tyvar_info env var tvinfo =
  env_with_tvenv env (IMap.add var tvinfo env.tvenv)

let remove_tyvar env var =
  (* Don't remove it entirely if we have marked it as eager_solve_fail *)
  log_env_change "remove_tyvar" env @@
  let tvinfo = get_tyvar_info env var in
  if tvinfo.eager_solve_fail
  then set_tyvar_info env var { empty_tyvar_info with eager_solve_fail = true }
  else env_with_tvenv env (IMap.remove var env.tvenv)

let set_tyvar_eager_solve_fail env var =
  let tvinfo = get_tyvar_info env var in
  set_tyvar_info env var { tvinfo with eager_solve_fail = true }

let get_tyvar_appears_covariantly env var =
  let tvinfo = get_tyvar_info env var in
  tvinfo.appears_covariantly

let get_tyvar_appears_contravariantly env var =
  let tvinfo = get_tyvar_info env var in
  tvinfo.appears_contravariantly

let get_tyvar_appears_invariantly env var =
  (get_tyvar_appears_covariantly env var) && (get_tyvar_appears_contravariantly env var)

let get_tyvar_type_consts env var =
  let tvinfo = get_tyvar_info env var in
  tvinfo.type_constants

let get_tyvar_type_const env var (_, tyconstid) =
  SMap.get tyconstid (get_tyvar_type_consts env var)

let set_tyvar_type_const env var (_, tyconstid_ as tyconstid) ty =
  let tvinfo = get_tyvar_info env var in
  let type_constants = SMap.add tyconstid_ (tyconstid, ty) tvinfo.type_constants in
  set_tyvar_info env var { tvinfo with type_constants }

(* Conjoin a subtype proposition onto the subtype_prop in the environment *)
let add_subtype_prop env prop =
  log_env_change "add_subtype_prop" env @@
  {env with subtype_prop = TL.conj env.subtype_prop prop}

(* Generate a fresh generic parameter with a specified prefix but distinct
 * from all generic parameters in the environment *)
let add_fresh_generic_parameter env prefix ~reified ~enforceable ~newable =
  let rec iterate i =
    let name = Printf.sprintf "%s#%d" prefix i in
    if is_generic_parameter env name then iterate (i+1) else name in
  let name = iterate 1 in
  let env =
    env_with_tpenv env
      (SMap.add name {lower_bounds = empty_bounds;
                      upper_bounds = empty_bounds;
                      reified;
                      enforceable;
                      newable} env.lenv.tpenv) in
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
      begin match ty with
      | _, Tvar _ -> acc
      | _ -> this#on_type acc ty
      end
  end
let get_tparams_aux env acc ty = (tparams_visitor env)#on_type acc ty
let get_tparams env ty = get_tparams_aux env SSet.empty ty

let get_tpenv_tparams env =
  SMap.fold begin fun _x { lower_bounds; upper_bounds; reified = _; enforceable = _ ; newable = _ } acc ->
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

let empty_local_id_map = Local_id.Map.empty

let empty_local tpenv local_reactive = {
  tpenv = tpenv;
  fake_members = empty_fake_members;
  local_types = LEnvC.empty_locals;
  local_using_vars = LID.Set.empty;
  local_mutability = LID.Map.empty;
  local_reactive = local_reactive;
}

let initial_local tpenv local_reactive = {
  tpenv = tpenv;
  fake_members = empty_fake_members;
  local_types = LEnvC.initial_locals;
  local_using_vars = LID.Set.empty;
  local_mutability = LID.Map.empty;
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
      (* Actually should get set straight away anyway *)
      return_type = (Reason.Rnone, Tunresolved []);
      return_disposable = false;
      return_mutable = false;
      return_explicit = false;
      return_void_to_rx = false;
    };
    params  = LID.Map.empty;
    condition_types = SMap.empty;
    self_id = "";
    self    = Reason.none, Tany;
    static  = false;
    parent_id = "";
    parent  = Reason.none, Tany;
    fun_kind = Ast.FSync;
    fun_mutable = None;
    anons   = IMap.empty;
    file    = file;
  };
  global_tpenv = SMap.empty;
  subtype_prop = TL.valid;
  log_levels = TypecheckerOptions.log_levels tcopt;
  tvenv = IMap.empty;
  tyvars_stack = [];
  allow_wildcards = false;
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

let add_wclass env x =
  let dep = Dep.Class x in
  Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep);
  ()

let get_typedef env x =
  add_wclass env x;
  TLazyHeap.get_typedef x

let is_typedef x =
  match Naming_table.Types.get_pos x with
  | Some (_p, Naming_table.TTypedef) -> true
  | _ -> false

let get_class env x =
  add_wclass env x;
  TLazyHeap.get_class x

let get_class_dep env x =
  Decl_env.add_extends_dependency env.decl_env x;
  get_class env x

let get_enum_constraint env x =
  match get_class env x with
  | None -> None
  | Some tc ->
    match (Cls.enum_type tc) with
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
  add_wclass env x;
  match TLazyHeap.get_class x with
  | Some tc when (Cls.enum_type tc) <> None -> Some tc
  | _ -> None

let is_enum env x = get_enum env x <> None

let get_typeconst env class_ mid =
  add_wclass env (Cls.name class_);
  let dep = Dep.Const ((Cls.name class_), mid) in
  Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep);
  Cls.get_typeconst class_ mid

(* Used to access class constants. *)
let get_const env class_ mid =
  add_wclass env (Cls.name class_);
  let dep = Dep.Const ((Cls.name class_), mid) in
  Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep);
  Cls.get_const class_ mid

(* Used to access "global constants". That is constants that were
 * introduced with "const X = ...;" at topelevel, or "define('X', ...);"
 *)
let get_gconst env cst_name =
  let dep = Dep.GConst cst_name in
  Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep);
  TLazyHeap.get_gconst cst_name

let get_static_member is_method env class_ mid =
  add_wclass env (Cls.name class_);
  let add_dep x =
    let dep = if is_method then Dep.SMethod (x, mid)
      else Dep.SProp (x, mid) in
    Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep);
  in
  add_dep (Cls.name class_);
  (* The type of a member is stored separately in the heap. This means that
   * any user of the member also has a dependency on the class where the member
   * originated.
   *)
  let ce_opt = if is_method then Cls.get_smethod class_ mid
    else Cls.get_sprop class_ mid in
  Option.iter ce_opt (fun ce -> add_dep ce.ce_origin);
  ce_opt

let suggest_member members mid =
  let members = Sequence.fold ~f:begin fun acc (x, {ce_type = lazy (r, _); _}) ->
    let pos = Reason.to_pos r in
    SMap.add (String.lowercase x) (pos, x) acc
  end members ~init:SMap.empty
  in
  SMap.get mid members

let suggest_static_member is_method class_ mid =
  let mid = String.lowercase mid in
  let members = if is_method then (Cls.smethods class_) else (Cls.sprops class_) in
  suggest_member members mid

let get_member is_method env class_ mid =
  add_wclass env (Cls.name class_);
  let add_dep x =
    let dep = if is_method then Dep.Method (x, mid)
      else Dep.Prop (x, mid) in
    Option.iter env.decl_env.droot (fun root -> Typing_deps.add_idep root dep)
  in
  add_dep (Cls.name class_);
  (* The type of a member is stored separately in the heap. This means that
   * any user of the member also has a dependency on the class where the member
   * originated.
   *)
  let ce_opt = if is_method then Cls.get_method class_ mid
    else Cls.get_prop class_ mid in
  Option.iter ce_opt (fun ce -> add_dep ce.ce_origin);
  ce_opt

let suggest_member is_method class_ mid =
  let mid = String.lowercase mid in
  let members = if is_method then (Cls.methods class_) else (Cls.props class_) in
  suggest_member members mid

let get_construct env class_ =
  add_wclass env (Cls.name class_);
  let add_dep x =
    let dep = Dep.Cstr (x) in
    Option.iter env.decl_env.Decl_env.droot
      (fun root -> Typing_deps.add_idep root dep);
  in
  add_dep (Cls.name class_);
  Option.iter (fst (Cls.construct class_)) (fun ce -> add_dep ce.ce_origin);
  (Cls.construct class_)

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
  let params = LID.Map.add x param params in
  set_params env params

let clear_params env =
  set_params env LID.Map.empty

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
  TLazyHeap.get_fun x

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

let check_now_or_add_todo env f =
  if TypecheckerOptions.new_inference (get_tcopt env)
  then
    let env, _ = f env in
    env
  else
    add_todo env f

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

let is_strict env = FileInfo.is_strict (get_mode env)
let is_decl env = get_mode env = FileInfo.Mdecl

let iter_anonymous env f =
  IMap.iter (fun _id (_, _, ftys, pos, _) ->
    let (untyped,typed) = !ftys in f pos (untyped @ typed)) env.genv.anons

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
    LID.to_string obj_name^"->"^member_name

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
    env, Local_id.make_unscoped my_fake_local_id

  let make_static _ env class_name member_name =
    let my_fake_local_id = make_static_id class_name member_name in
    let env = add_member env my_fake_local_id in
    env, Local_id.make_unscoped my_fake_local_id

end


(*****************************************************************************)
(* Locals *)
(*****************************************************************************)

(* We want to "take a picture" of the current type
 * that is, the current type shouldn't be affected by a
 * future unification.
 *)

let rec unbind seen env ty =
  if TypecheckerOptions.new_inference (get_tcopt env)
  then env, ty
  else
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

let add_to_local_id_map = Local_id.Map.add ?combine:None

let set_local_ env x ty =
  let local_types = LEnvC.add_to_cont C.Next x ty env.lenv.local_types in
  { env with lenv = { env.lenv with local_types } }

let next_cont_opt env = LEnvC.get_cont_option C.Next env.lenv.local_types

let next_cont_exn env = LEnvC.get_cont C.Next env.lenv.local_types

(* We maintain 2 states for a local: the type
 * that the local currently has, and an expression_id generated from
 * the last assignment to this local.
 *)
let set_local env x new_type =
  let env, new_type = unbind env new_type in
  let new_type = match new_type with
    | _, Tunresolved [ty] -> ty
    | _ -> new_type in
  match next_cont_opt env with
  | None -> env
  | Some next_cont ->
    let expr_id = match LID.Map.get x next_cont with
      | None -> Ident.tmp()
      | Some (_, y) -> y in
    let local = new_type, expr_id in
    set_local_ env x local

let is_using_var env x =
  LID.Set.mem x env.lenv.local_using_vars

let set_using_var env x =
  { env with lenv = {
    env.lenv with local_using_vars = LID.Set.add x env.lenv.local_using_vars } }

let unset_local env local =
  let {fake_members; local_types; local_using_vars; tpenv; local_mutability;
    local_reactive; } = env.lenv in
  let local_types = LEnvC.remove_from_cont C.Next local local_types in
  let local_using_vars = LID.Set.remove local local_using_vars in
  let local_mutability = LID.Map.remove local local_mutability in
  let env = { env with
    lenv = {fake_members; local_types; local_using_vars;
            tpenv; local_mutability; local_reactive} }
  in
  env

let add_mutable_var env local mutability_type =
env_with_mut
  env
  (LID.Map.add local mutability_type env.lenv.local_mutability)

let local_is_mutable ~include_borrowed env id =
  let module TME = Typing_mutability_env in
  match LID.Map.get id (get_env_mutability env) with
  | Some (_, TME.Mutable) -> true
  | Some (_, TME.Borrowed) -> include_borrowed
  | _ -> false

let get_local_in_ctx env ?error_if_undef_at_pos:p x ctx =
  let not_found_is_ok x =
    let x = LID.to_string x in
    SG.is_superglobal x && not (is_strict env) ||
    SSet.mem x env.lenv.fake_members.valid in
  let error_if_pos_provided posopt =
    match posopt with
    | Some p ->
      let in_rx_scope = env_local_reactive env in
      Errors.undefined ~in_rx_scope p (LID.to_string x);
    | None -> () in
  let lcl = LID.Map.get x ctx in
  begin match lcl with
  | None -> if not_found_is_ok x then () else error_if_pos_provided p
  | Some _ -> ()
  end;
  lcl

let tany env =
  let dynamic_view_enabled = TypecheckerOptions.dynamic_view (get_tcopt env) in
  if dynamic_view_enabled then Tdynamic else Tany

let get_local_ty_in_ctx env ?error_if_undef_at_pos x ctx =
  match get_local_in_ctx env ?error_if_undef_at_pos x ctx with
  | None -> false, (Reason.Rnone, tany env)
  | Some (x, _) -> true, x

let get_local_in_next_continuation ?error_if_undef_at_pos:p env x =
  let next_cont = next_cont_exn env in
  get_local_ty_in_ctx env ?error_if_undef_at_pos:p x next_cont

(* While checking todos at the end of a function body, the Next continuation
 * might have been moved to the 'Exit' (if there is a `return` statement)
 * or the 'Catch' continuation (if the function always throws). So we find
 * which continuation is still present and get the local from there. *)
let get_local_for_todo ?error_if_undef_at_pos:p env x =
  let local_types = env.lenv.local_types in
  let ctx = LEnvC.try_get_conts [C.Next; C.Exit; C.Catch] local_types in
  get_local_ty_in_ctx env ?error_if_undef_at_pos:p x ctx

let get_local_ ?error_if_undef_at_pos:p env x =
  if env.checking_todos
    then get_local_for_todo ?error_if_undef_at_pos:p env x
    else get_local_in_next_continuation ?error_if_undef_at_pos:p env x

let get_local env x = snd (get_local_ env x)

let get_locals env plids =
  let next_cont = next_cont_exn env in
  List.fold plids ~init:LID.Map.empty ~f:(fun locals (p, lid) ->
    match get_local_in_ctx env ~error_if_undef_at_pos:p lid next_cont with
    | None -> locals
    | Some ty_eid -> LID.Map.add lid ty_eid locals)

let set_locals env locals =
  LID.Map.fold (fun lid ty env -> set_local_ env lid ty) locals env

let is_local_defined env x = fst (get_local_ env x)

let get_local_check_defined env (p, x) =
  snd (get_local_ ~error_if_undef_at_pos:p env x)

let set_local_expr_id env x new_eid =
  let local_types = env.lenv.local_types in
  match LEnvC.get_cont_option C.Next local_types with
  | None -> env
  | Some next_cont ->
    begin match LID.Map.get x next_cont with
    | Some (type_, eid) when eid <> new_eid ->
        let local = type_, new_eid in
        let local_types = LEnvC.add_to_cont C.Next x local local_types in
        let env ={ env with lenv = { env.lenv with local_types } }
        in
        env
    | _ -> env
    end

let get_local_expr_id env x =
  let next_cont = LEnvC.get_cont C.Next env.lenv.local_types in
  let lcl = LID.Map.get x next_cont in
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
    Tast.reactivity = env_reactivity env;
    Tast.local_mutability = get_env_mutability env;
    Tast.fun_mutable = function_is_mutable env;
  }

(* Compute the type variables appearing covariantly (positively)
 * resp. contravariantly (negatively) in a given type ty.
 * Return a pair of sets of positive and negative type variables
 * (as well as an updated environment).
 *)
let rec get_tyvars env ty =
  let get_tyvars_union (env, acc_positive, acc_negative) ty =
    let env, positive, negative = get_tyvars env ty in
    env, ISet.union acc_positive positive, ISet.union acc_negative negative in
  let env, ety = expand_type env ty in
  match snd ety with
  | Tvar v ->
    env, ISet.singleton v, ISet.empty
  | Tany | Tnonnull | Terr | Tdynamic | Tobject | Tprim _ | Tanon _ ->
    env, ISet.empty, ISet.empty
  | Toption ty ->
    get_tyvars env ty
  | Ttuple tyl | Tunresolved tyl ->
    List.fold_left tyl ~init:(env, ISet.empty, ISet.empty) ~f:get_tyvars_union
  | Tshape (_, m) ->
    Nast.ShapeMap.fold
      (fun _ {sft_ty; _} res -> get_tyvars_union res sft_ty)
      m (env, ISet.empty, ISet.empty)
  | Tfun ft ->
    let env, params_positive, params_negative =
      List.fold_left ft.ft_params ~init:(env, ISet.empty, ISet.empty)
        ~f:(fun res {fp_type; _} -> get_tyvars_union res fp_type) in
    let env, ret_positive, ret_negative = get_tyvars env ft.ft_ret in
    env, ISet.union ret_positive params_negative, ISet.union ret_negative params_positive
  | Tabstract (AKnewtype (name, tyl), _) ->
    begin match get_typedef env name with
    | Some {td_tparams; _} ->
      let variancel = List.map td_tparams (fun t -> t.tp_variance) in
      get_tyvars_variance_list (env, ISet.empty, ISet.empty) variancel tyl
    | None -> env, ISet.empty, ISet.empty
    end
  | Tabstract (_, Some ty) -> get_tyvars env ty
  | Tabstract (_, None) -> env, ISet.empty, ISet.empty
  | Tclass ((_, cid), _, tyl) ->
    begin match get_class env cid with
    | Some cls ->
      let variancel = List.map (Cls.tparams cls) (fun t -> t.tp_variance) in
      get_tyvars_variance_list (env, ISet.empty, ISet.empty) variancel tyl
    | None -> env, ISet.empty, ISet.empty
    end
  | Tarraykind ak ->
    begin match ak with
    | AKany | AKempty -> env, ISet.empty, ISet.empty
    | AKvarray ty | AKvec ty | AKvarray_or_darray ty -> get_tyvars env ty
    | AKdarray (ty1, ty2) | AKmap (ty1, ty2) ->
      let env, positive1, negative1 = get_tyvars env ty1 in
      let env, positive2, negative2 = get_tyvars env ty2 in
      env, ISet.union positive1 positive2, ISet.union negative1 negative2
    end

and get_tyvars_variance_list (env, acc_positive, acc_negative) variancel tyl =
  match variancel, tyl with
  | variance::variancel, ty::tyl ->
    let env, positive, negative = get_tyvars env ty in
    let acc_positive, acc_negative =
      match variance with
      | Ast.Covariant ->
        ISet.union acc_positive positive, ISet.union acc_negative negative
      | Ast.Contravariant ->
        ISet.union acc_positive negative, ISet.union acc_negative positive
      | Ast.Invariant ->
        let positive_or_negative = ISet.union positive negative in
        ISet.union acc_positive positive_or_negative,
        ISet.union acc_negative positive_or_negative in
    get_tyvars_variance_list (env, acc_positive, acc_negative) variancel tyl
  | _ -> (env, acc_positive, acc_negative)

let rec set_tyvar_appears_covariantly env var =
  let tvinfo = get_tyvar_info env var in
  if tvinfo.appears_covariantly
  then env
  else
    let env = env_with_tvenv env (IMap.add var { tvinfo with appears_covariantly = true } env.tvenv) in
    update_variance_of_tyvars_occurring_in_lower_bounds env tvinfo.lower_bounds

and set_tyvar_appears_contravariantly env var =
  let tvinfo = get_tyvar_info env var in
  if tvinfo.appears_contravariantly
  then env
  else
    let env = env_with_tvenv env (IMap.add var { tvinfo with appears_contravariantly = true } env.tvenv) in
    update_variance_of_tyvars_occurring_in_upper_bounds env tvinfo.upper_bounds

and update_variance_of_tyvars_occurring_in_lower_bounds env tys =
  TySet.fold
    (fun ty env -> update_variance_of_tyvars_occurring_in_lower_bound env ty)
    tys env

and update_variance_of_tyvars_occurring_in_upper_bounds env tys =
  TySet.fold
    (fun ty env -> update_variance_of_tyvars_occurring_in_upper_bound env ty)
    tys env

and update_variance_of_tyvars_occurring_in_lower_bound env ty =
  let env, ety = expand_type env ty in
  match snd ety with
  | Tvar _ -> env
  | _ ->
    let env, positive, negative = get_tyvars env ty in
    let env =
      ISet.fold
        (fun var env -> set_tyvar_appears_covariantly env var)
        positive env in
    let env =
      ISet.fold
        (fun var env -> set_tyvar_appears_contravariantly env var)
        negative env in
    env

and update_variance_of_tyvars_occurring_in_upper_bound env ty =
  let env, ety = expand_type env ty in
  match snd ety with
  | Tvar _ -> env
  | _ ->
    let env, positive, negative = get_tyvars env ty in
    let env =
      ISet.fold
        (fun var env -> set_tyvar_appears_contravariantly env var)
        positive env in
    let env =
      ISet.fold
        (fun var env -> set_tyvar_appears_covariantly env var)
        negative env in
    env

(* After a type variable var has been "solved", or bound to a type ty, we need
 * to update the variance of type variables occurring in ty. Suppose that
 * variable var is marked "appears covariantly", i.e. it appears (at least) in
 * positive positions in the type of an expression. Then when we substitute ty
 * for var, variables that appear positively in ty must now be marked as
 * appearing covariantly; variables that appear negatively in ty must now be
 * marked as appearing contravariantly. And the dual, if the variable var is marked
 * "appears contravariantly".
 *)
and update_variance_after_bind env var ty =
  let appears_contravariantly = get_tyvar_appears_contravariantly env var in
  let appears_covariantly = get_tyvar_appears_covariantly env var in
  let env, positive, negative = get_tyvars env ty in
  let env =
    ISet.fold
      (fun var env ->
        let env =
          if appears_contravariantly then set_tyvar_appears_contravariantly env var else env in
          if appears_covariantly then set_tyvar_appears_covariantly env var else env)
      positive env in
  let env =
    ISet.fold
      (fun var env ->
        let env =
          if appears_contravariantly then set_tyvar_appears_covariantly env var else env in
          if appears_covariantly then set_tyvar_appears_contravariantly env var else env)
      negative env in
  env

let set_tyvar_variance env ?(flip = false) ty =
  log_env_change "set_tyvar_variance" env @@
  let tyvars = get_current_tyvars env in
  let env, positive, negative = get_tyvars env ty in
  let (positive, negative) = if flip then (negative, positive) else (positive, negative) in
  List.fold_left tyvars ~init:env ~f:(fun env var ->
    let env = if ISet.mem var positive then set_tyvar_appears_covariantly env var else env in
    let env = if ISet.mem var negative then set_tyvar_appears_contravariantly env var else env in
    env)

let fresh_invariant_type_var env p =
  let v = Ident.tmp () in
  let env =
    log_env_change "fresh_invariant_type_var" env @@
    let env = add_current_tyvar env p v in
    let env = set_tyvar_appears_covariantly env v in
    let env = set_tyvar_appears_contravariantly env v in
    env in
  env, (Reason.Rtype_variable p, Tvar v)

 (* Add a single new upper bound [ty] to type variable [var] in [env.tvenv].
  * If the optional [intersect] operation is supplied, then use this to avoid
  * adding redundant bounds by merging the type with existing bounds. This makes
  * sense because a conjunction of upper bounds
  *   (v <: t1) /\ ... /\ (v <: tn)
  * is equivalent to a single upper bound
  *   v <: (t1 & ... & tn)
  *)
 let add_tyvar_upper_bound ?intersect env var ty =
   log_env_change "add_tyvar_upper_bound" env @@
  (* Don't add superfluous v <: v or v <: ?v to environment *)
  if is_tvar ~elide_nullable:true ty var then env else
  let tvinfo = get_tyvar_info env var in
  let upper_bounds = match intersect with
    | None -> ty ++ tvinfo.upper_bounds
    | Some intersect ->
      TySet.of_list (intersect ty (TySet.elements tvinfo.upper_bounds)) in
   let env = env_with_tvenv env
    (IMap.add var { tvinfo with upper_bounds } env.tvenv) in
   if get_tyvar_appears_contravariantly env var
   then update_variance_of_tyvars_occurring_in_upper_bound env ty
   else env

(* Remove type variable `upper_var` from the upper bounds on `var`, if it exists
 *)
let remove_tyvar_upper_bound env var upper_var =
  log_env_change "remove_tyvar_upper_bound" env @@
  let tvinfo = get_tyvar_info env var in
  let upper_bounds = TySet.filter
    (fun ty -> match expand_type env ty with _, (_, Tvar v) -> v <> upper_var | _ -> true)
    tvinfo.upper_bounds in
  env_with_tvenv env (IMap.add var { tvinfo with upper_bounds } env.tvenv)


(* Remove type variable `lower_var` from the lower bounds on `var`, if it exists
 *)
let remove_tyvar_lower_bound env var lower_var =
  log_env_change "remove_tyvar_lower_bound var" env @@
  let tvinfo = get_tyvar_info env var in
  let lower_bounds = TySet.filter
    (fun ty -> match expand_type env ty with _, (_, Tvar v) -> v <> lower_var | _ -> true)
    tvinfo.lower_bounds in
  env_with_tvenv env (IMap.add var { tvinfo with lower_bounds } env.tvenv)


(* Add a single new upper bound [ty] to type variable [var] in [env.tvenv].
 * If the optional [union] operation is supplied, then use this to avoid
 * adding redundant bounds by merging the type with existing bounds. This makes
 * sense because a conjunction of lower bounds
 *   (t1 <: v) /\ ... /\ (tn <: v)
 * is equivalent to a single lower bound
 *   (t1 | ... | tn) <: v
 *)
let add_tyvar_lower_bound ?union env var ty =
  log_env_change "add_tyvar_lower_bound" env @@
  (* Don't add superfluous v <: v to environment *)
  if is_tvar ~elide_nullable:false ty var then env else
  let tvinfo = get_tyvar_info env var in
  let lower_bounds = match union with
    | None -> ty ++ tvinfo.lower_bounds
    | Some union ->
      TySet.of_list (union ty (TySet.elements tvinfo.lower_bounds)) in
  let env = env_with_tvenv env
    (IMap.add var { tvinfo with lower_bounds } env.tvenv) in
  if get_tyvar_appears_covariantly env var
  then update_variance_of_tyvars_occurring_in_lower_bound env ty
  else env
