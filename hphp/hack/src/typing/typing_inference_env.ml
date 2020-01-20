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
    ( Aast.sid (* id of the type constant "T", containing its position. *)
    * locl_ty )
    SMap.t;
      (** Map associating a type to each type constant id of this variable.
          Whenever we localize "T1::T" in a constraint, we add a fresh type variable
          indexed by "T" in the type_constants of the type variable representing T1.
          This allows to properly check constraints on "T1::T". *)
  pu_accesses:
    (* base * enum * new_var * typname *)
    (locl_ty * Aast.sid * locl_ty * Aast.sid) SMap.t;
      (** Map associating PU information to each instance of
          C:@E:@#v:@T
          when the type variable #v is not resolved yet. We introduce a new type
          variable to 'postpone' the checking of this expression until the end,
          when #v will be known. *)
}

type solving_info =
  | TVIType of locl_ty  (** when the type variable is bound to a type *)
  | TVIConstraints of tyvar_constraints
      (** when the type variable is still unsolved *)

type tyvar_info = {
  tyvar_pos: Pos.t;
      (** Where was the type variable introduced? (e.g. generic method invocation,
          new object construction) *)
  is_global: bool;
  eager_solve_failed: bool;
  solving_info: solving_info;
}

type tvenv = tyvar_info IMap.t

type t = {
  tvenv: tvenv;
  tyvars_stack: (Pos.t * Ident.t list) list;
  subtype_prop: Typing_logic.subtype_prop;
  tyvar_occurrences: Typing_tyvar_occurrences.t;
}

