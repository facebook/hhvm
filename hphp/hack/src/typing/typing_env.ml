(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
open Decl_env
open Typing_env_types
open Typing_defs
open Aast
open Typing_env_return_info
module Dep = Typing_deps.Dep
module Inf = Typing_inference_env
module LID = Local_id
module SN = Naming_special_names
module SG = SN.Superglobals
module LEnvC = Typing_per_cont_env
module C = Typing_continuations
module Cls = Decl_provider.Class
module Fake = Typing_fake_members
module ITySet = Internal_type_set
module TPEnv = Type_parameter_env
module KDefs = Typing_kinding_defs
module TySet = Typing_set

type class_or_typedef_result =
  | ClassResult of Typing_classes_heap.Api.t
  | TypedefResult of Typing_defs.typedef_type

let show_env _ = "<env>"

let pp_env _ _ = Printf.printf "%s\n" "<env>"

let get_tcopt env = env.genv.tcopt

let map_tcopt env ~f =
  let tcopt = f env.genv.tcopt in
  let genv = { env.genv with tcopt } in
  { env with genv }

let get_deps_mode env = Provider_context.get_deps_mode env.decl_env.Decl_env.ctx

let get_ctx env = env.decl_env.Decl_env.ctx

let get_file env = env.genv.file

let get_current_decl env =
  Option.map env.decl_env.Decl_env.droot ~f:Typing_deps.Dep.to_decl_reference

let get_current_decl_and_file env : Pos_or_decl.ctx =
  { Pos_or_decl.file = get_file env; decl = get_current_decl env }

let fill_in_pos_filename_if_in_current_decl env pos =
  Pos_or_decl.fill_in_filename_if_in_current_decl
    ~current_decl_and_file:(get_current_decl_and_file env)
    pos

let unify_error_assert_primary_pos_in_current_decl env =
  Errors.unify_error_assert_primary_pos_in_current_decl
    ~current_decl_and_file:(get_current_decl_and_file env)

let invalid_type_hint_assert_primary_pos_in_current_decl env =
  Errors.invalid_type_hint_assert_primary_pos_in_current_decl
    ~current_decl_and_file:(get_current_decl_and_file env)

let get_tracing_info env = env.tracing_info

let set_log_level env key log_level =
  { env with log_levels = SMap.add key log_level env.log_levels }

let get_log_level env key =
  Option.value (SMap.find_opt key env.log_levels) ~default:0

let env_log_function = ref (fun _pos _name _old_env _new_env -> ())

let set_env_log_function f = env_log_function := f

let log_env_change_ :
    type res. string -> ?level:int -> env -> env * res -> env * res =
 fun name ?(level = 1) old_env (new_env, res) ->
  ( if get_log_level new_env name >= 1 || get_log_level new_env "env" >= level
  then
    let pos =
      Option.value
        (Inf.get_current_pos_from_tyvar_stack old_env.inference_env)
        ~default:old_env.function_pos
    in
    !env_log_function pos name old_env new_env );
  (new_env, res)

let log_env_change name ?(level = 1) old_env new_env =
  let (env, ()) = log_env_change_ name ~level old_env (new_env, ()) in
  env

let wrap_inference_env_call :
    type res. env -> (Inf.t -> Inf.t * res) -> env * res =
 fun env f ->
  let (inference_env, res) = f env.inference_env in
  ({ env with inference_env }, res)

let wrap_inference_env_call_res : type res. env -> (Inf.t -> res) -> res =
 fun env f ->
  let (_env, res) =
    wrap_inference_env_call env (fun env ->
        let res = f env in
        (env, res))
  in
  res

let wrap_inference_env_call_env : env -> (Inf.t -> Inf.t) -> env =
 fun env f ->
  let (env, ()) =
    wrap_inference_env_call env (fun env ->
        let env = f env in
        (env, ()))
  in
  env

let expand_var env r v =
  wrap_inference_env_call env (fun env -> Inf.expand_var env r v)

let fresh_type_reason ?variance env p r =
  log_env_change_ "fresh_type_reason" env
  @@ wrap_inference_env_call env (fun env ->
         Inf.fresh_type_reason ?variance env p r)

let fresh_type env p =
  log_env_change_ "fresh_type" env
  @@ wrap_inference_env_call env (fun env -> Inf.fresh_type env p)

let fresh_type_invariant env p =
  log_env_change_ "fresh_type_invariant" env
  @@ wrap_inference_env_call env (fun env -> Inf.fresh_type_invariant env p)

let new_global_tyvar env ?i r =
  log_env_change_ "new_global_tyvar" env
  @@ wrap_inference_env_call env (fun env -> Inf.new_global_tyvar env ?i r)

let add_subtype_prop env prop =
  log_env_change "add_subtype_prop" env
  @@ wrap_inference_env_call_env env (fun env -> Inf.add_subtype_prop env prop)

let is_global_tyvar env var =
  wrap_inference_env_call_res env (fun env -> Inf.is_global_tyvar env var)

let get_global_tyvar_reason env var =
  wrap_inference_env_call_res env (fun env ->
      Inf.get_global_tyvar_reason env var)

let empty_bounds = TySet.empty

let tyvar_is_solved_or_skip_global env var =
  Inf.tyvar_is_solved_or_skip_global env.inference_env var

let make_tyvar_no_more_occur_in_tyvar env v ~no_more_in:v' =
  wrap_inference_env_call_env env (fun env ->
      Inf.make_tyvar_no_more_occur_in_tyvar env v ~no_more_in:v')

let not_implemented s _ =
  failwith (Printf.sprintf "Function %s not implemented" s)

type simplify_unions = env -> locl_ty -> env * locl_ty

let (simplify_unions_ref : simplify_unions ref) =
  ref (not_implemented "simplify_unions")

let simplify_unions x = !simplify_unions_ref x

let bind env v ty =
  wrap_inference_env_call_env env (fun env -> Inf.bind env v ty)

(** Unions and intersections containing unsolved type variables may remain
in an unsimplified form once those type variables get solved.

For example, consider the union (#1 | int) where #1 is an unsolved type variable.
If #1 gets solved to int, then this union will remain in the unsimplified form
(int | int) which compromise the robustness of some of our logic and might
cause performance issues (by creating big unsimplified unions).

To solve this problem, we wrap each union and intersection in a type var,
so we'd get `#2 -> (#1 | int)` (This is done in Typing_union and
Typing_intersection), and register that #1 occurs in #2 in
[env.tyvar_occurrences]. Then when #1 gets solved, we simplify #2.

This function deals with this simplification.

The simplification is recursive: simplifying a type variable will
trigger simplification of its own occurrences. *)
let simplify_occurrences env v =
  let rec simplify_occurrences env v ~seen_tyvars =
    let vars = Inf.get_tyvar_occurrences env.inference_env v in
    let (env, seen_tyvars) =
      ISet.fold
        (fun v' (env, seen_tyvars) ->
          (* This type variable is now solved and does not contain any unsolved
          type variable, so we can remove it from its occurrences. *)
          let env = make_tyvar_no_more_occur_in_tyvar env v ~no_more_in:v' in
          (* Only simplify when the type of v' does not contain any more
          unsolved type variables. *)
          if not @@ Inf.contains_unsolved_tyvars env.inference_env v' then
            simplify_type_of_var env v' ~seen_tyvars
          else
            (env, seen_tyvars))
        vars
        (env, seen_tyvars)
    in
    (env, seen_tyvars)
  and simplify_type_of_var env v ~seen_tyvars =
    if ISet.mem v seen_tyvars then
      (* TODO raise exception. *)
      (env, seen_tyvars)
    else
      let seen_tyvars = ISet.add v seen_tyvars in
      match Inf.get_direct_binding env.inference_env v with
      | None -> failwith "Can only simplify type of bounded variables"
      | Some ty ->
        (* Only simplify the type of variables which are bound directly to a
      concrete type to preserve the variable aliasings and save some memory. *)
        let env =
          match get_node ty with
          | Tvar _ -> env
          | _ ->
            let (env, ty) = simplify_unions env ty in
            (* we only call this function when v does not recursively contain unsolved
          type variables, so ty here should not contain unsolved type variables and
          it is safe to simply bind it without reupdating the type var occurrences. *)
            let env = bind env v ty in
            env
        in
        simplify_occurrences env v ~seen_tyvars
  in
  if not @@ Inf.contains_unsolved_tyvars env.inference_env v then
    fst @@ simplify_occurrences env v ~seen_tyvars:ISet.empty
  else
    env

let add env ?(tyvar_pos = Pos.none) v ty =
  let env =
    wrap_inference_env_call_env env (fun env -> Inf.add env ~tyvar_pos v ty)
  in
  let env = simplify_occurrences env v in
  env

let get_type env r var =
  wrap_inference_env_call env (fun env -> Inf.get_type env r var)

let expand_type env ty =
  wrap_inference_env_call env (fun env -> Inf.expand_type env ty)

let expand_internal_type env ty =
  wrap_inference_env_call env (fun env -> Inf.expand_internal_type env ty)

let get_tyvar_pos env var =
  wrap_inference_env_call_res env (fun env -> Inf.get_tyvar_pos env var)

let get_tyvar_lower_bounds env var =
  wrap_inference_env_call_res env (fun env ->
      Inf.get_tyvar_lower_bounds env var)

let set_tyvar_lower_bounds env var tys =
  wrap_inference_env_call_env env (fun env ->
      Inf.set_tyvar_lower_bounds env var tys)

let get_tyvar_upper_bounds env var =
  wrap_inference_env_call_res env (fun env ->
      Inf.get_tyvar_upper_bounds env var)

let set_tyvar_upper_bounds env var tys =
  wrap_inference_env_call_env env (fun env ->
      Inf.set_tyvar_upper_bounds env var tys)

let get_tyvar_appears_covariantly env var =
  wrap_inference_env_call_res env (fun env ->
      Inf.get_tyvar_appears_covariantly env var)

let set_tyvar_appears_covariantly env var =
  wrap_inference_env_call_env env (fun env ->
      Inf.set_tyvar_appears_covariantly env var)

let get_tyvar_appears_contravariantly env var =
  wrap_inference_env_call_res env (fun env ->
      Inf.get_tyvar_appears_contravariantly env var)

let set_tyvar_appears_contravariantly env var =
  wrap_inference_env_call_env env (fun env ->
      Inf.set_tyvar_appears_contravariantly env var)

let get_tyvar_appears_invariantly env var =
  wrap_inference_env_call_res env (fun env ->
      Inf.get_tyvar_appears_invariantly env var)

let get_tyvar_eager_solve_fail env var =
  wrap_inference_env_call_res env (fun env ->
      Inf.get_tyvar_eager_solve_fail env var)

let set_tyvar_eager_solve_fail env var =
  wrap_inference_env_call_env env (fun env ->
      Inf.set_tyvar_eager_solve_fail env var)

let get_tyvar_type_consts env var =
  wrap_inference_env_call_res env (fun env -> Inf.get_tyvar_type_consts env var)

let get_tyvar_type_const env var tid =
  wrap_inference_env_call_res env (fun env ->
      Inf.get_tyvar_type_const env var tid)

let set_tyvar_type_const env var tconstid ty =
  wrap_inference_env_call_env env (fun env ->
      Inf.set_tyvar_type_const env var tconstid ty)

let get_current_tyvars env =
  wrap_inference_env_call_res env Inf.get_current_tyvars

let open_tyvars env p =
  wrap_inference_env_call_env env (fun env -> Inf.open_tyvars env p)

let close_tyvars env = wrap_inference_env_call_env env Inf.close_tyvars

let extract_global_inference_env env =
  wrap_inference_env_call env (fun env -> Inf.extract_global_inference_env env)

let wrap_ty_in_var env r ty =
  wrap_inference_env_call env (fun env -> Inf.wrap_ty_in_var env r ty)

let get_shape_field_name = function
  | Typing_defs.TSFlit_int (_, s)
  | Typing_defs.TSFlit_str (_, s) ->
    s
  | Typing_defs.TSFclass_const ((_, s1), (_, s2)) -> s1 ^ "::" ^ s2

let get_shape_field_name_pos = function
  | Typing_defs.TSFlit_int (p, _)
  | Typing_defs.TSFlit_str (p, _)
  | Typing_defs.TSFclass_const ((p, _), _) ->
    p

let next_cont_opt env = LEnvC.get_cont_option C.Next env.lenv.per_cont_env

let all_continuations env = LEnvC.all_continuations env.lenv.per_cont_env

let get_tpenv env =
  match next_cont_opt env with
  | None -> TPEnv.empty
  | Some entry -> entry.Typing_per_cont_env.tpenv

let get_global_tpenv env = env.tpenv

let get_pos_and_kind_of_generic env name =
  match TPEnv.get_with_pos name (get_tpenv env) with
  | Some r -> Some r
  | None -> TPEnv.get_with_pos name env.tpenv

let get_lower_bounds env name tyargs =
  let tpenv = get_tpenv env in
  let local = TPEnv.get_lower_bounds tpenv name tyargs in
  let global = TPEnv.get_lower_bounds env.tpenv name tyargs in
  TySet.union local global

let get_upper_bounds env name tyargs =
  let tpenv = get_tpenv env in
  let local = TPEnv.get_upper_bounds tpenv name tyargs in
  let global = TPEnv.get_upper_bounds env.tpenv name tyargs in
  TySet.union local global

let get_reified env name =
  let tpenv = get_tpenv env in
  let local = TPEnv.get_reified tpenv name in
  let global = TPEnv.get_reified env.tpenv name in
  match (local, global) with
  | (Reified, _)
  | (_, Reified) ->
    Reified
  | (SoftReified, _)
  | (_, SoftReified) ->
    SoftReified
  | _ -> Erased

let get_enforceable env name =
  let tpenv = get_tpenv env in
  let local = TPEnv.get_enforceable tpenv name in
  let global = TPEnv.get_enforceable env.tpenv name in
  local || global

let get_newable env name =
  let tpenv = get_tpenv env in
  let local = TPEnv.get_newable tpenv name in
  let global = TPEnv.get_newable env.tpenv name in
  local || global

let get_require_dynamic env name =
  let tpenv = get_tpenv env in
  let local = TPEnv.get_require_dynamic tpenv name in
  let global = TPEnv.get_require_dynamic env.tpenv name in
  local || global

(* Get bounds that are both an upper and lower of a given generic *)
let get_equal_bounds env name tyargs =
  let lower = get_lower_bounds env name tyargs in
  let upper = get_upper_bounds env name tyargs in
  TySet.inter lower upper

let env_with_tpenv env tpenv =
  {
    env with
    lenv =
      {
        env.lenv with
        per_cont_env =
          Typing_per_cont_env.(
            update_cont_entry C.Next env.lenv.per_cont_env (fun entry ->
                { entry with tpenv }));
      };
  }

let env_with_global_tpenv env tpenv = { env with tpenv }

let add_upper_bound_global env name ty =
  let tpenv =
    let (env, ty) = expand_type env ty in
    match deref ty with
    | (r, Tgeneric (formal_super, [])) ->
      TPEnv.add_lower_bound env.tpenv formal_super (mk (r, Tgeneric (name, [])))
    | (_r, Tgeneric (_formal_super, _targs)) ->
      (* TODO(T70068435) Revisit this when implementing bounds on HK generic vars *)
      env.tpenv
    | _ -> env.tpenv
  in
  { env with tpenv = TPEnv.add_upper_bound tpenv name ty }

let add_lower_bound_global env name ty =
  let tpenv =
    let (env, ty) = expand_type env ty in
    match deref ty with
    | (r, Tgeneric (formal_super, [])) ->
      TPEnv.add_upper_bound env.tpenv formal_super (mk (r, Tgeneric (name, [])))
    | (_r, Tgeneric (_formal_super, _targs)) ->
      (* TODO(T70068435) Revisit this when implementing bounds on HK generic vars *)
      env.tpenv
    | _ -> env.tpenv
  in
  { env with tpenv = TPEnv.add_lower_bound tpenv name ty }

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
  env_with_tpenv env (TPEnv.add_upper_bound ?intersect (get_tpenv env) name ty)

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
  env_with_tpenv env (TPEnv.add_lower_bound ?union (get_tpenv env) name ty)

(* Add type parameters to environment, initially with no bounds.
 * Existing type parameters with the same name will be overridden. *)
let add_generic_parameters env tparaml =
  env_with_tpenv env (TPEnv.add_generic_parameters (get_tpenv env) tparaml)

let is_generic_parameter env name =
  TPEnv.mem name (get_tpenv env) || SSet.mem name env.fresh_typarams

let get_generic_parameters env =
  TPEnv.get_tparam_names (TPEnv.union (get_tpenv env) env.tpenv)

let get_tpenv_size env = TPEnv.size (get_tpenv env) + TPEnv.size env.tpenv

let is_consistent env = TPEnv.is_consistent (get_tpenv env)

let mark_inconsistent env =
  env_with_tpenv env (TPEnv.mark_inconsistent (get_tpenv env))

let fresh_param_name env prefix : env * string =
  let rec iterate i =
    let name = Printf.sprintf "%s#%d" prefix i in
    if is_generic_parameter env name then
      iterate (i + 1)
    else
      name
  in
  let name = iterate 1 in
  let env = { env with fresh_typarams = SSet.add name env.fresh_typarams } in
  (env, name)

(* Generate a fresh generic parameter with a specified prefix but distinct
 * from all generic parameters in the environment *)
let add_fresh_generic_parameter_by_kind env pos prefix kind =
  let (env, name) = fresh_param_name env prefix in
  let env =
    env_with_tpenv env (TPEnv.add ~def_pos:pos name kind (get_tpenv env))
  in
  (env, name)

let add_fresh_generic_parameter env pos prefix ~reified ~enforceable ~newable =
  let kind =
    KDefs.
      {
        lower_bounds = empty_bounds;
        upper_bounds = empty_bounds;
        reified;
        enforceable;
        newable;
        require_dynamic = true;
        parameters = [];
      }
  in
  add_fresh_generic_parameter_by_kind env pos prefix kind

let is_fresh_generic_parameter name =
  String.contains name '#' && not (DependentKind.is_generic_dep_ty name)

let tparams_visitor env =
  object (this)
    inherit [SSet.t] Type_visitor.locl_type_visitor

    method! on_tgeneric acc _ s _ =
      (* as for tnewtype: not traversing args, although they may contain Tgenerics *)
      SSet.add s acc

    (* Perserving behavior but this seems incorrect to me since a newtype may
     * contain type arguments with generics
     *)
    method! on_tdependent acc _ _ _ = acc

    method! on_tnewtype acc _ _ _ _ = acc

    method! on_tvar acc r ix =
      let (_env, ty) = expand_var env r ix in
      match get_node ty with
      | Tvar _ -> acc
      | _ -> this#on_type acc ty
  end

let get_tparams_aux env acc ty = (tparams_visitor env)#on_type acc ty

let get_tparams env ty = get_tparams_aux env SSet.empty ty

let get_tpenv_tparams env =
  TPEnv.fold
    begin
      fun _x
          KDefs.
            {
              lower_bounds;
              upper_bounds;
              reified = _;
              enforceable = _;
              newable = _;
              require_dynamic = _;
              (* FIXME what to do here? it seems dangerous to just traverse *)
              parameters = _;
            }
          acc ->
      let folder ty acc =
        let (_env, ty) = expand_type env ty in
        match get_node ty with
        | Tgeneric _ -> acc
        | _ -> get_tparams_aux env acc ty
      in
      TySet.fold folder lower_bounds @@ TySet.fold folder upper_bounds acc
    end
    (get_tpenv env)
    SSet.empty

(* Replace types for locals with empty environment *)
let env_with_locals env locals =
  { env with lenv = { env.lenv with per_cont_env = locals } }

(* This is used whenever we start checking a method. Retain tpenv from the class type parameters *)
let reinitialize_locals env =
  env_with_locals
    env
    LEnvC.(initial_locals { empty_entry with tpenv = get_tpenv env })

let initial_local tpenv =
  {
    per_cont_env = LEnvC.(initial_locals { empty_entry with tpenv });
    local_using_vars = LID.Set.empty;
  }

let empty ?origin ?(mode = FileInfo.Mstrict) ctx file ~droot =
  {
    function_pos = Pos.none;
    fresh_typarams = SSet.empty;
    lenv = initial_local TPEnv.empty;
    in_loop = false;
    in_try = false;
    in_case = false;
    in_expr_tree = false;
    inside_constructor = false;
    in_support_dynamic_type_method_check = false;
    decl_env = { mode; droot; ctx };
    tracing_info =
      Option.map origin ~f:(fun origin -> { Decl_counters.origin; file });
    genv =
      {
        tcopt = Provider_context.get_tcopt ctx;
        return =
          {
            (* Actually should get set straight away anyway *)
            return_type =
              {
                et_type = mk (Reason.Rnone, Tunion []);
                et_enforced = Unenforced;
              };
            return_disposable = false;
            return_explicit = false;
            return_dynamically_callable = false;
          };
        params = LID.Map.empty;
        condition_types = SMap.empty;
        self = None;
        static = false;
        val_kind = Other;
        parent = None;
        fun_kind = Ast_defs.FSync;
        fun_is_ctor = false;
        file;
        this_module = None;
        this_internal = false;
      };
    tpenv = TPEnv.empty;
    log_levels = TypecheckerOptions.log_levels (Provider_context.get_tcopt ctx);
    inference_env = Inf.empty_inference_env;
    allow_wildcards = false;
    big_envs = ref [];
    pessimize = false;
    fun_tast_info = None;
  }

let set_env_pessimize env =
  let pessimize_coefficient =
    TypecheckerOptions.simple_pessimize (get_tcopt env)
  in
  let pessimize =
    Pos.pessimize_enabled env.function_pos pessimize_coefficient
  in
  { env with pessimize }

let set_env_function_pos env function_pos = { env with function_pos }

let set_fun_tast_info env fun_tast_info =
  { env with fun_tast_info = Some fun_tast_info }

let set_condition_type env n ty =
  {
    env with
    genv =
      { env.genv with condition_types = SMap.add n ty env.genv.condition_types };
  }

let get_condition_type env n = SMap.find_opt n env.genv.condition_types

let fun_is_constructor env = env.genv.fun_is_ctor

let set_fun_is_constructor env is_ctor =
  { env with genv = { env.genv with fun_is_ctor = is_ctor } }

let make_depend_on_class env class_name =
  let dep = Dep.Type class_name in
  Option.iter env.decl_env.droot ~f:(fun root ->
      Typing_deps.add_idep (get_deps_mode env) root dep);
  ()

let make_depend_on_constructor env class_name =
  make_depend_on_class env class_name;
  let dep = Dep.Cstr class_name in
  Option.iter env.decl_env.droot ~f:(fun root ->
      Typing_deps.add_idep (get_deps_mode env) root dep);
  ()

let make_depend_on_class_def env x cd =
  match cd with
  | Some cd when Pos_or_decl.is_hhi (Cls.pos cd) -> ()
  | _ -> make_depend_on_class env x

let print_size _kind _name obj =
  match obj with
  | None -> obj
  | Some _r ->
    (*    Printf.printf
      "%s %s: %d\n"
      kind
      name
      (Obj.reachable_words (Obj.repr r) * (Sys.word_size / 8));*)
    obj

let get_typedef env x =
  let res =
    print_size
      "type"
      x
      (Decl_provider.get_typedef
         ?tracing_info:(get_tracing_info env)
         (get_ctx env)
         x)
  in
  match res with
  | Some td when Pos_or_decl.is_hhi td.td_pos -> res
  | _ ->
    make_depend_on_class env x;
    res

let is_typedef env x =
  match Naming_provider.get_type_kind (get_ctx env) x with
  | Some Naming_types.TTypedef -> true
  | _ -> false

let is_typedef_visible env ?(expand_visible_newtype = true) td =
  let { Typing_defs.td_vis; td_pos; _ } = td in
  match td_vis with
  | Aast.Opaque ->
    expand_visible_newtype
    && Relative_path.equal
         (Pos.filename (Pos_or_decl.unsafe_to_raw_pos td_pos))
         (get_file env)
  | Aast.Transparent
  (* Internal types are not opaque, but they may be inaccessible altogether *)
  | Aast.Tinternal ->
    true

let get_class (env : env) (name : Decl_provider.type_key) : Cls.t option =
  let res =
    print_size
      "class"
      name
      (Decl_provider.get_class
         ?tracing_info:(get_tracing_info env)
         (get_ctx env)
         name)
  in
  make_depend_on_class_def env name res;
  res

let get_class_or_typedef env x =
  if is_typedef env x then
    match get_typedef env x with
    | None -> None
    | Some td -> Some (TypedefResult td)
  else
    match get_class env x with
    | None -> None
    | Some cd -> Some (ClassResult cd)

let get_class_dep env x =
  let res = get_class env x in
  match res with
  | Some cd when Pos_or_decl.is_hhi (Cls.pos cd) -> res
  | _ ->
    Decl_env.add_extends_dependency env.decl_env x;
    res

let get_fun env x =
  let res =
    print_size
      "fun"
      x
      (Decl_provider.get_fun
         ?tracing_info:(get_tracing_info env)
         (get_ctx env)
         x)
  in
  match res with
  | Some fd when Pos_or_decl.is_hhi fd.fe_pos -> res
  | _ ->
    let dep = Typing_deps.Dep.Fun x in
    Option.iter env.decl_env.Decl_env.droot ~f:(fun root ->
        Typing_deps.add_idep (get_deps_mode env) root dep);
    res

let get_enum_constraint env x =
  match get_class env x with
  | None -> None
  | Some tc ->
    (match Cls.enum_type tc with
    | None -> None
    | Some e -> e.te_constraint)

let get_enum env x =
  match get_class env x with
  | Some tc when Option.is_some (Cls.enum_type tc) -> Some tc
  | _ -> None

let is_enum env x =
  match get_enum env x with
  | Some cls ->
    (match Cls.enum_type cls with
    | Some enum_type -> not enum_type.te_enum_class
    | None -> false (* we know this is impossible due to get_enum *))
  | None -> false

let is_enum_class env x =
  match get_enum env x with
  | Some cls ->
    (match Cls.enum_type cls with
    | Some enum_type -> enum_type.te_enum_class
    | None -> false (* we know this is impossible due to get_enum *))
  | None -> false

let get_typeconst env class_ mid =
  if not (Pos_or_decl.is_hhi (Cls.pos class_)) then begin
    let dep = Dep.Const (Cls.name class_, mid) in
    make_depend_on_class env (Cls.name class_);
    Option.iter env.decl_env.droot ~f:(fun root ->
        Typing_deps.add_idep (get_deps_mode env) root dep)
  end;
  Cls.get_typeconst class_ mid

(* Used to access class constants. *)
let get_const env class_ mid =
  if not (Pos_or_decl.is_hhi (Cls.pos class_)) then begin
    let dep = Dep.Const (Cls.name class_, mid) in
    make_depend_on_class env (Cls.name class_);
    Option.iter env.decl_env.droot ~f:(fun root ->
        Typing_deps.add_idep (get_deps_mode env) root dep)
  end;
  Cls.get_const class_ mid

let consts env class_ =
  if not (Pos_or_decl.is_hhi (Cls.pos class_)) then
    make_depend_on_class env (Cls.name class_);
  Cls.consts class_

(* Used to access "global constants". That is constants that were
 * introduced with "const X = ...;" at topelevel, or "define('X', ...);"
 *)
let get_gconst env cst_name =
  let res =
    Decl_provider.get_gconst
      ?tracing_info:(get_tracing_info env)
      (get_ctx env)
      cst_name
  in
  match res with
  | Some cst when Pos_or_decl.is_hhi cst.cd_pos -> res
  | _ ->
    let dep = Dep.GConst cst_name in
    Option.iter env.decl_env.droot ~f:(fun root ->
        Typing_deps.add_idep (get_deps_mode env) root dep);
    res

let get_static_member is_method env class_ mid =
  (* The type of a member is stored separately in the heap. This means that
   * any user of the member also has a dependency on the class where the member
   * originated.
   *)
  let ce_opt =
    if is_method then
      Cls.get_smethod class_ mid
    else
      Cls.get_sprop class_ mid
  in
  if not (Pos_or_decl.is_hhi (Cls.pos class_)) then begin
    make_depend_on_class env (Cls.name class_);
    let add_dep x =
      let dep =
        if is_method then
          Dep.SMethod (x, mid)
        else
          Dep.SProp (x, mid)
      in
      Option.iter env.decl_env.droot ~f:(fun root ->
          Typing_deps.add_idep (get_deps_mode env) root dep)
    in
    add_dep (Cls.name class_);
    Option.iter ce_opt ~f:(fun ce -> add_dep ce.ce_origin)
  end;

  ce_opt

(* Given a list of things whose name we can extract with `f`, return
   the item whose name is closest to `name`. *)
let most_similar (name : string) (possibilities : 'a list) (f : 'a -> string) :
    'a option =
  let distance = String_utils.levenshtein_distance in
  let choose_closest x y =
    if distance (f x) name < distance (f y) name then
      x
    else
      y
  in
  List.fold possibilities ~init:None ~f:(fun acc possibility ->
      match acc with
      | None -> Some possibility
      | Some current_best -> Some (choose_closest current_best possibility))

let suggest_member members mid =
  let pairs =
    List.map members ~f:(fun (x, { ce_type = (lazy ty); _ }) -> (get_pos ty, x))
  in
  most_similar mid pairs snd

let suggest_static_member is_method class_ mid =
  let mid = String.lowercase mid in
  let members =
    if is_method then
      Cls.smethods class_
    else
      Cls.sprops class_
  in
  suggest_member members mid

let get_member is_method env class_ mid =
  let add_dep x =
    let dep =
      if is_method then
        Dep.Method (x, mid)
      else
        Dep.Prop (x, mid)
    in
    Option.iter env.decl_env.droot ~f:(fun root ->
        Typing_deps.add_idep (get_deps_mode env) root dep)
  in
  (* The type of a member is stored separately in the heap. This means that
   * any user of the member also has a dependency on the class where the member
   * originated.
   *)
  let ce_opt =
    if is_method then
      Cls.get_method class_ mid
    else
      Cls.get_prop class_ mid
  in
  if not (Pos_or_decl.is_hhi (Cls.pos class_)) then
    make_depend_on_class env (Cls.name class_);
  Option.iter ce_opt ~f:(fun ce ->
      add_dep (Cls.name class_);
      add_dep ce.ce_origin);
  ce_opt

let suggest_member is_method class_ mid =
  let mid = String.lowercase mid in
  let members =
    if is_method then
      Cls.methods class_
    else
      Cls.props class_
  in
  suggest_member members mid

let get_construct env class_ =
  if not (Pos_or_decl.is_hhi (Cls.pos class_)) then begin
    make_depend_on_constructor env (Cls.name class_);
    Option.iter
      (fst (Cls.construct class_))
      ~f:(fun ce -> make_depend_on_constructor env ce.ce_origin)
  end;
  Cls.construct class_

let get_return env = env.genv.return

let set_return env x =
  let genv = env.genv in
  let genv = { genv with return = x } in
  { env with genv }

let get_params env = env.genv.params

let set_params env params = { env with genv = { env.genv with params } }

let set_param env x param =
  let params = get_params env in
  let params = LID.Map.add x param params in
  set_params env params

let clear_params env = set_params env LID.Map.empty

let with_env env f =
  let ret = get_return env in
  let params = get_params env in
  let (env, result) = f env in
  let env = set_params env params in
  let env = set_return env ret in
  (env, result)

let with_origin env origin f =
  let ti1 = env.tracing_info in
  let ti2 = Option.map ti1 ~f:(fun ti -> { ti with Decl_counters.origin }) in
  let env = { env with tracing_info = ti2 } in
  let (env, result) = f env in
  let env = { env with tracing_info = ti1 } in
  (env, result)

let with_origin2 env origin f =
  let ti1 = env.tracing_info in
  let ti2 = Option.map ti1 ~f:(fun ti -> { ti with Decl_counters.origin }) in
  let env = { env with tracing_info = ti2 } in
  let (env, r1, r2) = f env in
  let env = { env with tracing_info = ti1 } in
  (env, r1, r2)

let with_in_expr_tree env in_expr_tree f =
  let old_in_expr_tree = env.in_expr_tree in
  let env = { env with in_expr_tree } in
  let (env, r1, r2) = f env in
  let env = { env with in_expr_tree = old_in_expr_tree } in
  (env, r1, r2)

let is_in_expr_tree env = env.in_expr_tree

let set_in_expr_tree env b = { env with in_expr_tree = b }

let is_static env = env.genv.static

let get_val_kind env = env.genv.val_kind

let get_self_ty env = Option.map env.genv.self ~f:snd

let get_self_class_type env =
  match get_self_ty env with
  | Some self ->
    begin
      match get_node self with
      | Tclass (id, exact, tys) -> Some (id, exact, tys)
      | _ -> None
    end
  | None -> None

let get_self_id env = Option.map env.genv.self ~f:fst

let get_self_class env =
  let open Option in
  get_self_id env >>= get_class env

let get_parent_ty env = Option.map env.genv.parent ~f:snd

let get_parent_id env = Option.map env.genv.parent ~f:fst

let get_parent_class env =
  let open Option in
  get_parent_id env >>= get_class_dep env

let get_fn_kind env = env.genv.fun_kind

let set_fn_kind env fn_type =
  let genv = env.genv in
  let genv = { genv with fun_kind = fn_type } in
  { env with genv }

let set_module env m = { env with genv = { env.genv with this_module = m } }

let get_module env = env.genv.this_module

let set_internal env b = { env with genv = { env.genv with this_internal = b } }

let get_internal env = env.genv.this_internal

let set_self env self_id self_ty =
  let genv = env.genv in
  let genv = { genv with self = Some (self_id, self_ty) } in
  { env with genv }

let set_parent env parent_id parent_ty =
  let genv = env.genv in
  let genv = { genv with parent = Some (parent_id, parent_ty) } in
  { env with genv }

let set_static env =
  let genv = env.genv in
  let genv = { genv with static = true } in
  { env with genv }

let set_val_kind env x =
  let genv = env.genv in
  let genv = { genv with val_kind = x } in
  { env with genv }

let set_mode env mode =
  let decl_env = env.decl_env in
  let decl_env = { decl_env with mode } in
  { env with decl_env }

let get_mode env = env.decl_env.mode

let is_strict env = FileInfo.is_strict (get_mode env)

let is_hhi env = FileInfo.(equal_mode (get_mode env) Mhhi)

let get_allow_solve_globals env =
  wrap_inference_env_call_res env Inf.get_allow_solve_globals

let set_allow_solve_globals env flag =
  wrap_inference_env_call_env env (fun env ->
      Inf.set_allow_solve_globals env flag)

(*****************************************************************************)
(* Locals *)
(*****************************************************************************)

let set_local_ env x ty =
  let per_cont_env = LEnvC.add_to_cont C.Next x ty env.lenv.per_cont_env in
  { env with lenv = { env.lenv with per_cont_env } }

(* We maintain 2 states for a local: the type
 * that the local currently has, and an expression_id generated from
 * the last assignment to this local.
 *)
let set_local ?(immutable = false) env x new_type pos =
  let new_type =
    match get_node new_type with
    | Tunion [ty] -> ty
    | _ -> new_type
  in
  match next_cont_opt env with
  | None -> env
  | Some next_cont ->
    let expr_id =
      match LID.Map.find_opt x next_cont.LEnvC.local_types with
      | None -> Ident.tmp ()
      | Some (_, _, y) -> y
    in
    let expr_id =
      if immutable then
        LID.make_immutable expr_id
      else
        expr_id
    in
    let local = (new_type, pos, expr_id) in
    set_local_ env x local

let is_using_var env x = LID.Set.mem x env.lenv.local_using_vars

let set_using_var env x =
  {
    env with
    lenv =
      {
        env.lenv with
        local_using_vars = LID.Set.add x env.lenv.local_using_vars;
      };
  }

let unset_local env local =
  let { per_cont_env; local_using_vars } = env.lenv in
  let per_cont_env =
    LEnvC.remove_from_cont C.Next local
    @@ LEnvC.remove_from_cont C.Catch local
    @@ per_cont_env
  in
  let local_using_vars = LID.Set.remove local local_using_vars in
  let env = { env with lenv = { per_cont_env; local_using_vars } } in
  env

let tany env =
  let dynamic_view_enabled = TypecheckerOptions.dynamic_view (get_tcopt env) in
  if dynamic_view_enabled then
    Tdynamic
  else
    Typing_defs.make_tany ()

let get_local_in_ctx env ?error_if_undef_at_pos:p x ctx_opt =
  let not_found_is_ok x ctx =
    let xstr = LID.to_string x in
    (String.equal xstr SG.globals || SG.is_superglobal xstr)
    && not (is_strict env)
    || Fake.is_valid ctx.LEnvC.fake_members x
  in
  let error_if_pos_provided posopt ctx =
    (* Is this local variable something the user could write, or an
       internal variable used in desugaring? *)
    let denotable_local (lid : LID.t) : bool =
      (* Allow $foo or $_bar1 but not $#foo or $0bar. *)
      let local_regexp = Str.regexp "^\\$[a-zA-z_][a-zA-Z0-9_]*$" in
      Str.string_match local_regexp (LID.to_string lid) 0
    in

    match posopt with
    | Some p ->
      let lid = LID.to_string x in
      let suggest_most_similar lid =
        (* Ignore fake locals *)
        let all_locals =
          LID.Map.fold
            (fun k v acc ->
              if denotable_local k then
                (k, v) :: acc
              else
                acc)
            ctx.LEnvC.local_types
            []
        in
        let var_name (k, _) = LID.to_string k in
        match most_similar lid all_locals var_name with
        | Some (k, (_, pos, _)) -> Some (LID.to_string k, pos)
        | None -> None
      in
      Errors.undefined p lid (suggest_most_similar lid)
    | None -> ()
  in
  match ctx_opt with
  | None ->
    (* If the continuation is absent, we are in dead code so the variable should
    have type nothing. *)
    Some (Typing_make_type.nothing Reason.Rnone, Pos.none, 0)
  | Some ctx ->
    let lcl = LID.Map.find_opt x ctx.LEnvC.local_types in
    begin
      match lcl with
      | None ->
        if not_found_is_ok x ctx then
          ()
        else
          error_if_pos_provided p ctx
      | Some _ -> ()
    end;
    lcl

let get_local_ty_in_ctx env ?error_if_undef_at_pos x ctx_opt =
  match get_local_in_ctx env ?error_if_undef_at_pos x ctx_opt with
  | None -> (false, mk (Reason.Rnone, tany env), Pos.none)
  | Some (x, pos, _) -> (true, x, pos)

let get_local_in_next_continuation ?error_if_undef_at_pos:p env x =
  let next_cont = next_cont_opt env in
  get_local_ty_in_ctx env ?error_if_undef_at_pos:p x next_cont

let get_local_ ?error_if_undef_at_pos:p env x =
  let (mut, ty, _) =
    get_local_in_next_continuation ?error_if_undef_at_pos:p env x
  in
  (mut, ty)

let get_local env x = snd (get_local_ env x)

let get_local_pos env x =
  let (_, ty, pos) = get_local_in_next_continuation env x in
  (ty, pos)

let get_locals env plids =
  let next_cont = next_cont_opt env in
  List.fold plids ~init:LID.Map.empty ~f:(fun locals (p, lid) ->
      match get_local_in_ctx env ~error_if_undef_at_pos:p lid next_cont with
      | None -> locals
      | Some ty_eid -> LID.Map.add lid ty_eid locals)

let set_locals env locals =
  LID.Map.fold (fun lid ty env -> set_local_ env lid ty) locals env

let is_local_defined env x =
  let next_cont = next_cont_opt env in
  Option.is_some next_cont && fst (get_local_ env x)

let get_local_check_defined env (p, x) =
  snd (get_local_ ~error_if_undef_at_pos:p env x)

let set_local_expr_id env x new_eid =
  let per_cont_env = env.lenv.per_cont_env in
  match LEnvC.get_cont_option C.Next per_cont_env with
  | None -> env
  | Some next_cont ->
    begin
      match LID.Map.find_opt x next_cont.LEnvC.local_types with
      | Some (type_, pos, eid)
        when not (Typing_local_types.equal_expression_id eid new_eid) ->
        if LID.is_immutable eid then Errors.immutable_local pos;
        let local = (type_, pos, new_eid) in
        let per_cont_env = LEnvC.add_to_cont C.Next x local per_cont_env in
        let env = { env with lenv = { env.lenv with per_cont_env } } in
        env
      | _ -> env
    end

let get_local_expr_id env x =
  match next_cont_opt env with
  | None -> (* dead code *) None
  | Some next_cont ->
    let lcl = LID.Map.find_opt x next_cont.LEnvC.local_types in
    Option.map lcl ~f:(fun (_, _, x) -> x)

let set_fake_members env fake_members =
  let per_cont_env =
    LEnvC.update_cont_entry C.Next env.lenv.per_cont_env (fun entry ->
        { entry with LEnvC.fake_members })
  in
  { env with lenv = { env.lenv with per_cont_env } }

let get_fake_members env =
  match LEnvC.get_cont_option C.Next env.lenv.per_cont_env with
  | None -> Fake.empty
  | Some next_cont -> next_cont.LEnvC.fake_members

let update_lost_info name blame env ty =
  let info r = Reason.Rlost_info (name, r, blame) in
  let rec update_ty (env, seen_tyvars) ty =
    let (env, ty) = expand_type env ty in
    match deref ty with
    | (_, Tvar v) ->
      if ISet.mem v seen_tyvars then
        ((env, seen_tyvars), ty)
      else
        let seen_tyvars = ISet.add v seen_tyvars in
        let bs = get_tyvar_lower_bounds env v in
        let ((env, seen_tyvars), bs) =
          ITySet.fold_map bs ~init:(env, seen_tyvars) ~f:update_ty_i
        in
        let env = set_tyvar_lower_bounds env v bs in
        let bs = get_tyvar_upper_bounds env v in
        let ((env, seen_tyvars), bs) =
          ITySet.fold_map bs ~init:(env, seen_tyvars) ~f:update_ty_i
        in
        let env = set_tyvar_upper_bounds env v bs in
        ((env, seen_tyvars), ty)
    | (r, Toption ty) ->
      let ((env, seen_tyvars), ty) = update_ty (env, seen_tyvars) ty in
      ((env, seen_tyvars), mk (info r, Toption ty))
    | (r, Tunion tyl) ->
      let ((env, seen_tyvars), tyl) =
        List.fold_map tyl ~init:(env, seen_tyvars) ~f:update_ty
      in
      ((env, seen_tyvars), mk (info r, Tunion tyl))
    | _ -> ((env, seen_tyvars), map_reason ty ~f:info)
  and update_ty_i (env, seen_tyvars) ty =
    match ty with
    | LoclType ty ->
      let ((env, seen_tyvars), ty) = update_ty (env, seen_tyvars) ty in
      ((env, seen_tyvars), LoclType ty)
    | ConstraintType ty ->
      let (r, ty) = deref_constraint_type ty in
      ((env, seen_tyvars), ConstraintType (mk_constraint_type (info r, ty)))
  in
  let ((env, _seen_tyvars), ty) = update_ty (env, ISet.empty) ty in
  (env, ty)

let forget_generic forget env blame =
  let fake_members = get_fake_members env in
  let fake_members = forget fake_members blame in
  set_fake_members env fake_members

let forget_members = forget_generic Fake.forget

let forget_prefixed_members env lid =
  forget_generic (fun fake_members -> Fake.forget_prefixed fake_members lid) env

let forget_suffixed_members env suffix =
  forget_generic
    (fun fake_members -> Fake.forget_suffixed fake_members suffix)
    env

module FakeMembers = struct
  let update_fake_members env fake_members =
    let per_cont_env =
      LEnvC.update_cont_entry C.Next env.lenv.per_cont_env (fun entry ->
          LEnvC.{ entry with fake_members })
    in
    { env with lenv = { env.lenv with per_cont_env } }

  let is_valid env obj member_name =
    match obj with
    | (_, This)
    | (_, Lvar _) ->
      let fake_members = get_fake_members env in
      let id = Fake.make_id obj member_name in
      Fake.is_valid fake_members id
    | _ -> false

  let is_valid_static env cid member_name =
    let name = Fake.make_static_id cid member_name in
    let fake_members = get_fake_members env in
    Fake.is_valid fake_members name

  let check_static_invalid env cid member_name ty =
    let fake_members = get_fake_members env in
    let fake_id = Fake.make_static_id cid member_name in
    match Fake.is_invalid fake_members fake_id with
    | None -> (env, ty)
    | Some blame -> update_lost_info (Local_id.to_string fake_id) blame env ty

  let check_instance_invalid env obj member_name ty =
    match obj with
    | (_, This)
    | (_, Lvar _) ->
      let fake_members = get_fake_members env in
      let fake_id = Fake.make_id obj member_name in
      begin
        match Fake.is_invalid fake_members fake_id with
        | None -> (env, ty)
        | Some blame ->
          update_lost_info (Local_id.to_string fake_id) blame env ty
      end
    | _ -> (env, ty)

  let add_member env fake_id pos =
    let fake_members = get_fake_members env in
    let fake_members = Fake.add fake_members fake_id pos in
    set_fake_members env fake_members

  let make env obj_name member_name pos =
    let my_fake_local_id = Fake.make_id obj_name member_name in
    let env = add_member env my_fake_local_id pos in
    (env, my_fake_local_id)

  let make_static env class_name member_name pos =
    let my_fake_local_id = Fake.make_static_id class_name member_name in
    let env = add_member env my_fake_local_id pos in
    (env, my_fake_local_id)
end

(*****************************************************************************)
(* Sets up/cleans up the environment when typing anonymous function / lambda *)
(*****************************************************************************)

let closure env f =
  (* Remember parts of the environment specific to the enclosing function
     that will be overwritten when typing a lambda *)
  let old_lenv = env.lenv in
  let old_return = get_return env in
  let old_params = get_params env in
  let outer_fun_kind = get_fn_kind env in
  (* Typing *)
  let (env, ret) = f env in
  (* Restore the environment fields that were clobbered *)
  let env = { env with lenv = old_lenv } in
  let env = set_params env old_params in
  let env = set_return env old_return in
  let env = set_fn_kind env outer_fun_kind in
  (env, ret)

let in_try env f =
  let old_in_try = env.in_try in
  let env = { env with in_try = true } in
  let (env, result) = f env in
  ({ env with in_try = old_in_try }, result)

let in_case env f =
  let old_in_case = env.in_case in
  let env = { env with in_case = true } in
  let (env, result) = f env in
  ({ env with in_case = old_in_case }, result)

(* Return the subset of env which is saved in the Typed AST's EnvAnnotation. *)
let save local_tpenv env =
  {
    Tast.tcopt = get_tcopt env;
    Tast.inference_env = env.inference_env;
    Tast.tpenv = TPEnv.union local_tpenv env.tpenv;
    Tast.condition_types = env.genv.condition_types;
    Tast.pessimize = env.pessimize;
    Tast.fun_tast_info = env.fun_tast_info;
  }

(* Compute the type variables appearing covariantly (positively)
 * resp. contravariantly (negatively) in a given type ty.
 * Return a pair of sets of positive and negative type variables
 * (as well as an updated environment).
 *)
let rec get_tyvars env (ty : locl_ty) = get_tyvars_i env (LoclType ty)

and get_tyvars_i env (ty : internal_type) =
  let get_tyvars_union (env, acc_positive, acc_negative) ty =
    let (env, positive, negative) = get_tyvars env ty in
    (env, ISet.union acc_positive positive, ISet.union acc_negative negative)
  in
  let get_tyvars_param (env, acc_positive, acc_negative) fp =
    let (env, positive, negative) = get_tyvars env fp.fp_type.et_type in
    match get_fp_mode fp with
    (* Parameters are treated contravariantly *)
    | FPnormal ->
      (env, ISet.union negative acc_positive, ISet.union positive acc_negative)
    (* Inout parameters are both co- and contra-variant *)
    | FPinout ->
      let tyvars = ISet.union negative positive in
      (env, ISet.union tyvars acc_positive, ISet.union tyvars acc_negative)
  in
  let (env, ety) = expand_internal_type env ty in
  match ety with
  | LoclType ety ->
    (match get_node ety with
    | Tvar v -> (env, ISet.singleton v, ISet.empty)
    | Tany _
    | Tnonnull
    | Terr
    | Tdynamic
    | Tobject
    | Tprim _
    | Tneg _ ->
      (env, ISet.empty, ISet.empty)
    | Toption ty -> get_tyvars env ty
    | Ttuple tyl
    | Tunion tyl
    | Tintersection tyl ->
      List.fold_left tyl ~init:(env, ISet.empty, ISet.empty) ~f:get_tyvars_union
    | Tshape (_, m) ->
      TShapeMap.fold
        (fun _ { sft_ty; _ } res -> get_tyvars_union res sft_ty)
        m
        (env, ISet.empty, ISet.empty)
    | Tfun ft ->
      let (env, params_positive, params_negative) =
        match ft.ft_arity with
        | Fstandard -> (env, ISet.empty, ISet.empty)
        | Fvariadic fp -> get_tyvars_param (env, ISet.empty, ISet.empty) fp
      in
      let (env, params_positive, params_negative) =
        List.fold_left
          ft.ft_params
          ~init:(env, params_positive, params_negative)
          ~f:get_tyvars_param
      in
      let (env, ret_positive, ret_negative) =
        get_tyvars env ft.ft_ret.et_type
      in
      ( env,
        ISet.union ret_positive params_positive,
        ISet.union ret_negative params_negative )
    | Tnewtype (name, tyl, _) ->
      if List.is_empty tyl then
        (env, ISet.empty, ISet.empty)
      else begin
        match get_typedef env name with
        | Some { td_tparams; _ } ->
          let variancel = List.map td_tparams ~f:(fun t -> t.tp_variance) in
          get_tyvars_variance_list (env, ISet.empty, ISet.empty) variancel tyl
        | None -> (env, ISet.empty, ISet.empty)
      end
    | Tdependent (_, ty) -> get_tyvars env ty
    | Tgeneric (_, tyl) ->
      (* TODO(T69931993) Once implementing variance support for HK types, query
        tyvar env here for list of variances *)
      let variancel =
        List.replicate ~num:(List.length tyl) Ast_defs.Invariant
      in
      get_tyvars_variance_list (env, ISet.empty, ISet.empty) variancel tyl
    | Tclass ((_, cid), _, tyl) ->
      if List.is_empty tyl then
        (env, ISet.empty, ISet.empty)
      else begin
        match get_class env cid with
        | Some cls ->
          let variancel =
            List.map (Cls.tparams cls) ~f:(fun t -> t.tp_variance)
          in
          get_tyvars_variance_list (env, ISet.empty, ISet.empty) variancel tyl
        | None -> (env, ISet.empty, ISet.empty)
      end
    | Tvarray ty -> get_tyvars env ty
    | Tdarray (ty1, ty2) ->
      let (env, positive1, negative1) = get_tyvars env ty1 in
      let (env, positive2, negative2) = get_tyvars env ty2 in
      (env, ISet.union positive1 positive2, ISet.union negative1 negative2)
    | Tvec_or_dict (ty1, ty2)
    | Tvarray_or_darray (ty1, ty2) ->
      let (env, positive1, negative1) = get_tyvars env ty1 in
      let (env, positive2, negative2) = get_tyvars env ty2 in
      (env, ISet.union positive1 positive2, ISet.union negative1 negative2)
    | Tunapplied_alias _ -> (env, ISet.empty, ISet.empty)
    | Taccess (ty, _ids) -> get_tyvars env ty)
  | ConstraintType ty ->
    (match deref_constraint_type ty with
    | (_, Tdestructure { d_required; d_optional; d_variadic; d_kind = _ }) ->
      let (env, positive1, negative1) =
        List.fold_left
          d_required
          ~init:(env, ISet.empty, ISet.empty)
          ~f:get_tyvars_union
      in
      let (env, positive2, negative2) =
        List.fold_left
          d_optional
          ~init:(env, ISet.empty, ISet.empty)
          ~f:get_tyvars_union
      in
      let (env, positive3, negative3) =
        match d_variadic with
        | Some ty -> get_tyvars env ty
        | None -> (env, ISet.empty, ISet.empty)
      in
      ( env,
        ISet.union (ISet.union positive1 positive2) positive3,
        ISet.union (ISet.union negative1 negative2) negative3 )
    | (_, Thas_member hm) ->
      let { hm_type; hm_name = _; hm_class_id = _; hm_explicit_targs = _ } =
        hm
      in
      get_tyvars env hm_type
    | (_, TCunion (lty, cty))
    | (_, TCintersection (lty, cty)) ->
      let (env, positive1, negative1) = get_tyvars env lty in
      let (env, positive2, negative2) = get_tyvars_i env (ConstraintType cty) in
      (env, ISet.union positive1 positive2, ISet.union negative1 negative2))

and get_tyvars_variance_list (env, acc_positive, acc_negative) variancel tyl =
  match (variancel, tyl) with
  | (variance :: variancel, ty :: tyl) ->
    let (env, positive, negative) = get_tyvars env ty in
    let (acc_positive, acc_negative) =
      match variance with
      | Ast_defs.Covariant ->
        (ISet.union acc_positive positive, ISet.union acc_negative negative)
      | Ast_defs.Contravariant ->
        (ISet.union acc_positive negative, ISet.union acc_negative positive)
      | Ast_defs.Invariant ->
        let positive_or_negative = ISet.union positive negative in
        ( ISet.union acc_positive positive_or_negative,
          ISet.union acc_negative positive_or_negative )
    in
    get_tyvars_variance_list (env, acc_positive, acc_negative) variancel tyl
  | _ -> (env, acc_positive, acc_negative)

let rec set_tyvar_appears_covariantly_and_propagate env var =
  if get_tyvar_appears_covariantly env var then
    env
  else
    let env = set_tyvar_appears_covariantly env var in
    let lower_bounds = get_tyvar_lower_bounds env var in
    update_variance_of_tyvars_occurring_in_lower_bounds env lower_bounds

and set_tyvar_appears_contravariantly_and_propagate env var =
  if get_tyvar_appears_contravariantly env var then
    env
  else
    let env = set_tyvar_appears_contravariantly env var in
    let upper_bounds = get_tyvar_upper_bounds env var in
    update_variance_of_tyvars_occurring_in_upper_bounds env upper_bounds

and update_variance_of_tyvars_occurring_in_lower_bounds env tys =
  ITySet.fold
    (fun ty env -> update_variance_of_tyvars_occurring_in_lower_bound env ty)
    tys
    env

and update_variance_of_tyvars_occurring_in_upper_bounds env tys =
  ITySet.fold
    (fun ty env -> update_variance_of_tyvars_occurring_in_upper_bound env ty)
    tys
    env

and update_variance_of_tyvars_occurring_in_lower_bound env ty =
  let (env, ety) = expand_internal_type env ty in
  match ety with
  | LoclType ty when is_tyvar ty -> env
  | _ ->
    let (env, positive, negative) = get_tyvars_i env ty in
    let env =
      ISet.fold
        (fun var env -> set_tyvar_appears_covariantly env var)
        positive
        env
    in
    let env =
      ISet.fold
        (fun var env -> set_tyvar_appears_contravariantly env var)
        negative
        env
    in
    env

and update_variance_of_tyvars_occurring_in_upper_bound env ty =
  let (env, ety) = expand_internal_type env ty in
  match ety with
  | LoclType ty when is_tyvar ty -> env
  | _ ->
    let (env, positive, negative) = get_tyvars_i env ty in
    let env =
      ISet.fold
        (fun var env -> set_tyvar_appears_contravariantly env var)
        positive
        env
    in
    let env =
      ISet.fold
        (fun var env -> set_tyvar_appears_covariantly env var)
        negative
        env
    in
    env

let set_tyvar_appears_covariantly = set_tyvar_appears_covariantly_and_propagate

let set_tyvar_appears_contravariantly =
  set_tyvar_appears_contravariantly_and_propagate

(* After a type variable var has been "solved", or bound to a type ty, we need
 * to update the variance of type variables occurring in ty. Suppose that
 * variable var is marked "appears covariantly", i.e. it appears (at least) in
 * positive positions in the type of an expression. Then when we substitute ty
 * for var, variables that appear positively in ty must now be marked as
 * appearing covariantly; variables that appear negatively in ty must now be
 * marked as appearing contravariantly. And the dual, if the variable var is marked
 * "appears contravariantly".
 *)
let update_variance_after_bind env var ty =
  let appears_contravariantly = get_tyvar_appears_contravariantly env var in
  let appears_covariantly = get_tyvar_appears_covariantly env var in
  let (env, positive, negative) = get_tyvars env ty in
  let env =
    ISet.fold
      (fun var env ->
        let env =
          if appears_contravariantly then
            set_tyvar_appears_contravariantly env var
          else
            env
        in
        if appears_covariantly then
          set_tyvar_appears_covariantly env var
        else
          env)
      positive
      env
  in
  let env =
    ISet.fold
      (fun var env ->
        let env =
          if appears_contravariantly then
            set_tyvar_appears_covariantly env var
          else
            env
        in
        if appears_covariantly then
          set_tyvar_appears_contravariantly env var
        else
          env)
      negative
      env
  in
  env

let set_tyvar_variance_i env ?(flip = false) ?(for_all_vars = false) ty =
  log_env_change "set_tyvar_variance" env
  @@
  let (env, positive, negative) = get_tyvars_i env ty in
  let (positive, negative) =
    if flip then
      (negative, positive)
    else
      (positive, negative)
  in
  let tyvars =
    if for_all_vars then
      ISet.union positive negative |> ISet.elements
    else
      get_current_tyvars env
  in
  List.fold_left tyvars ~init:env ~f:(fun env var ->
      let env =
        if ISet.mem var positive then
          set_tyvar_appears_covariantly env var
        else
          env
      in
      let env =
        if ISet.mem var negative then
          set_tyvar_appears_contravariantly env var
        else
          env
      in
      env)

let set_tyvar_variance env ?(flip = false) ?(for_all_vars = false) ty =
  set_tyvar_variance_i env ~flip ~for_all_vars (LoclType ty)

let add_tyvar_upper_bound ?intersect env var (ty : internal_type) =
  log_env_change "add_tyvar_upper_bound" env
  @@ wrap_inference_env_call_env env (fun env ->
         Inf.add_tyvar_upper_bound ?intersect env var ty)

(* Add a single new upper bound [ty] to type variable [var] in [env.inference_env].
 * If the optional [intersect] operation is supplied, then use this to avoid
 * adding redundant bounds by merging the type with existing bounds. This makes
 * sense because a conjunction of upper bounds
 *   (v <: t1) /\ ... /\ (v <: tn)
 * is equivalent to a single upper bound
 *   v <: (t1 & ... & tn)
 *)
let add_tyvar_upper_bound_and_update_variances
    ?intersect env var (ty : internal_type) =
  log_env_change "add_tyvar_upper_bound" env
  @@
  let env = add_tyvar_upper_bound ?intersect env var ty in
  if get_tyvar_appears_contravariantly env var then
    update_variance_of_tyvars_occurring_in_upper_bound env ty
  else
    env

(* Remove type variable `upper_var` from the upper bounds on `var`, if it exists
 *)
let remove_tyvar_upper_bound env var upper_var =
  log_env_change "remove_tyvar_upper_bound" env
  @@ wrap_inference_env_call_env env (fun env ->
         Inf.remove_tyvar_upper_bound env var upper_var)

(* Remove type variable `lower_var` from the lower bounds on `var`, if it exists
 *)
let remove_tyvar_lower_bound env var lower_var =
  log_env_change "remove_tyvar_lower_bound var" env
  @@ wrap_inference_env_call_env env (fun env ->
         Inf.remove_tyvar_lower_bound env var lower_var)

let add_tyvar_lower_bound ?union env var ty =
  log_env_change "add_tyvar_lower_bound" env
  @@ wrap_inference_env_call_env env (fun env ->
         Inf.add_tyvar_lower_bound ?union env var ty)

(* Add a single new lower bound [ty] to type variable [var] in [env.tvenv].
 * If the optional [union] operation is supplied, then use this to avoid
 * adding redundant bounds by merging the type with existing bounds. This makes
 * sense because a conjunction of lower bounds
 *   (t1 <: v) /\ ... /\ (tn <: v)
 * is equivalent to a single lower bound
 *   (t1 | ... | tn) <: v
 *)
let add_tyvar_lower_bound_and_update_variances ?union env var ty =
  log_env_change "add_tyvar_lower_bound" env
  @@
  let env = add_tyvar_lower_bound ?union env var ty in
  if get_tyvar_appears_covariantly env var then
    update_variance_of_tyvars_occurring_in_lower_bound env ty
  else
    env

let initialize_tyvar_as_in ~as_in:genv env v =
  log_env_change "initialize_tyvar_as_in" env
  @@ wrap_inference_env_call_env env (fun env ->
         Inf.initialize_tyvar_as_in ~as_in:genv env v)

let copy_tyvar_from_genv_to_env var ~to_:env ~from:genv =
  log_env_change_ "copy_tyvar_from_genv_to_env" env
  @@ wrap_inference_env_call env (fun env ->
         Inf.copy_tyvar_from_genv_to_env var ~to_:env ~from:genv)

let get_all_tyvars env =
  wrap_inference_env_call_res env (fun env -> Inf.get_vars env)

let remove_var env var ~search_in_upper_bounds_of ~search_in_lower_bounds_of =
  log_env_change "remove_var" env
  @@ wrap_inference_env_call_env env (fun env ->
         Inf.remove_var
           env
           var
           ~search_in_upper_bounds_of
           ~search_in_lower_bounds_of)

let unsolve env v =
  wrap_inference_env_call_env env (fun env -> Inf.unsolve env v)

module Log = struct
  (** Convert a type variable from an environment into json *)
  let tyvar_to_json
      (p_locl_ty : locl_ty -> string)
      (p_internal_type : internal_type -> string)
      (env : env)
      (v : Ident.t) =
    wrap_inference_env_call_res env (fun env ->
        Inf.Log.tyvar_to_json p_locl_ty p_internal_type env v)
end
