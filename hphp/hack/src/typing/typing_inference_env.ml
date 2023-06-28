(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module ITySet = Internal_type_set
module Occ = Typing_tyvar_occurrences
module TL = Typing_logic

exception InconsistentTypeVarState of string

[@@@warning "-32"]

let pp _ _ = Printf.printf "%s\n" "<inference_env>"

[@@@warning "+32"]

type tyvar_constraints = {
  appears_covariantly: bool;
      (** Does this type variable appear covariantly in the type of the expression? *)
  appears_contravariantly: bool;
      (** Does this type variable appear contravariantly in the type of the expression?
          If it appears in an invariant position then both will be true; if it doesn't
          appear at all then both will be false. *)
  lower_bounds: ITySet.t;
  upper_bounds: ITySet.t;
  type_constants:
    (pos_id (* id of the type constant "T", containing its position. *)
    * locl_ty)
    SMap.t;
      (** Map associating a type to each type constant id of this variable.
          Whenever we localize "T1::T" in a constraint, we add a fresh type variable
          indexed by "T" in the type_constants of the type variable representing T1.
          This allows to properly check constraints on "T1::T". *)
}

type solving_info =
  | TVIType of locl_ty  (** when the type variable is bound to a type *)
  | TVIConstraints of tyvar_constraints
      (** when the type variable is still unsolved *)

type tyvar_info = {
  tyvar_pos: Pos.t;
      (** Where was the type variable introduced? (e.g. generic method invocation,
          new object construction) *)
  global_reason: Reason.t option;
  eager_solve_failed: bool;
  solving_info: solving_info;
}

type tvenv = tyvar_info IMap.t

type identifier = int [@@deriving eq]

module Identifier_provider : sig
  val reinitialize : unit -> unit

  val make_identifier : unit -> identifier
end = struct
  let counter : int ref = ref 0

  let reinitialize () =
    counter := 0;
    ()

  let make_identifier () =
    let identifier = !counter in
    counter := !counter + 1;
    identifier
end

type t = {
  tvenv: tvenv;
  tyvars_stack: (Pos.t * identifier list) list;
  subtype_prop: TL.subtype_prop;
  tyvar_occurrences: Typing_tyvar_occurrences.t;
  allow_solve_globals: bool;
}

type global_tyvar_info = {
  tyvar_reason: Reason.t;
  solving_info_g: solving_info;
}

type global_tvenv = global_tyvar_info IMap.t

type t_global = global_tvenv

type t_global_with_pos = Pos.t * t_global

module Log = struct
  open Typing_log_value

  let tyvar_constraints_as_value tvcstr =
    let {
      appears_covariantly;
      appears_contravariantly;
      type_constants;
      lower_bounds;
      upper_bounds;
    } =
      tvcstr
    in
    make_map
      [
        ("appears_covariantly", bool_as_value appears_covariantly);
        ("appears_contravariantly", bool_as_value appears_contravariantly);
        ("lower_bounds", internal_type_set_as_value lower_bounds);
        ("upper_bounds", internal_type_set_as_value upper_bounds);
        ( "type_constants",
          smap_as_value (fun (_, ty) -> locl_type_as_value ty) type_constants );
      ]

  let solving_info_as_value sinfo =
    match sinfo with
    | TVIType ty -> variant_as_value "TVIType" (locl_type_as_value ty)
    | TVIConstraints tvcstr ->
      variant_as_value "TVIConstraints" (tyvar_constraints_as_value tvcstr)

  let tyvar_info_as_value tvinfo =
    let { tyvar_pos; global_reason; eager_solve_failed; solving_info } =
      tvinfo
    in
    make_map
      [
        ("tyvar_pos", pos_as_value tyvar_pos);
        ( "global_reason",
          list_as_value
            (Option.to_list (Option.map ~f:reason_as_value global_reason)) );
        ("eager_solve_failed", bool_as_value eager_solve_failed);
        ("solving_info", solving_info_as_value solving_info);
      ]

  let tvenv_as_value tvenv =
    Map
      (IMap.fold
         (fun i tvinfo m ->
           SMap.add (var_as_string i) (tyvar_info_as_value tvinfo) m)
         tvenv
         SMap.empty)

  let tyvars_stack_as_value tyvars_stack =
    List
      (List.map tyvars_stack ~f:(fun (_p, l) ->
           List (List.map l ~f:(fun i -> Atom (var_as_string i)))))

  let inference_env_as_value env =
    let {
      tvenv;
      tyvars_stack;
      subtype_prop;
      tyvar_occurrences;
      allow_solve_globals;
    } =
      env
    in
    make_map
      [
        ("tvenv", tvenv_as_value tvenv);
        ("tyvars_stack", tyvars_stack_as_value tyvars_stack);
        ("subtype_prop", subtype_prop_as_value subtype_prop);
        ("tyvar_occurrences", Occ.Log.as_value tyvar_occurrences);
        ("allow_solve_globals", bool_as_value allow_solve_globals);
      ]

  let global_tyvar_info_as_value tvinfo =
    let { tyvar_reason; solving_info_g } = tvinfo in
    make_map
      [
        ("tyvar_reason", reason_as_value tyvar_reason);
        ("solving_info", solving_info_as_value solving_info_g);
      ]

  let global_tvenv_as_value genv =
    Map
      (IMap.fold
         (fun i tvinfo m ->
           SMap.add (var_as_string i) (global_tyvar_info_as_value tvinfo) m)
         genv
         SMap.empty)

  let global_inference_env_as_value genv = global_tvenv_as_value genv

  let reason_to_json r =
    let open Hh_json in
    let p = Reason.to_pos r |> Pos_or_decl.unsafe_to_raw_pos in
    let to_n x = JSON_Number (string_of_int x) in
    JSON_Object
      [
        ("reason", JSON_String (Reason.to_constructor_string r));
        ("filename", JSON_String (Relative_path.suffix (Pos.filename p)));
        ("start_line", to_n @@ fst (Pos.line_column p));
        ("start_column", to_n @@ snd (Pos.line_column p));
        ("end_line", to_n @@ fst (Pos.end_line_column p));
        ("end_column", to_n @@ snd (Pos.end_line_column p));
      ]

  let solving_info_to_json
      (p_locl_ty : locl_ty -> string)
      (p_internal_type : internal_type -> string)
      solving_info =
    let open Hh_json in
    let constraints_to_json
        {
          appears_covariantly;
          appears_contravariantly;
          lower_bounds;
          upper_bounds;
          type_constants = _;
        } =
      let bounds_to_json bs =
        ITySet.elements bs
        |> List.map ~f:(fun b -> JSON_String (p_internal_type b))
      in
      JSON_Object
        [
          ("appears_covariantly", JSON_Bool appears_covariantly);
          ("appears_contravariantly", JSON_Bool appears_contravariantly);
          ("lower_bounds", JSON_Array (bounds_to_json lower_bounds));
          ("upper_bounds", JSON_Array (bounds_to_json upper_bounds));
        ]
    in
    let locl_ty_to_json x = JSON_String (p_locl_ty x) in
    match solving_info with
    | TVIType locl_ty -> JSON_Object [("type", locl_ty_to_json locl_ty)]
    | TVIConstraints cstr ->
      JSON_Object [("constraints", constraints_to_json cstr)]

  let tyvar_to_json_g
      (p_locl_ty : locl_ty -> string)
      (p_internal_type : internal_type -> string)
      (genv : t_global)
      (v : Ident.t) =
    let open Hh_json in
    match IMap.find_opt v genv with
    | None -> JSON_Null
    | Some { tyvar_reason; solving_info_g } ->
      JSON_Object
        [
          ("tyvar_reason", reason_to_json tyvar_reason);
          ( "solving_info",
            solving_info_to_json p_locl_ty p_internal_type solving_info_g );
        ]

  let tyvar_to_json
      (p_locl_ty : locl_ty -> string)
      (p_internal_type : internal_type -> string)
      (env : t)
      (v : identifier) =
    let open Hh_json in
    match IMap.find_opt v env.tvenv with
    | None -> JSON_Null
    | Some { tyvar_pos; global_reason; eager_solve_failed; solving_info } ->
      let global_reason =
        match global_reason with
        | None -> JSON_Null
        | Some global_reason -> reason_to_json global_reason
      in
      JSON_Object
        [
          ("tyvar_pos", string_ @@ Pos.string (Pos.to_absolute tyvar_pos));
          ("global_reason", global_reason);
          ("eager_solve_failed", JSON_Bool eager_solve_failed);
          ( "solving_info",
            solving_info_to_json p_locl_ty p_internal_type solving_info );
        ]
end

let empty_tvenv = IMap.empty

let empty_inference_env =
  {
    tvenv = empty_tvenv;
    tyvars_stack = [];
    subtype_prop = TL.valid;
    tyvar_occurrences = Occ.init;
    allow_solve_globals = false;
  }

let empty_tyvar_constraints =
  {
    lower_bounds = ITySet.empty;
    upper_bounds = ITySet.empty;
    appears_covariantly = false;
    appears_contravariantly = false;
    type_constants = SMap.empty;
  }

let empty_tyvar_info pos =
  {
    tyvar_pos = pos;
    global_reason = None;
    eager_solve_failed = false;
    solving_info = TVIConstraints empty_tyvar_constraints;
  }

let get_allow_solve_globals env = env.allow_solve_globals

let set_allow_solve_globals env flag = { env with allow_solve_globals = flag }

let get_tyvar_info_opt env v = IMap.find_opt v env.tvenv

let set_tyvar_info env v tvinfo =
  { env with tvenv = IMap.add v tvinfo env.tvenv }

let get_solving_info_opt env var =
  match get_tyvar_info_opt env var with
  | None -> None
  | Some tvinfo -> Some tvinfo.solving_info

let set_solving_info env ?(tyvar_pos = Pos.none) x solving_info =
  let tvinfo =
    Option.value
      (get_tyvar_info_opt env x)
      ~default:(empty_tyvar_info tyvar_pos)
  in
  let tvinfo = { tvinfo with solving_info } in
  set_tyvar_info env x tvinfo

let tyvar_occurs_in_tyvar env = Occ.occurs_in env.tyvar_occurrences

let get_tyvar_occurrences env = Occ.get_tyvar_occurrences env.tyvar_occurrences

let get_tyvars_in_tyvar env = Occ.get_tyvars_in_tyvar env.tyvar_occurrences

let contains_unsolved_tyvars env =
  Occ.contains_unsolved_tyvars env.tyvar_occurrences

let get_global_tyvar_reason env v =
  match get_tyvar_info_opt env v with
  | None -> None
  | Some tvinfo -> tvinfo.global_reason

let is_global_tyvar env v = Option.is_some (get_global_tyvar_reason env v)

let tyvar_is_solved env var =
  match get_solving_info_opt env var with
  | None -> false
  | Some sinfo ->
    (match sinfo with
    | TVIConstraints _ -> false
    | TVIType _ -> true)

let tyvar_is_solved_or_skip_global env var =
  tyvar_is_solved env var
  || ((not (get_allow_solve_globals env)) && is_global_tyvar env var)

(** Get type variables in a type that are either unsolved or
solved to a type that itself contains unsolved type variables. *)
let get_unsolved_vars_in_ty env ty =
  let gatherer =
    object
      inherit [ISet.t] Type_visitor.locl_type_visitor

      method! on_tvar vars _r v =
        (* Add it if it has unsolved type vars or if it is itself unsolved. *)
        if
          (not (tyvar_is_solved env v))
          || Occ.contains_unsolved_tyvars env.tyvar_occurrences v
        then
          ISet.add v vars
        else
          vars
    end
  in
  gatherer#on_type ISet.empty ty

let make_tyvars_occur_in_tyvar env vars ~occur_in:x =
  {
    env with
    tyvar_occurrences =
      Occ.make_tyvars_occur_in_tyvar env.tyvar_occurrences vars ~occur_in:x;
  }

let make_tyvar_no_more_occur_in_tyvar env v ~no_more_in:v' =
  {
    env with
    tyvar_occurrences =
      Occ.make_tyvar_no_more_occur_in_tyvar
        env.tyvar_occurrences
        v
        ~no_more_in:v';
  }

let get_direct_binding env v =
  match get_solving_info_opt env v with
  | None
  | Some (TVIConstraints _) ->
    None
  | Some (TVIType ty) -> Some ty

let bind env ?(tyvar_pos = Pos.none) v ty =
  set_solving_info env ~tyvar_pos v (TVIType ty)

let update_tyvar_occurrences env v ty =
  let env =
    { env with tyvar_occurrences = Occ.unbind_tyvar env.tyvar_occurrences v }
  in
  let unsolved_vars_in_ty = get_unsolved_vars_in_ty env ty in
  let env = make_tyvars_occur_in_tyvar env unsolved_vars_in_ty ~occur_in:v in
  env

let add env ?(tyvar_pos = Pos.none) v ty =
  let env = bind env ~tyvar_pos v ty in
  let env = update_tyvar_occurrences env v ty in
  env

let get_type env r v =
  let rec get r v aliases =
    let shorten_paths () =
      ISet.fold (fun v' env -> add env v' (mk (r, Tvar v))) aliases env
    in
    match get_solving_info_opt env v with
    | Some (TVIType ty) -> begin
      match deref ty with
      | (r, Tvar v') ->
        if ISet.mem v aliases then
          raise
          @@ InconsistentTypeVarState
               "Two type variables are aliasing each other!";
        get r v' (ISet.add v aliases)
      | _ ->
        let env = shorten_paths () in
        (env, ty)
    end
    | None
    | Some (TVIConstraints _) ->
      let env = shorten_paths () in
      (env, mk (r, Tvar v))
  in
  get r v ISet.empty

let create_tyvar_constraints variance =
  let (appears_covariantly, appears_contravariantly) =
    match variance with
    | Some Ast_defs.Invariant -> (true, true)
    | Some Ast_defs.Covariant -> (true, false)
    | Some Ast_defs.Contravariant -> (false, true)
    | None -> (false, false)
  in
  let tyvar_constraints =
    {
      empty_tyvar_constraints with
      appears_covariantly;
      appears_contravariantly;
    }
  in
  TVIConstraints tyvar_constraints

let fresh_unsolved_tyvar env v ?variance ~global_reason tyvar_pos =
  let solving_info = create_tyvar_constraints variance in
  let tvinfo =
    { tyvar_pos; global_reason; solving_info; eager_solve_failed = false }
  in
  set_tyvar_info env v tvinfo

let add_current_tyvar ?variance env p v =
  let env = fresh_unsolved_tyvar env v ?variance ~global_reason:None p in
  match env.tyvars_stack with
  | (expr_pos, tyvars) :: rest ->
    { env with tyvars_stack = (expr_pos, v :: tyvars) :: rest }
  | _ -> env

let fresh_type_reason ?variance env p r =
  let v = Identifier_provider.make_identifier () in
  let env = add_current_tyvar ?variance env p v in
  (env, mk (r, Tvar v))

let fresh_type ?variance env p =
  fresh_type_reason env p (Reason.Rtype_variable p) ?variance

let fresh_type_invariant = fresh_type ~variance:Ast_defs.Invariant

let wrap_ty_in_var env r ty =
  let v = Identifier_provider.make_identifier () in
  let tvinfo =
    {
      tyvar_pos = Reason.to_pos r |> Pos_or_decl.unsafe_to_raw_pos;
      global_reason = None;
      eager_solve_failed = false;
      solving_info = TVIType ty;
    }
  in
  let env = set_tyvar_info env v tvinfo in
  let env = update_tyvar_occurrences env v ty in
  (env, mk (r, Tvar v))

let open_tyvars env p = { env with tyvars_stack = (p, []) :: env.tyvars_stack }

let close_tyvars env =
  match env.tyvars_stack with
  | [] -> raise @@ InconsistentTypeVarState "close_tyvars: empty stack"
  | _ :: rest -> { env with tyvars_stack = rest }

let get_current_tyvars env =
  match env.tyvars_stack with
  | [] -> []
  | (_, tyvars) :: _ -> tyvars

let get_tyvar_constraints_opt env var =
  match get_solving_info_opt env var with
  | Some (TVIConstraints constraints) -> Some constraints
  | None
  | Some (TVIType _) ->
    None

let get_tyvar_constraints_exn env var =
  match get_solving_info_opt env var with
  | None ->
    raise
    @@ InconsistentTypeVarState
         (Printf.sprintf
            "Attempting to get constraints for non-existing type variable #%d."
            var)
  | Some (TVIType _) ->
    raise
    @@ InconsistentTypeVarState
         (Printf.sprintf
            "Attempting to get constraints for already solved type variable #%d."
            var)
  | Some (TVIConstraints constraints) -> constraints

let set_tyvar_constraints env v tyvar_constraints =
  set_solving_info env v (TVIConstraints tyvar_constraints)

let get_tyvar_eager_solve_fail env v =
  match get_tyvar_info_opt env v with
  | None -> false
  | Some tvinfo -> tvinfo.eager_solve_failed

let expand_var env r v =
  let (env, ty) = get_type env r v in
  if get_tyvar_eager_solve_fail env v then
    (env, mk (Reason.Rsolve_fail (Reason.to_pos r), get_node ty))
  else
    (env, ty)

let expand_type env x =
  match deref x with
  | (r, Tvar x) -> expand_var env r x
  | _ -> (env, x)

let full_expander =
  object (this)
    inherit [t] Type_mapper_generic.internal_type_mapper

    method! on_tvar env r v =
      let (env, ty) = expand_var env r v in
      match get_node ty with
      | Tvar _ -> (env, ty)
      | _ -> this#on_type env ty
  end

let fully_expand_type env ty = full_expander#on_type env ty

let expand_internal_type env ty =
  match ty with
  | ConstraintType _ -> (env, ty)
  | LoclType ty ->
    let (env, ty) = expand_type env ty in
    (env, LoclType ty)

let get_tyvar_pos env var =
  match get_tyvar_info_opt env var with
  | None -> Pos.none
  | Some tvinfo -> tvinfo.tyvar_pos

let get_tyvar_lower_bounds_opt env v =
  Option.map (get_tyvar_constraints_opt env v) ~f:(fun x -> x.lower_bounds)

let get_tyvar_upper_bounds_opt env v =
  Option.map (get_tyvar_constraints_opt env v) ~f:(fun x -> x.upper_bounds)

let get_tyvar_lower_bounds env var : ITySet.t =
  match get_solving_info_opt env var with
  | None -> ITySet.empty
  | Some solving_info ->
    (match solving_info with
    | TVIType ty -> ITySet.singleton (LoclType ty)
    | TVIConstraints cstr -> cstr.lower_bounds)

let get_tyvar_upper_bounds env var : ITySet.t =
  match get_solving_info_opt env var with
  | None -> ITySet.empty
  | Some solving_info ->
    (match solving_info with
    | TVIType ty -> ITySet.singleton (LoclType ty)
    | TVIConstraints cstr -> cstr.upper_bounds)

let set_tyvar_lower_bounds env var lower_bounds =
  let tyvar_constraints = get_tyvar_constraints_exn env var in
  let tyvar_constraints = { tyvar_constraints with lower_bounds } in
  let env = set_tyvar_constraints env var tyvar_constraints in
  env

let set_tyvar_upper_bounds env var upper_bounds =
  let tyvar_constraints = get_tyvar_constraints_exn env var in
  let tyvar_constraints = { tyvar_constraints with upper_bounds } in
  let env = set_tyvar_constraints env var tyvar_constraints in
  env

let set_tyvar_eager_solve_fail env v =
  match get_tyvar_info_opt env v with
  | None -> env
  | Some tvinfo ->
    let tvinfo = { tvinfo with eager_solve_failed = true } in
    set_tyvar_info env v tvinfo

let get_tyvar_appears_covariantly env var =
  match get_tyvar_constraints_opt env var with
  | Some cstr -> cstr.appears_covariantly
  | None -> false

let get_tyvar_appears_contravariantly env var =
  match get_tyvar_constraints_opt env var with
  | Some cstr -> cstr.appears_contravariantly
  | None -> false

let get_tyvar_appears_invariantly env var =
  get_tyvar_appears_covariantly env var
  && get_tyvar_appears_contravariantly env var

let set_tyvar_appears_covariantly env v =
  let tyvar_constraints = get_tyvar_constraints_exn env v in
  let tyvar_constraints =
    { tyvar_constraints with appears_covariantly = true }
  in
  let env = set_tyvar_constraints env v tyvar_constraints in
  env

let set_tyvar_appears_contravariantly env v =
  let tyvar_constraints = get_tyvar_constraints_exn env v in
  let tyvar_constraints =
    { tyvar_constraints with appears_contravariantly = true }
  in
  let env = set_tyvar_constraints env v tyvar_constraints in
  env

let get_tyvar_type_consts env var =
  match get_tyvar_constraints_opt env var with
  | Some cstr -> cstr.type_constants
  | None -> SMap.empty

let get_tyvar_type_const env var (_, tyconstid) =
  SMap.find_opt tyconstid (get_tyvar_type_consts env var)

let set_tyvar_type_const env var ((_, tyconstid_) as tyconstid) ty =
  let tvinfo = get_tyvar_constraints_exn env var in
  let type_constants =
    SMap.add tyconstid_ (tyconstid, ty) tvinfo.type_constants
  in
  let env = set_tyvar_constraints env var { tvinfo with type_constants } in
  (* We don't want to solve such type variables too early, because of valid
   * code patterns like that one:
   *
   * abstract class A { abstract type const T; }
   *
   * function f<TA as A, T>(TA $_, T $_) : ... where T = TA::T
   *
   * We force it covariantly until we can implement a proper fix to
   * avoid eager solving (to the upperbound) as it would bind TA to
   * an abstract class
   *)
  let env = set_tyvar_appears_covariantly env var in
  env

(** Conjoin a subtype proposition onto the subtype_prop in the environment *)
let add_subtype_prop env prop =
  { env with subtype_prop = TL.conj env.subtype_prop prop }

let get_current_pos_from_tyvar_stack env =
  match env.tyvars_stack with
  | (p, _) :: _ -> Some p
  | _ -> None

(* Add a single new upper bound [ty] to type variable [var] in [env.tvenv].
 * If the optional [intersect] operation is supplied, then use this to avoid
 * adding redundant bounds by merging the type with existing bounds. This makes
 * sense because a conjunction of upper bounds
 *   (v <: t1) /\ ... /\ (v <: tn)
 * is equivalent to a single upper bound
 *   v <: (t1 & ... & tn)
 *)
let add_tyvar_upper_bound ?intersect env var (ty : internal_type) =
  let tvconstraints = get_tyvar_constraints_exn env var in
  let upper_bounds =
    match intersect with
    | None -> ITySet.add ty tvconstraints.upper_bounds
    | Some intersect ->
      ITySet.of_list (intersect ty (ITySet.elements tvconstraints.upper_bounds))
  in
  let env = set_tyvar_constraints env var { tvconstraints with upper_bounds } in
  env

(* Add a single new lower bound [ty] to type variable [var] in [env.tvenv].
 * If the optional [union] operation is supplied, then use this to avoid
 * adding redundant bounds by merging the type with existing bounds. This makes
 * sense because a conjunction of lower bounds
 *   (t1 <: v) /\ ... /\ (tn <: v)
 * is equivalent to a single lower bound
 *   (t1 | ... | tn) <: v
 *)
let add_tyvar_lower_bound ?union env var ty =
  let tvconstraints = get_tyvar_constraints_exn env var in
  let lower_bounds =
    match union with
    | None -> ITySet.add ty tvconstraints.lower_bounds
    | Some union ->
      ITySet.of_list (union ty (ITySet.elements tvconstraints.lower_bounds))
  in
  let env = set_tyvar_constraints env var { tvconstraints with lower_bounds } in
  env

(* Remove type variable `upper_var` from the upper bounds on `var`, if it exists
 *)
let remove_tyvar_upper_bound env var upper_var =
  match get_tyvar_constraints_opt env var with
  | None -> env
  | Some tvconstraints ->
    let upper_bounds =
      ITySet.filter
        (fun ty ->
          let (_env, ty) = expand_internal_type env ty in
          not @@ InternalType.is_var_v ty ~v:upper_var)
        tvconstraints.upper_bounds
    in
    set_tyvar_constraints env var { tvconstraints with upper_bounds }

(* Remove type variable `lower_var` from the lower bounds on `var`, if it exists
 *)
let remove_tyvar_lower_bound env var lower_var =
  match get_tyvar_constraints_opt env var with
  | None -> env
  | Some tvconstraints ->
    let lower_bounds =
      ITySet.filter
        (fun ty ->
          let (_env, ty) = expand_internal_type env ty in
          not @@ InternalType.is_var_v ty ~v:lower_var)
        tvconstraints.lower_bounds
    in
    set_tyvar_constraints env var { tvconstraints with lower_bounds }

let global_tyvar_info_to_dummy_tyvar_info gtvinfo =
  let { solving_info_g; tyvar_reason } = gtvinfo in
  {
    global_reason = Some tyvar_reason;
    eager_solve_failed = false;
    solving_info = solving_info_g;
    tyvar_pos = Reason.to_pos tyvar_reason |> Pos_or_decl.unsafe_to_raw_pos;
  }

let get_vars (env : t) = IMap.keys env.tvenv

let is_empty_g (genv : t_global) = IMap.is_empty genv

let get_vars_g (genv : t_global) = IMap.keys genv

let get_unsolved_vars (env : t) : identifier list =
  IMap.fold
    (fun v tvinfo vars ->
      match tvinfo.solving_info with
      | TVIConstraints _ -> v :: vars
      | TVIType _ -> vars)
    env.tvenv
    []

let get_tyvar_info_exn_g (genv : t_global) v = IMap.find v genv

let get_tyvar_reason_exn_g (genv : t_global) var =
  let tvinfo = get_tyvar_info_exn_g genv var in
  tvinfo.tyvar_reason

let get_tyvar_pos_exn_g (genv : t_global) var =
  Reason.to_pos (get_tyvar_reason_exn_g genv var)

let has_var env v = Option.is_some (get_tyvar_info_opt env v)

let initialize_tyvar_as_in ~as_in:(genv : t_global) (env : t) v =
  match get_tyvar_info_opt env v with
  | Some _ -> env
  | None ->
    get_tyvar_info_exn_g genv v
    |> global_tyvar_info_to_dummy_tyvar_info
    |> set_tyvar_info env v

let copy_tyvar_from_genv_to_env v ~to_:(env : t) ~from:(genv : t_global) =
  let tvinfo =
    get_tyvar_info_exn_g genv v |> global_tyvar_info_to_dummy_tyvar_info
  in
  let rec get_tmp v =
    if has_var env v then
      get_tmp (Ident.tmp ())
    else
      v
  in
  let v' = get_tmp v in
  let env = set_tyvar_info env v' tvinfo in
  (env, v')

module Size = struct
  (* This can be useful to debug type which blow up in size *)
  let ty_size env ty =
    let ty_size_visitor =
      object
        inherit [int] Type_visitor.locl_type_visitor as super

        method! on_type acc ty = 1 + super#on_type acc ty

        method! on_tvar acc r v =
          let (_, ty) = expand_var env r v in
          match get_node ty with
          | Tvar v' when equal_identifier v' v -> acc
          | _ -> super#on_type acc ty
      end
    in
    ty_size_visitor#on_type 0 ty

  let rec constraint_type_size env ty =
    let type_size_list env l =
      List.fold ~init:0 ~f:(fun size ty -> size + ty_size env ty) l
    in
    let type_size_option ~f opt = Option.value_map ~default:0 ~f opt in
    match deref_constraint_type ty with
    | (_, Tdestructure d) ->
      let { d_required; d_optional; d_variadic; _ } = d in
      1
      + type_size_list env d_required
      + type_size_list env d_optional
      + type_size_option ~f:(ty_size env) d_variadic
    | (_, Thas_member hm) -> 1 + ty_size env hm.hm_type
    | (_, Thas_type_member htm) ->
      1 + ty_size env htm.htm_lower + ty_size env htm.htm_upper
    | (_, Tcan_index ci) -> 1 + ty_size env ci.ci_val + ty_size env ci.ci_key
    | (_, Tcan_traverse ct) ->
      1 + ty_size env ct.ct_val + type_size_option ~f:(ty_size env) ct.ct_key
    | (_, TCunion (lty, cty))
    | (_, TCintersection (lty, cty)) ->
      1 + ty_size env lty + constraint_type_size env cty

  let internal_type_size env ty =
    match ty with
    | LoclType ty -> ty_size env ty
    | ConstraintType ty -> constraint_type_size env ty

  let internal_type_set_size env tys =
    ITySet.elements tys
    |> List.map ~f:(internal_type_size env)
    |> List.fold ~init:0 ~f:( + )

  let type_constants_size env tconsts =
    SMap.map (fun (_id, ty) -> ty_size env ty) tconsts |> fun m ->
    SMap.fold (fun _ x y -> x + y) m 0

  let solving_info_size env solving_info =
    match solving_info with
    | TVIType ty -> ty_size env ty
    | TVIConstraints tvcstr ->
      let {
        appears_covariantly = _;
        appears_contravariantly = _;
        upper_bounds;
        lower_bounds;
        type_constants;
      } =
        tvcstr
      in
      let ubound_size = internal_type_set_size env upper_bounds in
      let lbound_size = internal_type_set_size env lower_bounds in
      let tconst_size = type_constants_size env type_constants in
      ubound_size + lbound_size + tconst_size

  let tyvar_info_size env tvinfo =
    let {
      tyvar_pos = _;
      solving_info;
      global_reason = _;
      eager_solve_failed = _;
    } =
      tvinfo
    in
    solving_info_size env solving_info

  let inference_env_size env =
    let {
      tvenv;
      subtype_prop = _;
      tyvars_stack = _;
      tyvar_occurrences = _;
      allow_solve_globals = _;
    } =
      env
    in
    IMap.map (tyvar_info_size env) tvenv |> fun m ->
    IMap.fold (fun _ x y -> x + y) m 0

  let global_inference_env_size (genv : t_global) =
    let env = empty_inference_env in
    IMap.map
      (fun tyvar_info -> solving_info_size env tyvar_info.solving_info_g)
      genv
    |> fun m -> IMap.fold (fun _ x y -> x + y) m 0
end

(* The following merging logic is used to gather all the information we have
 * on a global type variable from multiple sub-graphs. It simply unions
 * all the constraints and does not try to do anything clever like
 * keeping the graph transitively closed. *)

let merge_constraints cstr1 cstr2 =
  let {
    lower_bounds = lb1;
    upper_bounds = ub1;
    type_constants = tc1;
    appears_covariantly = cov1;
    appears_contravariantly = contra1;
  } =
    cstr1
  in
  let {
    lower_bounds = lb2;
    upper_bounds = ub2;
    type_constants = tc2;
    appears_covariantly = cov2;
    appears_contravariantly = contra2;
  } =
    cstr2
  in
  {
    lower_bounds = ITySet.union lb1 lb2;
    upper_bounds = ITySet.union ub1 ub2;
    appears_covariantly = cov1 || cov2;
    appears_contravariantly = contra1 || contra2;
    type_constants =
      (* Type constants must already have been made equivalent, so picking any should be fine *)
      (* TODO: that might actually not be true during initial merging, but let's fix that later. *)
      SMap.union tc1 tc2;
  }

let solving_info_as_constraints sinfo =
  match sinfo with
  | TVIConstraints c -> c
  | TVIType ty ->
    {
      lower_bounds = ITySet.singleton (LoclType ty);
      upper_bounds = ITySet.singleton (LoclType ty);
      appears_covariantly = false;
      appears_contravariantly = false;
      type_constants = SMap.empty;
    }

let merge_solving_infos sinfo1 sinfo2 =
  let cstr1 = solving_info_as_constraints sinfo1 in
  let cstr2 = solving_info_as_constraints sinfo2 in
  let cstr = merge_constraints cstr1 cstr2 in
  TVIConstraints cstr

let merge_tyvar_infos tvinfo1 tvinfo2 =
  let {
    tyvar_pos = pos1;
    global_reason = gl1;
    eager_solve_failed = esf1;
    solving_info = sinfo1;
  } =
    tvinfo1
  in
  let {
    tyvar_pos = pos2;
    global_reason = gl2;
    eager_solve_failed = esf2;
    solving_info = sinfo2;
  } =
    tvinfo2
  in
  let global_reason =
    match (gl1, gl2) with
    | (Some r1, Some r2) ->
      if Reason.is_none r1 then
        Some r2
      else
        Some r1
    | (Some r, None)
    | (None, Some r) ->
      Some r
    | (None, None) -> None
  in
  {
    tyvar_pos =
      (if Pos.equal pos1 Pos.none then
        pos2
      else
        pos1);
    global_reason;
    eager_solve_failed = esf1 || esf2;
    solving_info = merge_solving_infos sinfo1 sinfo2;
  }

let merge_tyvars env v1 v2 =
  if equal_identifier v1 v2 then
    env
  else
    let tvenv = env.tvenv in
    let tvinfo1 = IMap.find v1 env.tvenv in
    let tvinfo2 = IMap.find v2 env.tvenv in
    let tvinfo = merge_tyvar_infos tvinfo1 tvinfo2 in
    let tvenv = IMap.add v2 tvinfo tvenv in
    let env = { env with tvenv } in
    let env = add env v1 (mk (Reason.none, Tvar v2)) in
    let env = remove_tyvar_lower_bound env v2 v2 in
    let env = remove_tyvar_upper_bound env v2 v2 in
    env

let move_tyvar_from_genv_to_env v ~to_:(env : t) ~from:(genv : t_global) =
  let tvinfo1 =
    get_tyvar_info_exn_g genv v |> global_tyvar_info_to_dummy_tyvar_info
  in
  let tvenv = env.tvenv in
  let tvinfo =
    match IMap.find_opt v tvenv with
    | None -> tvinfo1
    | Some tvinfo2 -> merge_tyvar_infos tvinfo1 tvinfo2
  in
  let tvenv = IMap.add v tvinfo tvenv in
  let env = { env with tvenv } in
  env

let simple_merge env1 env2 =
  let {
    tvenv = tvenv1;
    subtype_prop = subtype_prop1;
    tyvars_stack = tyvars_stack1;
    tyvar_occurrences = tyvar_occurrences1;
    allow_solve_globals = allow_solve_globals1;
  } =
    env1
  in
  let {
    tvenv = tvenv2;
    subtype_prop = _;
    tyvars_stack = _;
    tyvar_occurrences = _;
    allow_solve_globals = _;
  } =
    env2
  in
  let tvenv =
    IMap.merge
      (fun _v tvinfo1 tvinfo2 ->
        match (tvinfo1, tvinfo2) with
        | (None, None) -> None
        | (Some tvinfo, None)
        | (None, Some tvinfo) ->
          Some tvinfo
        | (Some tvinfo1, Some tvinfo2) ->
          let {
            tyvar_pos = tyvar_pos1;
            global_reason = global_reason1;
            eager_solve_failed = eager_solve_failed1;
            solving_info = sinfo1;
          } =
            tvinfo1
          in
          let {
            tyvar_pos = tyvar_pos2;
            global_reason = global_reason2;
            eager_solve_failed = eager_solve_failed2;
            solving_info = sinfo2;
          } =
            tvinfo2
          in
          let tvinfo =
            {
              tyvar_pos =
                (if Pos.equal tyvar_pos1 Pos.none then
                  tyvar_pos2
                else
                  tyvar_pos1);
              global_reason =
                (if Option.is_some global_reason1 then
                  global_reason1
                else
                  global_reason2);
              eager_solve_failed = eager_solve_failed1 || eager_solve_failed2;
              solving_info =
                (match (sinfo1, sinfo2) with
                | (TVIConstraints _, TVIType ty)
                | (TVIType ty, TVIConstraints _) ->
                  TVIType ty
                | (TVIConstraints _, TVIConstraints _)
                | (TVIType _, TVIType _) ->
                  sinfo1);
            }
          in
          Some tvinfo)
      tvenv1
      tvenv2
  in
  {
    tvenv;
    subtype_prop = subtype_prop1;
    tyvars_stack = tyvars_stack1;
    tyvar_occurrences = tyvar_occurrences1;
    allow_solve_globals = allow_solve_globals1;
  }

let get_nongraph_subtype_prop env = env.subtype_prop

let is_alias_for_another_var env v =
  match get_solving_info_opt env v with
  | Some (TVIType ty) -> begin
    match get_node ty with
    | Tvar v' when Int.( <> ) v v' -> true
    | _ -> false
  end
  | _ -> false

(** Some ty vars in the map will carry no additional information, e.g.
some ty vars belong to methods declared in parent classes, even
when these methods are not used in the subclass itself. In those cases,
the ty var will be registered, but the accompagnying ty var info
will contain nothing useful. It will in essence be an identity element
under the merge operation. *)
let solving_info_carries_information = function
  | TVIType _ -> true
  | TVIConstraints tvcstr ->
    let {
      appears_covariantly;
      appears_contravariantly;
      upper_bounds;
      lower_bounds;
      type_constants;
    } =
      tvcstr
    in
    appears_contravariantly
    || appears_covariantly
    || (not @@ ITySet.is_empty upper_bounds)
    || (not @@ ITySet.is_empty lower_bounds)
    || (not @@ SMap.is_empty type_constants)

let tyvar_info_carries_information tvinfo =
  let { tyvar_pos = _; solving_info; global_reason = _; eager_solve_failed = _ }
      =
    tvinfo
  in
  solving_info_carries_information solving_info

let global_tyvar_info_carries_information tvinfo =
  let { tyvar_reason = _; solving_info_g } = tvinfo in
  solving_info_carries_information solving_info_g

let compress env =
  let {
    tvenv;
    subtype_prop;
    tyvars_stack;
    tyvar_occurrences;
    allow_solve_globals;
  } =
    env
  in
  let tvenv = IMap.filter (fun _k -> tyvar_info_carries_information) tvenv in
  { tvenv; subtype_prop; tyvars_stack; tyvar_occurrences; allow_solve_globals }

let compress_g genv =
  IMap.filter (fun _ -> global_tyvar_info_carries_information) genv

let construct_undirected_tyvar_graph genvs (additional_edges : ISet.t list) :
    (Pos.t * global_tyvar_info) list IMap.t * ISet.t IMap.t =
  (* gather all constraints per type variable *)
  let all_constraints : (Pos.t * global_tyvar_info) list IMap.t =
    List.fold genvs ~init:IMap.empty ~f:(fun all_constraints (pos, genv) ->
        IMap.fold
          (fun tyvar tyvar_info ->
            IMap.update tyvar @@ function
            | None -> Some [(pos, tyvar_info)]
            | Some tl -> Some ((pos, tyvar_info) :: tl))
          genv
          all_constraints)
  in
  (* create an undirected graph of type variables where an edge
   * u <~> v means u <: v or v <: u.
   * the graph is a mapping from a tyvar to other tyvars (a collection of
   * directed edges).
   *)
  let add_edge ~(from : identifier) ~(to_ : identifier) (graph : ISet.t IMap.t)
      =
    IMap.update
      from
      (fun neighbors ->
        let neighbors = Option.value neighbors ~default:ISet.empty in
        Some (ISet.add to_ neighbors))
      graph
  in
  let add_edges ~(from : identifier) ~(to_ : ISet.t) (graph : ISet.t IMap.t) =
    IMap.update
      from
      (fun neighbors ->
        let neighbors = Option.value neighbors ~default:ISet.empty in
        Some (ISet.union to_ neighbors))
      graph
  in
  let add_vertex (vertex : identifier) (graph : ISet.t IMap.t) =
    IMap.update
      vertex
      (function
        | Some s -> Some s
        | None -> Some ISet.empty)
      graph
  in
  let walk tyvar ty graph =
    let graph = add_vertex tyvar graph in
    match ty with
    | ConstraintType _ -> graph
    | LoclType ty ->
      let walker =
        object
          inherit [ISet.t IMap.t] Type_mapper_generic.internal_type_mapper

          method! on_tvar graph r tyvar' =
            let graph =
              graph
              |> add_edge ~from:tyvar ~to_:tyvar'
              |> add_edge ~from:tyvar' ~to_:tyvar
            in
            (graph, mk (r, Tvar tyvar'))
        end
      in
      fst @@ walker#on_type graph ty
  in
  let graph : ISet.t IMap.t =
    IMap.fold
      (fun tyvar infos edges ->
        List.fold infos ~init:edges ~f:(fun edges (_, tyvar_info) ->
            match tyvar_info.solving_info_g with
            | TVIType ty -> walk tyvar (LoclType ty) edges
            | TVIConstraints cstrs ->
              ITySet.fold (walk tyvar) cstrs.lower_bounds edges
              |> ITySet.fold (walk tyvar) cstrs.upper_bounds))
      all_constraints
      IMap.empty
  in
  let graph =
    List.fold
      additional_edges
      ~init:graph
      ~f:(fun graph vars_which_must_be_in_same_cc ->
        ISet.fold
          (fun v graph ->
            let new_edges = ISet.remove v vars_which_must_be_in_same_cc in
            add_edges ~from:v ~to_:new_edges graph)
          vars_which_must_be_in_same_cc
          graph)
  in
  (all_constraints, graph)

let split_undirected_tyvar_graph (graph : ISet.t IMap.t) : ISet.t list =
  (* compute connected components by doing DFS *)
  let visited : ISet.t ref = ref ISet.empty in
  let rec dfs (component : ISet.t) tyvar : ISet.t =
    let component = ISet.add tyvar component in
    visited := ISet.add tyvar !visited;
    ISet.fold
      (fun neighbor component ->
        if not (ISet.mem neighbor !visited) then
          dfs component neighbor
        else
          component)
      (IMap.find tyvar graph)
      component
  in
  let components : ISet.t list =
    IMap.fold
      (fun tyvar _ components ->
        if not (ISet.mem tyvar !visited) then
          let c = dfs ISet.empty tyvar in
          c :: components
        else
          components)
      graph
      []
  in
  (* sanity check: no type variables should be lost *)
  (* TODO: (hverr) T60273306 Normally the following should hold as well:
   *       sum(len(c for c in components)) == len(all)
   *       unfortunately it doesn't because somehow non-global (?)
   *       variables appear in lower/upper bounds of some global
   *       variables, for which we have no information whatsoever
   *       (e.g. position), where do these tyvars come from?
   *)
  let () =
    assert (
      equal_identifier
        (IMap.cardinal graph)
        (List.fold components ~init:0 ~f:(fun acc c -> acc + ISet.cardinal c)))
  in
  (* sanity check: check whether components are really disconnected *)
  let (_ : ISet.t) =
    List.fold components ~init:ISet.empty ~f:(fun c set ->
        ISet.fold
          (fun var set ->
            if ISet.mem var set then
              failwith "variable is in two connected components, impossible"
            else
              ISet.add var set)
          c
          set)
  in
  components

let connected_components_g genvs ~additional_edges =
  let (nodes, graph) =
    construct_undirected_tyvar_graph genvs additional_edges
  in
  let component_tyvars = split_undirected_tyvar_graph graph in
  (* collect tyvar information *)
  let components : (Pos.t * global_tyvar_info) list IMap.t list =
    List.map component_tyvars ~f:(fun vars ->
        ISet.fold
          (fun var ->
            IMap.update var @@ function
            | Some _ -> failwith "programming error, expected unique variables"
            | None -> IMap.find_opt var nodes)
          vars
          IMap.empty)
  in
  (* convert each component to a list of gobal environments *)
  let component_to_subgraph
      (component : (Pos.t * global_tyvar_info) list IMap.t) :
      t_global_with_pos list =
    IMap.fold
      (fun tyvar infos all ->
        let infos =
          List.map infos ~f:(fun (pos, tyvar_info) ->
              (pos, IMap.singleton tyvar tyvar_info))
        in
        infos @ all)
      component
      []
  in
  let components = List.map components ~f:component_to_subgraph in
  List.zip_exn component_tyvars components

let remove_var_from_bounds
    env v ~search_in_upper_bounds_of ~search_in_lower_bounds_of =
  let env =
    ISet.fold
      (fun v' env ->
        Option.fold
          (get_tyvar_upper_bounds_opt env v')
          ~init:env
          ~f:(fun env bounds ->
            ITySet.filter (InternalType.is_not_var_v ~v) bounds
            |> set_tyvar_upper_bounds env v'))
      search_in_upper_bounds_of
      env
  in
  let env =
    ISet.fold
      (fun v' env ->
        Option.fold
          (get_tyvar_lower_bounds_opt env v')
          ~init:env
          ~f:(fun env bounds ->
            ITySet.filter (InternalType.is_not_var_v ~v) bounds
            |> set_tyvar_lower_bounds env v'))
      search_in_lower_bounds_of
      env
  in
  env

let remove_var_from_tyvars_stack tyvars_stack var =
  List.map tyvars_stack ~f:(fun (p, vars) ->
      (p, List.filter vars ~f:(fun v -> not (equal_identifier var v))))

let replace_var_by_ty_in_prop prop v ty =
  let rec replace prop =
    match prop with
    | TL.IsSubtype (cd, ty1, ty2) ->
      let ty1 =
        if InternalType.is_var_v ty1 ~v then
          LoclType ty
        else
          ty1
      in
      let ty2 =
        if InternalType.is_var_v ty2 ~v then
          LoclType ty
        else
          ty2
      in
      TL.IsSubtype (cd, ty1, ty2)
    | TL.Disj (f, props) ->
      let props = List.map props ~f:replace in
      TL.Disj (f, props)
    | TL.Conj props ->
      let props = List.map props ~f:replace in
      TL.Conj props
  in
  replace prop

let remove_var env var ~search_in_upper_bounds_of ~search_in_lower_bounds_of =
  let env =
    remove_var_from_bounds
      env
      var
      ~search_in_upper_bounds_of
      ~search_in_lower_bounds_of
  in
  let (env, ty) = expand_var env Reason.none var in
  let {
    tvenv;
    tyvars_stack;
    tyvar_occurrences;
    subtype_prop;
    allow_solve_globals;
  } =
    env
  in
  let tyvar_occurrences = Occ.remove_var tyvar_occurrences var in
  let tvenv = IMap.remove var tvenv in
  let tyvars_stack = remove_var_from_tyvars_stack tyvars_stack var in
  let subtype_prop = replace_var_by_ty_in_prop subtype_prop var ty in
  { tvenv; tyvars_stack; tyvar_occurrences; subtype_prop; allow_solve_globals }

let forget_tyvar_g (genv : t_global) var_to_forget =
  let default_mapper_env = Type_mapper_forget.make_env var_to_forget in
  let mapper = new Type_mapper_forget.forget_tyvar_mapper in
  let process_solving_info : solving_info -> solving_info option = function
    | TVIType locl_ty ->
      let locl_ty = mapper#recurse_opt default_mapper_env locl_ty in
      Option.map locl_ty ~f:(fun locl_ty -> TVIType locl_ty)
    | TVIConstraints cstrs ->
      let map_bound ity =
        mapper#recurse_internal_type_opt default_mapper_env ity
      in
      let lower_bounds =
        cstrs.lower_bounds
        |> ITySet.elements
        |> List.filter_map ~f:map_bound
        |> ITySet.of_list
      in
      let upper_bounds =
        cstrs.upper_bounds
        |> ITySet.elements
        |> List.filter_map ~f:map_bound
        |> ITySet.of_list
      in
      let cstrs = { cstrs with lower_bounds; upper_bounds } in
      Some (TVIConstraints cstrs)
  in
  let genv =
    IMap.mapi
      (fun var global_tyvar_info ->
        if equal_identifier var var_to_forget then
          None
        else
          let solving_info_g =
            process_solving_info global_tyvar_info.solving_info_g
          in
          Option.map solving_info_g ~f:(fun solving_info_g ->
              { global_tyvar_info with solving_info_g }))
      genv
  in
  let genv = IMap.filter_opt genv in
  (* sanity check *)
  let () =
    let throw_on_var_to_forget =
      object
        inherit [unit] Type_mapper_generic.internal_type_mapper

        method! on_tvar () r var =
          if equal_identifier var var_to_forget then
            failwith
            @@ Printf.sprintf "encountered %d, should have been removed" var
          else
            ((), mk (r, Tvar var))
      end
    in
    IMap.iter
      (fun _var global_tyvar_info ->
        match global_tyvar_info.solving_info_g with
        | TVIType locl_ty -> fst @@ throw_on_var_to_forget#on_type () locl_ty
        | TVIConstraints cstrs ->
          ITySet.iter
            (fun ity -> fst @@ throw_on_var_to_forget#on_internal_type () ity)
            cstrs.lower_bounds;
          ITySet.iter
            (fun ity -> fst @@ throw_on_var_to_forget#on_internal_type () ity)
            cstrs.upper_bounds)
      genv
  in
  genv

let visit_types_g
    (genv : t_global)
    (mapper : 'a Type_mapper_generic.internal_type_mapper_type)
    (acc : 'a) : 'a =
  IMap.fold
    (fun tyvar tyvar_info acc ->
      let acc = fst @@ mapper#on_tvar acc tyvar_info.tyvar_reason tyvar in
      match tyvar_info.solving_info_g with
      | TVIType ty -> fst @@ mapper#on_type acc ty
      | TVIConstraints cstrs ->
        let process_bounds bounds acc =
          ITySet.fold
            (fun ity vars -> fst @@ mapper#on_internal_type vars ity)
            bounds
            acc
        in
        acc
        |> process_bounds cstrs.lower_bounds
        |> process_bounds cstrs.upper_bounds)
    genv
    acc

(** This is a horrible thing meant to throw a warning when some invariant turns out
    to be wrong (namely a variable should not yet be solved) and to help us get back
    on our feet when this happens. Once we figure out why this invariant is wrong,
    this function should die. *)
let unsolve env v =
  let {
    tvenv;
    tyvars_stack;
    subtype_prop;
    tyvar_occurrences;
    allow_solve_globals;
  } =
    env
  in
  match IMap.find_opt v tvenv with
  | None -> env
  | Some tvinfo ->
    let { solving_info; tyvar_pos; global_reason; eager_solve_failed } =
      tvinfo
    in
    (match solving_info with
    | TVIConstraints _ -> env
    | TVIType ty ->
      let stack = Caml.Printexc.get_callstack 50 in
      Printf.eprintf
        "Did not expect variable #%d to be solved already. Unsolving it.\n"
        v;
      Caml.Printexc.print_raw_backtrace stderr stack;
      Printf.eprintf "%!";
      let solving_info =
        TVIConstraints
          {
            appears_covariantly = false;
            appears_contravariantly = false;
            upper_bounds = ITySet.singleton (LoclType ty);
            lower_bounds = ITySet.singleton (LoclType ty);
            type_constants = SMap.empty;
          }
      in
      let tvinfo =
        { solving_info; tyvar_pos; global_reason; eager_solve_failed }
      in
      let tvenv = IMap.add v tvinfo tvenv in
      {
        tvenv;
        tyvars_stack;
        subtype_prop;
        tyvar_occurrences;
        allow_solve_globals;
      })