type global_tyvar_info = {
  tyvar_pos: Pos.t;
  solving_info: solving_info;
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
      pu_accesses;
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
        ( "pu_acceses",
          smap_as_value (fun (_, _, ty, _) -> locl_type_as_value ty) pu_accesses
        );
      ]

  let solving_info_as_value sinfo =
    match sinfo with
    | TVIType ty -> variant_as_value "TVIType" (locl_type_as_value ty)
    | TVIConstraints tvcstr ->
      variant_as_value "TVIConstraints" (tyvar_constraints_as_value tvcstr)

  let tyvar_info_as_value tvinfo =
    let { tyvar_pos; is_global; eager_solve_failed; solving_info } = tvinfo in
    make_map
      [
        ("tyvar_pos", pos_as_value tyvar_pos);
        ("is_global", bool_as_value is_global);
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
      (List.map tyvars_stack (fun (_p, l) ->
           List (List.map l (fun i -> Atom (var_as_string i)))))

  let inference_env_as_value env =
    let { tvenv; tyvars_stack; subtype_prop; tyvar_occurrences } = env in
    make_map
      [
        ("tvenv", tvenv_as_value tvenv);
        ("tyvars_stack", tyvars_stack_as_value tyvars_stack);
        ("subtype_prop", subtype_prop_as_value subtype_prop);
        ("tyvar_occurrences", Occ.Log.as_value tyvar_occurrences);
      ]

  let global_tyvar_info_as_value tvinfo =
    let { tyvar_pos; solving_info } = tvinfo in
    make_map
      [
        ("tyvar_pos", pos_as_value tyvar_pos);
        ("solving_info", solving_info_as_value solving_info);
      ]

  let global_tvenv_as_value genv =
    Map
      (IMap.fold
         (fun i tvinfo m ->
           SMap.add (var_as_string i) (global_tyvar_info_as_value tvinfo) m)
         genv
         SMap.empty)

  let global_inference_env_as_value genv = global_tvenv_as_value genv

  let tyvar_to_json_g
      (p_locl_ty : locl_ty -> string)
      (p_internal_type : internal_type -> string)
      (genv : t_global)
      (v : Ident.t) =
    let open Hh_json in
    let pos_to_json p =
      let to_n x = JSON_Number (string_of_int x) in
      JSON_Object
        [
          ("filename", JSON_String (Relative_path.suffix (Pos.filename p)));
          ("start_line", to_n @@ fst (Pos.line_column p));
          ("start_column", to_n @@ snd (Pos.line_column p));
          ("end_line", to_n @@ fst (Pos.end_line_column p));
          ("end_column", to_n @@ snd (Pos.end_line_column p));
        ]
    in
    let constraints_to_json
        {
          appears_covariantly;
          appears_contravariantly;
          lower_bounds;
          upper_bounds;
          pu_accesses = _;
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
    let solving_info_to_json = function
      | TVIType locl_ty -> JSON_Object [("type", locl_ty_to_json locl_ty)]
      | TVIConstraints cstr ->
        JSON_Object [("constraints", constraints_to_json cstr)]
    in
    match IMap.find_opt v genv with
    | None -> JSON_Null
    | Some { tyvar_pos; solving_info } ->
      JSON_Object
        [
          ("tyvar_pos", pos_to_json tyvar_pos);
          ("solving_info", solving_info_to_json solving_info);
        ]
end

let empty_tvenv = IMap.empty

let empty_inference_env =
  {
    tvenv = empty_tvenv;
    tyvars_stack = [];
    subtype_prop = TL.valid;
    tyvar_occurrences = Occ.init;
  }

let empty_tyvar_constraints =
  {
    lower_bounds = ITySet.empty;
    upper_bounds = ITySet.empty;
    appears_covariantly = false;
    appears_contravariantly = false;
    type_constants = SMap.empty;
    pu_accesses = SMap.empty;
  }

let empty_tyvar_info pos =
  {
    tyvar_pos = pos;
    is_global = false;
    eager_solve_failed = false;
    solving_info = TVIConstraints empty_tyvar_constraints;
  }

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

let tyvar_is_solved env var =
  match get_solving_info_opt env var with
  | None -> false
  | Some sinfo ->
    (match sinfo with
    | TVIConstraints _ -> false
    | TVIType _ -> true)

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

let add env ?(tyvar_pos = Pos.none) v ty =
  let env =
    { env with tyvar_occurrences = Occ.unbind_tyvar env.tyvar_occurrences v }
  in
  let env =
    let unsolved_vars_in_ty = get_unsolved_vars_in_ty env ty in
    make_tyvars_occur_in_tyvar env unsolved_vars_in_ty ~occur_in:v
  in
  set_solving_info env ~tyvar_pos v (TVIType ty)

let get_type env r v =
  let rec get v aliases =
    let shorten_paths () =
      ISet.fold (fun v' env -> add env v' (mk (r, Tvar v))) aliases env
    in
    match get_solving_info_opt env v with
    | Some (TVIType ty) ->
      begin
        match get_node ty with
        | Tvar v' ->
          if ISet.mem v aliases then
            failwith "Two type variables are aliasing each other!";
          get v' (ISet.add v aliases)
        | _ ->
          let env = shorten_paths () in
          (env, ty)
      end
    | None
    | Some (TVIConstraints _) ->
      let env = shorten_paths () in
      (env, mk (r, Tvar v))
  in
  get v ISet.empty

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

let fresh_unsolved_tyvar env v ?variance ~is_global tyvar_pos =
  let solving_info = create_tyvar_constraints variance in
  let tvinfo =
    { tyvar_pos; is_global; solving_info; eager_solve_failed = false }
  in
  set_tyvar_info env v tvinfo

let add_current_tyvar ?variance env p v =
  let env = fresh_unsolved_tyvar env v ?variance ~is_global:false p in
  match env.tyvars_stack with
  | (expr_pos, tyvars) :: rest ->
    { env with tyvars_stack = (expr_pos, v :: tyvars) :: rest }
  | _ -> env

let fresh_type_reason ?variance env r =
  let v = Ident.tmp () in
  let env = add_current_tyvar ?variance env (Reason.to_pos r) v in
  (env, mk (r, Tvar v))

let fresh_type ?variance env p =
  fresh_type_reason env (Reason.Rtype_variable p) ?variance

let fresh_invariant_type_var = fresh_type ~variance:Ast_defs.Invariant

let new_global_tyvar = fresh_unsolved_tyvar ~is_global:true

let is_global_tyvar env v =
  match get_tyvar_info_opt env v with
  | None -> false
  | Some tvinfo -> tvinfo.is_global

let open_tyvars env p = { env with tyvars_stack = (p, []) :: env.tyvars_stack }

let close_tyvars env =
  match env.tyvars_stack with
  | [] -> failwith "close_tyvars: empty stack"
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

let get_tyvar_constraints_or_empty env var =
  match get_tyvar_constraints_opt env var with
  | Some constraints -> constraints
  | None -> empty_tyvar_constraints

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

let fully_expand_type_i env ty = full_expander#on_internal_type env ty

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
  let tyvar_constraints = get_tyvar_constraints_or_empty env var in
  let tyvar_constraints = { tyvar_constraints with lower_bounds } in
  let env = set_tyvar_constraints env var tyvar_constraints in
  env

let set_tyvar_upper_bounds env var upper_bounds =
  let tyvar_constraints = get_tyvar_constraints_or_empty env var in
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
  let tyvar_constraints = get_tyvar_constraints_or_empty env v in
  let tyvar_constraints =
    { tyvar_constraints with appears_covariantly = true }
  in
  let env = set_tyvar_constraints env v tyvar_constraints in
  env

let set_tyvar_appears_contravariantly env v =
  let tyvar_constraints = get_tyvar_constraints_or_empty env v in
  let tyvar_constraints =
    { tyvar_constraints with appears_contravariantly = true }
  in
  let env = set_tyvar_constraints env v tyvar_constraints in
  env

let get_tyvar_type_consts env var =
  match get_tyvar_constraints_opt env var with
  | Some cstr -> cstr.type_constants
  | None -> SMap.empty

let get_tyvar_pu_accesses env var =
  match get_tyvar_constraints_opt env var with
  | Some cstr -> cstr.pu_accesses
  | None -> SMap.empty

let get_tyvar_type_const env var (_, tyconstid) =
  SMap.find_opt tyconstid (get_tyvar_type_consts env var)

let get_tyvar_pu_access env var (_, typ_name) =
  SMap.find_opt typ_name (get_tyvar_pu_accesses env var)

let set_tyvar_type_const env var ((_, tyconstid_) as tyconstid) ty =
  let tvinfo = get_tyvar_constraints_or_empty env var in
  let type_constants =
    SMap.add tyconstid_ (tyconstid, ty) tvinfo.type_constants
  in
  set_tyvar_constraints env var { tvinfo with type_constants }

let set_tyvar_pu_access env var base enum new_var name =
  let tvinfo = get_tyvar_constraints_or_empty env var in
  let pu_accesses =
    SMap.add (snd name) (base, enum, new_var, name) tvinfo.pu_accesses
  in
  set_tyvar_constraints env var { tvinfo with pu_accesses }

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
  let tvconstraints = get_tyvar_constraints_or_empty env var in
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
  let tvconstraints = get_tyvar_constraints_or_empty env var in
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
          match expand_internal_type env ty with
          | (_, LoclType ty) ->
            begin
              match get_node ty with
              | Tvar v -> not (Ident.equal v upper_var)
              | _ -> true
            end
          | _ -> true)
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
          match expand_internal_type env ty with
          | (_, LoclType ty) ->
            begin
              match get_node ty with
              | Tvar v -> not (Ident.equal v lower_var)
              | _ -> true
            end
          | _ -> true)
        tvconstraints.lower_bounds
    in
    set_tyvar_constraints env var { tvconstraints with lower_bounds }

