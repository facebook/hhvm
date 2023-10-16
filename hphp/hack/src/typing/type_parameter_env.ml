(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

open Typing_defs
open Typing_kinding_defs
module SN = Naming_special_names
module TySet = Typing_set

type tparam_name = string

type tparam_bounds = TySet.t

let empty_bounds = TySet.empty

let singleton_bound ty = TySet.singleton ty

type tparam_info = Typing_kinding_defs.kind
[@@deriving hash, show { with_path = false }]

let tparam_info_size tpinfo =
  TySet.cardinal tpinfo.lower_bounds + TySet.cardinal tpinfo.upper_bounds

type t = {
  tparams: (Pos_or_decl.t * tparam_info) SMap.t;
      (** The position indicates where the type parameter was defined.
          It may be Pos.none if the type parameter denotes a fresh type variable
          (i.e., without a source location that defines it) *)
  consistent: bool;
}
[@@deriving hash, show { with_path = false }]

let empty = { tparams = SMap.empty; consistent = true }

let mem name tpenv = SMap.mem name tpenv.tparams

let get_with_pos name tpenv = SMap.find_opt name tpenv.tparams

let get name tpenv = Option.map (get_with_pos name tpenv) ~f:snd

let get_tparams tpenv = tpenv.tparams

let add ~def_pos name tpinfo tpenv =
  { tpenv with tparams = SMap.add name (def_pos, tpinfo) tpenv.tparams }

let union tpenv1 tpenv2 =
  {
    tparams = SMap.union tpenv1.tparams tpenv2.tparams;
    consistent = tpenv1.consistent && tpenv2.consistent;
  }

(* TODO(T70068435): needs to learn about parameters if higher-kinded? *)
let size tpenv =
  SMap.fold
    (fun _ (_, tpinfo) count -> tparam_info_size tpinfo + count)
    tpenv.tparams
    0

let fold f tpenv accu =
  SMap.fold
    (fun name (_, tparam_info) acc -> f name tparam_info acc)
    tpenv.tparams
    accu

let merge_env env tpenv1 tpenv2 ~combine =
  let (env, tparams) =
    match (tpenv1.consistent, tpenv2.consistent) with
    | (false, true) -> (env, tpenv2.tparams)
    | (true, false) -> (env, tpenv1.tparams)
    | _ -> SMap.merge_env env tpenv1.tparams tpenv2.tparams ~combine
  in
  (env, { tparams; consistent = tpenv1.consistent || tpenv2.consistent })

let get_lower_bounds tpenv name tyargs =
  (* TODO(T70068435) For now, anything with tyargs cannot have bounds.
     Eventually, we need to instantiate the parameters with the provided args.
     We must support the case here that the number of provided tyargs does not
     match the number of expected arguments for the named type parameter. In this
     case, chop off superfluous argumnents and fill in Tany for missing ones. *)
  match (get name tpenv, tyargs) with
  | (_, _ :: _)
  | (None, _) ->
    empty_bounds
  | (Some { lower_bounds; _ }, _) -> lower_bounds

let get_upper_bounds tpenv name tyargs =
  (* TODO(T70068435) For now, anything with tyargs cannot have bounds.
     Eventually, we need to instantiate the parameters with the provided args.
     We must support the case here that the number of provided tyargs does not
     match the number of expected arguments for the named type parameter. In this
     case, chop off superfluous argumnents and fill in Tany for missing ones. *)
  match (get name tpenv, tyargs) with
  | (_, _ :: _)
  | (None, _) ->
    empty_bounds
  | (Some { upper_bounds; _ }, _) -> upper_bounds

let get_arity tpenv name =
  match get name tpenv with
  | None -> 0
  | Some { parameters; _ } -> List.length parameters

let get_reified tpenv name =
  match get name tpenv with
  | None -> Aast.Erased
  | Some { reified; _ } -> reified

let get_enforceable tpenv name =
  match get name tpenv with
  | None -> false
  | Some { enforceable; _ } -> enforceable

let get_newable tpenv name =
  match get name tpenv with
  | None -> false
  | Some { newable; _ } -> newable

let get_require_dynamic tpenv name =
  match get name tpenv with
  | None -> false
  | Some { require_dynamic; _ } -> require_dynamic

let get_pos tpenv name =
  match get_with_pos name tpenv with
  | None -> Pos_or_decl.none
  | Some (pos, _) -> pos

let get_tparam_names tpenv = SMap.keys tpenv.tparams

let is_consistent tpenv = tpenv.consistent

let mark_inconsistent tpenv = { tpenv with consistent = false }

(* This assumes that [name] is a nullary generic parameter *)
let rec is_generic_param ~elide_nullable ty name =
  match get_node ty with
  | Tgeneric (name', []) -> String.equal name name'
  | Toption ty when elide_nullable -> is_generic_param ~elide_nullable ty name
  | _ -> false

(* Add a single new upper bound [ty] to generic parameter [name] in [tpenv] *)
let add_upper_bound_ tpenv name ty =
  (* Don't add superfluous T <: T or T <: ?T to environment *)
  if is_generic_param ~elide_nullable:true ty name then
    tpenv
  else
    let (def_pos, tpinfo) =
      match get_with_pos name tpenv with
      | None ->
        ( Pos_or_decl.none,
          {
            lower_bounds = empty_bounds;
            upper_bounds = singleton_bound ty;
            reified = Aast.Erased;
            enforceable = false;
            newable = false;
            require_dynamic = false;
            parameters = [];
          } )
      | Some (pos, tp) ->
        (pos, { tp with upper_bounds = TySet.add ty tp.upper_bounds })
    in
    add ~def_pos name tpinfo tpenv

(* Add a single new lower bound [ty] to generic parameter [name] in [tpenv] *)
let add_lower_bound_ tpenv name ty =
  (* Don't add superfluous T <: T to environment *)
  if is_generic_param ~elide_nullable:false ty name then
    tpenv
  else
    let (def_pos, tpinfo) =
      match get_with_pos name tpenv with
      | None ->
        ( Pos_or_decl.none,
          {
            lower_bounds = singleton_bound ty;
            upper_bounds = empty_bounds;
            reified = Aast.Erased;
            enforceable = false;
            newable = false;
            require_dynamic = false;
            parameters = [];
          } )
      | Some (pos, tp) ->
        (pos, { tp with lower_bounds = TySet.add ty tp.lower_bounds })
    in
    add ~def_pos name tpinfo tpenv

(* Add a single new upper bound [ty] to generic parameter [name].
   * If the optional [intersect] operation is supplied, then use this to avoid
   * adding redundant bounds by merging the type with existing bounds. This makes
   * sense because a conjunction of upper bounds
   *   (T <: t1) /\ ... /\ (T <: tn)
   * is equivalent to a single upper bound
   *   T <: (t1 & ... & tn)
*)
let add_upper_bound ?intersect env_tpenv name ty =
  let tpenv =
    match deref ty with
    | (r, Tgeneric (formal_super, [])) ->
      add_lower_bound_ env_tpenv formal_super (mk (r, Tgeneric (name, [])))
    | (_r, Tgeneric (_formal_super, _tyargs)) ->
      (* TODO(T70068435) Revisit this when implementing bounds on HK generic vars *)
      env_tpenv
    | _ -> env_tpenv
  in
  match intersect with
  | None -> add_upper_bound_ env_tpenv name ty
  | Some intersect ->
    let tyl =
      intersect ty (TySet.elements (get_upper_bounds env_tpenv name []))
    in
    let add_generic ty tys =
      if is_generic_param ~elide_nullable:true ty name then
        tys
      else
        TySet.add ty tys
    in

    let def_pos = get_pos env_tpenv name in
    let upper_bounds = List.fold_right ~init:TySet.empty ~f:add_generic tyl in
    let lower_bounds = get_lower_bounds env_tpenv name [] in
    let reified = get_reified env_tpenv name in
    let enforceable = get_enforceable env_tpenv name in
    let newable = get_newable env_tpenv name in
    let require_dynamic = get_require_dynamic env_tpenv name in
    let parameters = [] in
    add
      ~def_pos
      name
      {
        lower_bounds;
        upper_bounds;
        reified;
        enforceable;
        newable;
        require_dynamic;
        parameters;
      }
      tpenv

(* Add a single new upper lower [ty] to generic parameter [name].
 * If the optional [union] operation is supplied, then use this to avoid
 * adding redundant bounds by merging the type with existing bounds. This makes
 * sense because a conjunction of lower bounds
 *   (t1 <: T) /\ ... /\ (tn <: T)
 * is equivalent to a single lower bound
 *   (t1 | ... | tn) <: T
 *)
let add_lower_bound ?union env_tpenv name ty =
  let tpenv =
    match deref ty with
    | (r, Tgeneric (formal_sub, [])) ->
      add_upper_bound_ env_tpenv formal_sub (mk (r, Tgeneric (name, [])))
    | (_r, Tgeneric (_formal_sub, _tyargs)) ->
      (* TODO(T70068435) Revisit this when implementing bounds on HK generic vars *)
      env_tpenv
    | _ -> env_tpenv
  in
  match union with
  | None -> add_lower_bound_ env_tpenv name ty
  | Some union ->
    let tyl = union ty (TySet.elements (get_lower_bounds env_tpenv name [])) in
    let def_pos = get_pos env_tpenv name in
    let lower_bounds = List.fold_right ~init:TySet.empty ~f:TySet.add tyl in
    let upper_bounds = get_upper_bounds env_tpenv name [] in
    let reified = get_reified env_tpenv name in
    let enforceable = get_enforceable env_tpenv name in
    let newable = get_newable env_tpenv name in
    let require_dynamic = get_require_dynamic env_tpenv name in
    let parameters = [] in
    add
      ~def_pos
      name
      {
        lower_bounds;
        upper_bounds;
        reified;
        enforceable;
        newable;
        require_dynamic;
        parameters;
      }
      tpenv

let remove_upper_bound tpenv name bound =
  match get_with_pos name tpenv with
  | None -> tpenv
  | Some (def_pos, tparam_info) ->
    let bounds = tparam_info.upper_bounds in
    let bounds = TySet.remove bound bounds in
    let tparam_info = { tparam_info with upper_bounds = bounds } in
    add ~def_pos name tparam_info tpenv

let remove_lower_bound tpenv name bound =
  match get_with_pos name tpenv with
  | None -> tpenv
  | Some (def_pos, tparam_info) ->
    let bounds = tparam_info.lower_bounds in
    let bounds = TySet.remove bound bounds in
    let tparam_info = { tparam_info with lower_bounds = bounds } in
    add ~def_pos name tparam_info tpenv

let remove tpenv name =
  (* TODO(T70068435) Revisit this function when implementing bounds on HK generic vars,
     in particular, look at the two Tgeneric below. *)
  let tparam = mk (Typing_reason.Rnone, Tgeneric (name, [])) in
  let lower_bounds = get_lower_bounds tpenv name [] in
  let remove_from_upper_bounds_of ty tpenv =
    match get_node ty with
    | Tgeneric (name, _tyargs) -> remove_upper_bound tpenv name tparam
    | _ -> tpenv
  in
  let tpenv = TySet.fold remove_from_upper_bounds_of lower_bounds tpenv in
  let upper_bounds = get_upper_bounds tpenv name [] in
  let remove_from_lower_bounds_of ty tpenv =
    match get_node ty with
    | Tgeneric (name, _tyargs) -> remove_lower_bound tpenv name tparam
    | _ -> tpenv
  in
  let tpenv = TySet.fold remove_from_lower_bounds_of upper_bounds tpenv in
  { tpenv with tparams = SMap.remove name tpenv.tparams }

(* Add type parameters to environment, initially with no bounds.
 * Existing type parameters with the same name will be overridden. *)
let add_generic_parameters tpenv tparaml =
  let rec make_param_info ast_tparam =
    let { tp_user_attributes; tp_tparams; _ } = ast_tparam in
    let enforceable =
      Attributes.mem SN.UserAttributes.uaEnforceable tp_user_attributes
    in
    let newable =
      Attributes.mem SN.UserAttributes.uaNewable tp_user_attributes
    in
    let require_dynamic =
      Attributes.mem SN.UserAttributes.uaRequireDynamic tp_user_attributes
    in
    let nested_params =
      List.map tp_tparams ~f:(fun tp -> (tp.tp_name, make_param_info tp))
    in

    {
      lower_bounds = empty_bounds;
      upper_bounds = empty_bounds;
      reified = ast_tparam.tp_reified;
      enforceable;
      newable;
      require_dynamic;
      parameters = nested_params;
    }
  in

  let add_top tpenv ast_tparam =
    let (pos, name) = ast_tparam.tp_name in
    add ~def_pos:pos name (make_param_info ast_tparam) tpenv
  in
  List.fold_left tparaml ~f:add_top ~init:tpenv

let get_parameter_names tpi =
  List.map tpi.parameters ~f:(fun (name, _) -> snd name)

let force_lazy_values_tparam_info (info : tparam_info) =
  Typing_kinding_defs.force_lazy_values info

let force_lazy_values (env : t) =
  let { tparams; consistent } = env in
  {
    tparams =
      SMap.map
        (fun (p, info) -> (p, force_lazy_values_tparam_info info))
        tparams;
    consistent;
  }
