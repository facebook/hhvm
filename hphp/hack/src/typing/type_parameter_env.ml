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
module TySet = Typing_set

type tparam_bounds = TySet.t

let empty_bounds = TySet.empty

let singleton_bound ty = TySet.singleton ty

type tparam_info = {
  lower_bounds: tparam_bounds;
  upper_bounds: tparam_bounds;
  reified: Aast.reify_kind;
  enforceable: bool;
  newable: bool;
}

let tparam_info_size tpinfo =
  TySet.cardinal tpinfo.lower_bounds + TySet.cardinal tpinfo.upper_bounds

type t = {
  tparams: tparam_info SMap.t;
  consistent: bool;
}

let empty = { tparams = SMap.empty; consistent = true }

let mem name tpenv = SMap.mem name tpenv.tparams

let get name tpenv = SMap.find_opt name tpenv.tparams

let add name tpinfo tpenv =
  { tpenv with tparams = SMap.add name tpinfo tpenv.tparams }

let union tpenv1 tpenv2 =
  {
    tparams = SMap.union tpenv1.tparams tpenv2.tparams;
    consistent = tpenv1.consistent && tpenv2.consistent;
  }

let size tpenv =
  SMap.fold
    (fun _ tpinfo count -> tparam_info_size tpinfo + count)
    tpenv.tparams
    0

let fold f tpenv accu = SMap.fold f tpenv.tparams accu

let merge_env env tpenv1 tpenv2 ~combine =
  let (env, tparams) =
    SMap.merge_env env tpenv1.tparams tpenv2.tparams ~combine
  in
  (env, { tparams; consistent = tpenv1.consistent && tpenv2.consistent })

let get_lower_bounds tpenv name =
  match get name tpenv with
  | None -> empty_bounds
  | Some { lower_bounds; _ } -> lower_bounds

let get_upper_bounds tpenv name =
  match get name tpenv with
  | None -> empty_bounds
  | Some { upper_bounds; _ } -> upper_bounds

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

let get_names tpenv = SMap.keys tpenv.tparams

let is_consistent tpenv = tpenv.consistent

let mark_inconsistent tpenv = { tpenv with consistent = false }

let rec is_generic_param ~elide_nullable ty name =
  match get_node ty with
  | Tgeneric name' -> String.equal name name'
  | Toption ty when elide_nullable -> is_generic_param ~elide_nullable ty name
  | _ -> false