let expand_bounds_of_global_tyvars env (solving_info : solving_info) =
  match solving_info with
  | TVIType ty ->
    let (env, ty) = fully_expand_type env ty in
    (env, TVIType ty)
  | TVIConstraints tvconstraints ->
    let (env, upper_bounds) =
      ITySet.fold_map
        tvconstraints.upper_bounds
        ~init:env
        ~f:fully_expand_type_i
    in
    let (env, lower_bounds) =
      ITySet.fold_map
        tvconstraints.lower_bounds
        ~init:env
        ~f:fully_expand_type_i
    in
    (env, TVIConstraints { tvconstraints with upper_bounds; lower_bounds })

let extract_global_tyvar_info : t -> tyvar_info -> t * global_tyvar_info option
    =
 fun env tvinfo ->
  let { tyvar_pos; is_global; solving_info; eager_solve_failed = _ } = tvinfo in
  if is_global then
    let (env, solving_info) = expand_bounds_of_global_tyvars env solving_info in
    let gtvinfo = { tyvar_pos; solving_info } in
    (env, Some gtvinfo)
  else
    (env, None)

let extract_global_inference_env : t -> t * t_global =
 fun env ->
  let tvenv = env.tvenv in
  let extract_global_tyvar_info x _i y = extract_global_tyvar_info x y in
  let (env, tvenv) = IMap.map_env extract_global_tyvar_info env tvenv in
  let tvenv = IMap.filter_opt tvenv in
  (env, tvenv)

let global_tyvar_info_to_dummy_tyvar_info gtvinfo =
  let { solving_info; tyvar_pos } = gtvinfo in
  { is_global = true; eager_solve_failed = false; solving_info; tyvar_pos }

let get_vars (env : t) = IMap.keys env.tvenv

let get_vars_g (genv : t_global) = IMap.keys genv

let get_tyvar_info_exn_g (genv : t_global) v = IMap.find v genv

let has_var env v = Option.is_some (get_tyvar_info_opt env v)

let copy_tyvar_from_genv_to_env v ~to_:(env : t) ~from:(genv : t_global) =
  let tvinfo =
    get_tyvar_info_exn_g genv v |> global_tyvar_info_to_dummy_tyvar_info
  in
  let v' =
    if has_var env v then
      Ident.tmp ()
    else
      v
  in
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
          | Tvar v' when Ident.equal v' v -> acc
          | _ -> super#on_type acc ty
      end
    in
    ty_size_visitor#on_type 0 ty

  let rec constraint_type_size env ty =
    match deref_constraint_type ty with
    | (_, Tdestructure tyl) ->
      List.fold ~init:1 ~f:(fun size ty -> size + ty_size env ty) tyl
    | (_, Thas_member hm) ->
      1
      +
      let { hm_type = ty; hm_name = _; hm_class_id = _ } = hm in
      ty_size env ty
    | (_, TCunion (lty, cty))
    | (_, TCintersection (lty, cty)) ->
      1 + ty_size env lty + constraint_type_size env cty

  let internal_type_size env ty =
    match ty with
    | LoclType ty -> ty_size env ty
    | ConstraintType ty -> constraint_type_size env ty

  let internal_type_set_size env tys =
    Internal_type_set.elements tys
    |> List.map ~f:(internal_type_size env)
    |> List.fold ~init:0 ~f:( + )

  let type_constants_size env tconsts =
    SMap.map (fun (_id, ty) -> ty_size env ty) tconsts |> fun m ->
    SMap.fold (fun _ x y -> x + y) m 0

  let pu_accesses_size env pu_accesses =
    SMap.map
      (fun (ty1, _id, ty2, _id') -> ty_size env ty1 + ty_size env ty2)
      pu_accesses
    |> fun m -> SMap.fold (fun _ x y -> x + y) m 0

  let tyvar_info_size env tvinfo =
    let { tyvar_pos = _; solving_info; is_global = _; eager_solve_failed = _ } =
      tvinfo
    in
    match solving_info with
    | TVIType ty -> ty_size env ty
    | TVIConstraints tvcstr ->
      let {
        appears_covariantly = _;
        appears_contravariantly = _;
        upper_bounds;
        lower_bounds;
        type_constants;
        pu_accesses;
      } =
        tvcstr
      in
      let ubound_size = internal_type_set_size env upper_bounds in
      let lbound_size = internal_type_set_size env lower_bounds in
      let tconst_size = type_constants_size env type_constants in
      let pu_accesses_size = pu_accesses_size env pu_accesses in
      ubound_size + lbound_size + tconst_size + pu_accesses_size

  let inference_env_size env =
    let { tvenv; subtype_prop = _; tyvars_stack = _; tyvar_occurrences = _ } =
      env
    in
    IMap.map (tyvar_info_size env) tvenv |> fun m ->
    IMap.fold (fun _ x y -> x + y) m 0
end

let simple_merge env1 env2 =
  let {
    tvenv = tvenv1;
    subtype_prop = subtype_prop1;
    tyvars_stack = tyvars_stack1;
    tyvar_occurrences = tyvar_occurrences1;
  } =
    env1
  in
  let {
    tvenv = tvenv2;
    subtype_prop = _;
    tyvars_stack = _;
    tyvar_occurrences = _;
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
            is_global = is_global1;
            eager_solve_failed = eager_solve_failed1;
            solving_info = sinfo1;
          } =
            tvinfo1
          in
          let {
            tyvar_pos = tyvar_pos2;
            is_global = is_global2;
            eager_solve_failed = eager_solve_failed2;
            solving_info = sinfo2;
          } =
            tvinfo2
          in
          let tvinfo =
            {
              tyvar_pos =
                ( if Pos.equal tyvar_pos1 Pos.none then
                  tyvar_pos2
                else
                  tyvar_pos1 );
              is_global = is_global1 || is_global2;
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
  }

let get_nongraph_subtype_prop env = env.subtype_prop

let is_alias_for_another_var env v =
  match get_solving_info_opt env v with
  | Some (TVIType ty) ->
    begin
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
      pu_accesses;
    } =
      tvcstr
    in
    appears_contravariantly
    || appears_covariantly
    || (not @@ ITySet.is_empty upper_bounds)
    || (not @@ ITySet.is_empty lower_bounds)
    || (not @@ SMap.is_empty type_constants)
    || (not @@ SMap.is_empty pu_accesses)

let tyvar_info_carries_information tvinfo =
  let { tyvar_pos = _; solving_info; is_global = _; eager_solve_failed = _ } =
    tvinfo
  in
  solving_info_carries_information solving_info

let global_tyvar_info_carries_information tvinfo =
  let { tyvar_pos = _; solving_info } = tvinfo in
  solving_info_carries_information solving_info

let compress env =
  let { tvenv; subtype_prop; tyvars_stack; tyvar_occurrences } = env in
  let tvenv = IMap.filter (fun _k -> tyvar_info_carries_information) tvenv in
  { tvenv; subtype_prop; tyvars_stack; tyvar_occurrences }

let compress_g genv =
  IMap.filter (fun _ -> global_tyvar_info_carries_information) genv

let construct_undirected_tyvar_graph genvs :
    (Pos.t * global_tyvar_info) list IMap.t * ISet.t IMap.t =
  (* gather all constraints per type variable *)
  let all : (Pos.t * global_tyvar_info) list IMap.t =
    List.fold genvs ~init:IMap.empty ~f:(fun all (pos, genv) ->
        IMap.fold
          (fun tyvar tyvar_info ->
            IMap.update tyvar @@ function
            | None -> Some [(pos, tyvar_info)]
            | Some tl -> Some ((pos, tyvar_info) :: tl))
          genv
          all)
  in
  (* create an undirected graph of type variables where an edge
   * u <~> v means u <: v or v <: u.
   * the graph is a mapping from a tyvar to other tyvars (a collection of
   * directed edges).
   *)
  let add_edge ~(from : Ident.t) ~(to_ : Ident.t) (graph : ISet.t IMap.t) =
    IMap.update
      from
      (fun neighbors ->
        let neighbors = Option.value neighbors ~default:ISet.empty in
        Some (ISet.add to_ neighbors))
      graph
  in
  let walk tyvar ty graph =
    match ty with
    | ConstraintType _ -> graph
    | LoclType ty ->
      let walker =
        object
          inherit [_] Type_visitor.locl_type_visitor

          method! on_tvar graph _ tyvar' =
            add_edge ~from:tyvar ~to_:tyvar' graph
            |> add_edge ~from:tyvar' ~to_:tyvar
        end
      in
      walker#on_type graph ty
  in
  let graph : ISet.t IMap.t =
    IMap.fold
      (fun tyvar infos edges ->
        List.fold infos ~init:edges ~f:(fun edges (_, tyvar_info) ->
            match tyvar_info.solving_info with
            | TVIType ty -> walk tyvar (LoclType ty) edges
            | TVIConstraints cstrs ->
              ITySet.fold (walk tyvar) cstrs.lower_bounds edges
              |> ITySet.fold (walk tyvar) cstrs.upper_bounds))
      all
      IMap.empty
  in
  (all, graph)

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
      Int.equal
        (IMap.cardinal graph)
        (List.fold components ~init:0 ~f:(fun acc c -> acc + ISet.cardinal c))
    )
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

let connected_components_g genvs =
  let (nodes, graph) = construct_undirected_tyvar_graph genvs in
  let components = split_undirected_tyvar_graph graph in
  (* collect tyvar information *)
  let components : (Pos.t * global_tyvar_info) list IMap.t list =
    List.map components ~f:(fun vars ->
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
  List.map components ~f:component_to_subgraph