(* Add a single new upper bound [ty] to generic parameter [name] in [tpenv] *)
let add_upper_bound_ tpenv name ty =
  (* Don't add superfluous T <: T or T <: ?T to environment *)
  if is_generic_param ~elide_nullable:true ty name then
    tpenv
  else
    let tpinfo =
      match get name tpenv with
      | None ->
        {
          lower_bounds = empty_bounds;
          upper_bounds = singleton_bound ty;
          reified = Aast.Erased;
          enforceable = false;
          newable = false;
        }
      | Some tp -> { tp with upper_bounds = TySet.add ty tp.upper_bounds }
    in
    add name tpinfo tpenv

(* Add a single new lower bound [ty] to generic parameter [name] in [tpenv] *)
let add_lower_bound_ tpenv name ty =
  (* Don't add superfluous T <: T to environment *)
  if is_generic_param ~elide_nullable:false ty name then
    tpenv
  else
    let tpinfo =
      match get name tpenv with
      | None ->
        {
          lower_bounds = singleton_bound ty;
          upper_bounds = empty_bounds;
          reified = Aast.Erased;
          enforceable = false;
          newable = false;
        }
      | Some tp -> { tp with lower_bounds = TySet.add ty tp.lower_bounds }
    in
    add name tpinfo tpenv

(* Add a single new upper bound [ty] to generic parameter [name] in the local
  * type parameter environment of [env].
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
    | (r, Tgeneric formal_super) ->
      add_lower_bound_ env_tpenv formal_super (mk (r, Tgeneric name))
    | _ -> env_tpenv
  in
  match intersect with
  | None -> add_upper_bound_ env_tpenv name ty
  | Some intersect ->
    let tyl = intersect ty (TySet.elements (get_upper_bounds env_tpenv name)) in
    let add_generic ty tys =
      if is_generic_param ~elide_nullable:true ty name then
        tys
      else
        TySet.add ty tys
    in
    let upper_bounds = List.fold_right ~init:TySet.empty ~f:add_generic tyl in
    let lower_bounds = get_lower_bounds env_tpenv name in
    let reified = get_reified env_tpenv name in
    let enforceable = get_enforceable env_tpenv name in
    let newable = get_newable env_tpenv name in
    add name { lower_bounds; upper_bounds; reified; enforceable; newable } tpenv

(* Add a single new upper lower [ty] to generic parameter [name] in the
 * local type parameter environment [env].
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
    | (r, Tgeneric formal_sub) ->
      add_upper_bound_ env_tpenv formal_sub (mk (r, Tgeneric name))
    | _ -> env_tpenv
  in
  match union with
  | None -> add_lower_bound_ env_tpenv name ty
  | Some union ->
    let tyl = union ty (TySet.elements (get_lower_bounds env_tpenv name)) in
    let lower_bounds = List.fold_right ~init:TySet.empty ~f:TySet.add tyl in
    let upper_bounds = get_upper_bounds env_tpenv name in
    let reified = get_reified env_tpenv name in
    let enforceable = get_enforceable env_tpenv name in
    let newable = get_newable env_tpenv name in
    add name { lower_bounds; upper_bounds; reified; enforceable; newable } tpenv

let remove_upper_bound tpenv name bound =
  match get name tpenv with
  | None -> tpenv
  | Some tparam_info ->
    let bounds = tparam_info.upper_bounds in
    let bounds = TySet.remove bound bounds in
    let tparam_info = { tparam_info with upper_bounds = bounds } in
    add name tparam_info tpenv

let remove_lower_bound tpenv name bound =
  match get name tpenv with
  | None -> tpenv
  | Some tparam_info ->
    let bounds = tparam_info.lower_bounds in
    let bounds = TySet.remove bound bounds in
    let tparam_info = { tparam_info with lower_bounds = bounds } in
    add name tparam_info tpenv

let remove tpenv name =
  let tparam = mk (Typing_reason.Rnone, Tgeneric name) in
  let lower_bounds = get_lower_bounds tpenv name in
  let remove_from_upper_bounds_of ty tpenv =
    match get_node ty with
    | Tgeneric name -> remove_upper_bound tpenv name tparam
    | _ -> tpenv
  in
  let tpenv = TySet.fold remove_from_upper_bounds_of lower_bounds tpenv in
  let upper_bounds = get_upper_bounds tpenv name in
  let remove_from_lower_bounds_of ty tpenv =
    match get_node ty with
    | Tgeneric name -> remove_lower_bound tpenv name tparam
    | _ -> tpenv
  in
  let tpenv = TySet.fold remove_from_lower_bounds_of upper_bounds tpenv in
  { tpenv with tparams = SMap.remove name tpenv.tparams }

(* Add type parameters to environment, initially with no bounds.
 * Existing type parameters with the same name will be overridden. *)
let add_generic_parameters tpenv tparaml =
  let add_empty_bounds
      tpenv { tp_name = (_, name); tp_reified = reified; tp_user_attributes; _ }
      =
    let enforceable =
      Naming_attributes.mem SN.UserAttributes.uaEnforceable tp_user_attributes
    in
    let newable =
      Naming_attributes.mem SN.UserAttributes.uaNewable tp_user_attributes
    in
    let tpinfo =
      {
        lower_bounds = empty_bounds;
        upper_bounds = empty_bounds;
        reified;
        enforceable;
        newable;
      }
    in
    add name tpinfo tpenv
  in
  List.fold_left tparaml ~f:add_empty_bounds ~init:tpenv

let pp_tparam_info fmt tpi =
  Format.fprintf fmt "@[<hv 2>{ ";

  Format.fprintf fmt "@[%s =@ " "lower_bounds";
  TySet.pp fmt tpi.lower_bounds;
  Format.fprintf fmt "@]";
  Format.fprintf fmt ";@ ";

  Format.fprintf fmt "@[%s =@ " "upper_bounds";
  TySet.pp fmt tpi.upper_bounds;
  Format.fprintf fmt "@]";
  Format.fprintf fmt ";@ ";

  Format.fprintf fmt "@[%s =@ " "reified";
  Aast.pp_reify_kind fmt tpi.reified;
  Format.fprintf fmt "@]";
  Format.fprintf fmt ";@ ";

  Format.fprintf fmt "@[%s =@ " "enforceable";
  Format.pp_print_bool fmt tpi.enforceable;
  Format.fprintf fmt "@]";
  Format.fprintf fmt ";@ ";

  Format.fprintf fmt "@[%s =@ " "newable";
  Format.pp_print_bool fmt tpi.newable;
  Format.fprintf fmt "@]";

  Format.fprintf fmt " }@]"

let pp_tpenv fmt tpenv =
  Format.fprintf fmt "@[<hv 2>{ ";

  Format.fprintf fmt "@[%s =@ " "tparams";
  SMap.pp pp_tparam_info fmt tpenv.tparams;

  Format.fprintf fmt "@[%s =@ " "consistent";
  Format.pp_print_bool fmt tpenv.consistent;

  Format.fprintf fmt " }@]"

let pp = pp_tpenv
