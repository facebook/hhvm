(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
module T = Typing_defs
module Logic = Shape_analysis_logic
module HT = Hips_types

module StaticAccess = struct
  type ty = entity_ * T.TShapeField.t * Typing_defs.locl_ty

  let compare =
    Tuple.T3.compare
      ~cmp1:compare_entity_
      ~cmp2:T.TShapeField.compare
      ~cmp3:Typing_defs.compare_locl_ty

  module Set = struct
    module S = Caml.Set.Make (struct
      type t = ty

      let compare = compare
    end)

    include S
    include CommonSet (S)
  end
end

type constraints = {
  markers: (marker_kind * Pos.t) list;
  base_static_accesses: StaticAccess.Set.t;
  derived_static_accesses: StaticAccess.Set.t;
  optional_accesses: (entity_ * T.TShapeMap.key) list;
  dynamic_accesses: EntitySet.t;
  subsets: (entity_ * entity_) list;
}

let constraints_init =
  {
    markers = [];
    base_static_accesses = StaticAccess.Set.empty;
    derived_static_accesses = StaticAccess.Set.empty;
    optional_accesses = [];
    dynamic_accesses = EntitySet.empty;
    subsets = [];
  }

let rec transitive_closure (set : PointsToSet.t) : PointsToSet.t =
  let immediate_consequence (x, y) set =
    let add (y', z) set =
      if equal_entity_ y y' then
        PointsToSet.add (x, z) set
      else
        set
    in
    PointsToSet.fold add set set
  in
  let new_set = PointsToSet.fold immediate_consequence set set in
  if PointsToSet.cardinal new_set = PointsToSet.cardinal set then
    set
  else
    transitive_closure new_set

let disassemble constraints =
  let partition_constraint constraints = function
    | Marks (kind, entity) ->
      { constraints with markers = (kind, entity) :: constraints.markers }
    | Has_static_key (Base, entity, key, ty) ->
      {
        constraints with
        base_static_accesses =
          StaticAccess.Set.add
            (entity, key, ty)
            constraints.base_static_accesses;
      }
    | Has_static_key (Derived, entity, key, ty) ->
      {
        constraints with
        derived_static_accesses =
          StaticAccess.Set.add
            (entity, key, ty)
            constraints.derived_static_accesses;
      }
    | Has_optional_key (entity, key) ->
      {
        constraints with
        optional_accesses = (entity, key) :: constraints.optional_accesses;
      }
    | Has_dynamic_key entity ->
      {
        constraints with
        dynamic_accesses = EntitySet.add entity constraints.dynamic_accesses;
      }
    | Subsets (sub, sup) ->
      { constraints with subsets = (sub, sup) :: constraints.subsets }
  in
  List.fold ~init:constraints_init ~f:partition_constraint constraints

let assemble
    {
      markers;
      base_static_accesses;
      derived_static_accesses;
      optional_accesses;
      dynamic_accesses;
      subsets;
    } =
  List.map ~f:(fun (kind, entity) -> Marks (kind, entity)) markers
  @ List.map
      ~f:(fun (entity, key, ty) -> Has_static_key (Base, entity, key, ty))
      (StaticAccess.Set.elements base_static_accesses)
  @ List.map
      ~f:(fun (entity, key, ty) -> Has_static_key (Derived, entity, key, ty))
      (StaticAccess.Set.elements derived_static_accesses)
  @ List.map
      ~f:(fun (entity, key) -> Has_optional_key (entity, key))
      optional_accesses
  @ List.map
      ~f:(fun entity -> Has_dynamic_key entity)
      (EntitySet.elements dynamic_accesses)
  @ List.map ~f:(fun (sub, sup) -> Subsets (sub, sup)) subsets

let subset_lookups subsets =
  let update entity entity' =
    EntityMap.update entity (function
        | None -> Some (EntitySet.singleton entity')
        | Some set -> Some (EntitySet.add entity' set))
  in
  let (subset_map, superset_map) =
    let update_maps (e, e') (subset_map, superset_map) =
      let subset_map = update e' e subset_map in
      let superset_map = update e e' superset_map in
      (subset_map, superset_map)
    in
    PointsToSet.fold update_maps subsets (EntityMap.empty, EntityMap.empty)
  in

  (* Generate lookup functions *)
  let collect map entity =
    match EntityMap.find_opt entity map with
    | Some entities -> EntitySet.elements entities
    | None -> []
  in
  (collect subset_map, collect superset_map)

type adjacencies = {
  backwards: EntitySet.t;
  forwards: EntitySet.t;
}

let mk_adjacency_map subsets =
  let entities =
    List.concat_map ~f:(fun (e1, e2) -> [e1; e2]) subsets |> EntitySet.of_list
  in
  let backwards_of e (e1, e2) =
    if equal_entity_ e e2 then
      Some e1
    else
      None
  in
  let forwards_of e (e1, e2) =
    if equal_entity_ e e1 then
      Some e2
    else
      None
  in
  let find_adjacent of_ entity =
    List.filter_map ~f:(of_ entity) subsets |> EntitySet.of_list
  in
  let adjacency e =
    {
      backwards = find_adjacent backwards_of e;
      forwards = find_adjacent forwards_of e;
    }
  in
  let add e = EntityMap.add e (adjacency e) in
  EntitySet.fold add entities EntityMap.empty

(*
  Propagates `Has_static_key` constraints forward through the dataflow graph.
  The implementation is semi-naïve, i.e., at each iteration it only considers
  newly generated facts.

    has_static_key_u(E, Key, Ty) :- has_static_key(E, Key, Ty).
    has_static_key_u(F, Key, Ty) :- has_static_key(E, Key, Ty), subsets(E,F).
*)
let derive_static_accesses
    adjacency_map ~base_static_accesses ~derived_static_accesses =
  let open StaticAccess.Set in
  let rec close_upwards ~delta ~acc =
    if is_empty delta then
      acc
    else
      let propagate (e, k, ty) =
        match EntityMap.find_opt e adjacency_map with
        | Some adjacency ->
          EntitySet.fold (fun e -> add (e, k, ty)) adjacency.forwards empty
        | None -> empty
      in
      let delta = unions_map ~f:propagate delta in
      let delta = diff delta acc in
      let acc = union delta acc in
      close_upwards ~delta ~acc
  in
  let all = union base_static_accesses derived_static_accesses in
  close_upwards ~delta:all ~acc:all

(*
  Close dynamic key access in both directions to later invalidate all results
  that touch it.

    has_dynamic_key_ud(E) :- has_dynamic_key(E).
    has_dynamic_key_ud(F) :- has_dynamic_key_ud(E), (subsets_tr(E,F); subsets_tr(F,E)).
*)
let derive_dynamic_accesses adjacency_map dynamic_accesses =
  let open EntitySet in
  let rec close_upwards_and_downwards ~delta ~acc =
    if is_empty delta then
      acc
    else
      let propagate e =
        match EntityMap.find_opt e adjacency_map with
        | Some adjacency -> union adjacency.forwards adjacency.backwards
        | None -> empty
      in
      let delta = unions_map ~f:propagate delta in
      let delta = diff delta acc in
      let acc = union delta acc in
      close_upwards_and_downwards ~delta ~acc
  in
  close_upwards_and_downwards ~delta:dynamic_accesses ~acc:dynamic_accesses

(* The following program roughly summarises the solver in Datalog.

  Variables with single letters E and F and their variants with primes all
  range over entities.

  Comma (,) is used for conjunction semi-colon (;) is used for disjunction.
  Comma has higher precedence (binds tighter) than semi-colon.

  p :- q1, ..., qn

  means if q1 to qn holds, so does p.

  If the predicate name is `p_suffix`, the `suffix` conveys a property:
    `t` means transitively closed
    `r` means reflexively closed
    `u` means upwards closed by propagating through subsets
    `d` means downwards closed by propagating through subsets

  // Reflexive closure
  subsets_tr(E,E) :-
    subsets(E,_); subsets(_,E);
    has_static_key(E,_,_);
    has_dynamic_key(E,_,_).
  // Transitive closure closure
  subsets_tr(E,F) :- subsets(E,F).
  subsets_tr(E,F) :- subsets(E,F), subsets_re(E,F).

  // We conclude that a key is optional either we explicitly observed it in the
  // source code, for example, through the use of `idx` or due to control flow
  // making the key exist in one branch of execution but not another.
  //
  // TODO(T125888579): Note that this treatment is imprecise as we can later
  // observe the key to be definitely in place, but it would still be marked as
  // optional.
  has_optional_key_base(E,Key) :- has_optional_key(E,Key).
  has_optional_key_base(Join,Key) :-
    subsets(E,Join),
    subsets(F,Join),
    has_static_key_u(E,Key),
    not has_static_key_u(F,Key).
  has_optional_key_u(F,Key) :- has_optional_key_base(E,Key), subsets_tr(E,F).
*)
let deduce (constraints : constraint_ list) : constraint_ list =
  let {
    markers;
    base_static_accesses;
    derived_static_accesses;
    optional_accesses;
    dynamic_accesses;
    subsets;
  } =
    disassemble constraints
  in

  let subsets_reflexive =
    List.map ~f:(fun (_, pos) -> Literal pos) markers
    @ List.map
        (StaticAccess.Set.elements base_static_accesses)
        ~f:(fun (e, _, _) -> e)
    @ List.map
        (StaticAccess.Set.elements derived_static_accesses)
        ~f:(fun (e, _, _) -> e)
    @ EntitySet.elements dynamic_accesses
    @ List.concat_map subsets ~f:(fun (e, e') -> [e; e'])
    |> List.map ~f:(fun e -> (e, e))
  in
  let adjacency_map = mk_adjacency_map subsets in
  let subsets = subsets_reflexive @ subsets in
  let subsets = PointsToSet.of_list subsets |> transitive_closure in
  let (_collect_subsets, collect_supersets) = subset_lookups subsets in
  let subsets = PointsToSet.elements subsets in

  (* Close upwards *)
  let derived_static_accesses =
    derive_static_accesses
      adjacency_map
      ~base_static_accesses
      ~derived_static_accesses
  in
  let static_keys_of e =
    let add_key (e', key, _) map =
      if equal_entity_ e e' then
        T.TShapeSet.add key map
      else
        map
    in
    StaticAccess.Set.fold add_key derived_static_accesses T.TShapeSet.empty
  in

  let optional_accesses =
    let add_optional_key join adjacencies acc =
      let handle_predecessor pred_entity common_keys =
        let keys = static_keys_of pred_entity in
        T.TShapeSet.inter keys common_keys
      in
      let all_keys = static_keys_of join in
      let common_keys =
        if EntitySet.cardinal adjacencies.backwards < 2 then
          all_keys
        else
          EntitySet.fold handle_predecessor adjacencies.backwards all_keys
      in
      let optional_keys =
        T.TShapeSet.diff all_keys common_keys
        |> T.TShapeSet.elements
        |> List.map ~f:(fun optional_key -> (join, optional_key))
      in
      optional_keys @ acc
    in
    EntityMap.fold add_optional_key adjacency_map [] @ optional_accesses
  in
  (* Close upwards *)
  let optional_accesses =
    List.concat_map
      ~f:(fun (entity, key) ->
        collect_supersets entity |> List.map ~f:(fun entity -> (entity, key)))
      optional_accesses
  in

  (* Close upwards and downwards *)
  let dynamic_accesses =
    derive_dynamic_accesses adjacency_map dynamic_accesses
  in
  assemble
    {
      markers;
      base_static_accesses;
      derived_static_accesses;
      optional_accesses;
      dynamic_accesses;
      subsets;
    }

(*
  static_shape_result(E) :- marks(E), not has_dynamic_key_ud(E).
  static_shape_result_key(E,Key,Ty) :- has_static_key_u(E,Key,Ty).
  static_shape_result_key_optional(E,Key) :- has_optional_key_u(E,Key).

  dynamic_shape_result(E) :- marks(E), has_dynamic_key_ud(E).
*)
let produce_results
    (env : Typing_env_types.env) (constraints : constraint_ list) :
    shape_result list =
  let {
    markers;
    derived_static_accesses;
    optional_accesses;
    dynamic_accesses;
    _;
  } =
    disassemble constraints
  in

  (* Start collecting shape results starting with empty shapes of candidates *)
  let static_shape_results : (marker_kind * shape_keys) Pos.Map.t =
    markers
    |> List.fold
         ~f:(fun map (kind, pos) ->
           Pos.Map.add pos (kind, T.TShapeMap.empty) map)
         ~init:Pos.Map.empty
  in

  (* Invalidate candidates that are observed to experience dynamic access *)
  let static_shape_results : (marker_kind * shape_keys) Pos.Map.t =
    static_shape_results
    |> Pos.Map.filter (fun pos _ ->
           not @@ EntitySet.mem (Literal pos) dynamic_accesses)
  in

  (* Add known keys *)
  let optional_accesses =
    optional_accesses
    |> List.fold
         ~f:(fun map (entity, key) ->
           EntityMap.update
             entity
             (function
               | None -> Some (T.TShapeSet.singleton key)
               | Some keys -> Some (T.TShapeSet.add key keys))
             map)
         ~init:EntityMap.empty
  in
  let is_optional entity key =
    match EntityMap.find_opt entity optional_accesses with
    | Some set -> T.TShapeSet.mem key set
    | None -> false
  in
  let static_shape_results : (marker_kind * shape_keys) Pos.Map.t =
    let update_entity entity key ty = function
      | None -> None
      | Some (kind, shape_keys') ->
        let optional_field = is_optional entity key in
        Some (kind, Logic.(singleton key ty optional_field <> shape_keys') ~env)
    in
    let add_entity (entity, key, ty) pos_map =
      match entity with
      | Literal pos
      | Inter (HT.Param (_, _, pos)) ->
        Pos.Map.update pos (update_entity entity key ty) pos_map
      | Variable _ -> pos_map
    in
    StaticAccess.Set.fold
      add_entity
      derived_static_accesses
      static_shape_results
  in

  (* Convert to individual statically accessed dict results *)
  let static_shape_results : shape_result list =
    static_shape_results
    |> Pos.Map.bindings
    |> List.map ~f:(fun (pos, (marker_kind, keys_and_types)) ->
           Shape_like_dict (pos, marker_kind, keys_and_types))
  in

  let dynamic_shape_results =
    markers
    |> List.map ~f:(fun (_, pos) -> Literal pos)
    |> EntitySet.of_list
    |> EntitySet.inter dynamic_accesses
    |> EntitySet.elements
    |> List.map ~f:(fun entity_ -> Dynamically_accessed_dict entity_)
  in

  static_shape_results @ dynamic_shape_results

let is_same_entity (param_ent_1 : HT.entity) (ent : entity_) : bool =
  match ent with
  | Literal _
  | Variable _ ->
    false
  | Inter param_ent_2 -> HT.equal_entity param_ent_1 param_ent_2

let substitute_inter_intra
    (inter_constr : inter_constraint_) (intra_constr : constraint_) :
    constraint_ =
  match inter_constr with
  | HT.Arg (param_ent, intra_ent_1) ->
    let replace intra_ent_2 =
      if is_same_entity (HT.Param param_ent) intra_ent_2 then
        intra_ent_1
      else
        intra_ent_2
    in
    begin
      match intra_constr with
      | Marks _ -> intra_constr
      | Has_static_key (source, intra_ent_2, key, ty) ->
        Has_static_key (source, replace intra_ent_2, key, ty)
      | Has_optional_key (intra_ent_2, key) ->
        Has_optional_key (replace intra_ent_2, key)
      | Has_dynamic_key intra_ent_2 -> Has_dynamic_key (replace intra_ent_2)
      | Subsets (intra_ent_2, intra_ent_3) ->
        Subsets (replace intra_ent_2, replace intra_ent_3)
    end

let equiv
    (any_constr_list_1 : any_constraint list)
    (any_constr_list_2 : any_constraint list) : bool =
  let only_intra_constr any_constr =
    let only_inter_ent (intra_constr : constraint_) :
        entity_ -> constraint_ option = function
      | Inter _ -> Some intra_constr
      | _ -> None
    in
    match any_constr with
    | HT.Intra intra_constr ->
      (match intra_constr with
      | Marks _ -> Some intra_constr
      | Has_static_key (_, ent, _, _) -> only_inter_ent intra_constr ent
      | Has_optional_key (ent, _) -> only_inter_ent intra_constr ent
      | Has_dynamic_key ent -> only_inter_ent intra_constr ent
      | _ -> None)
    | HT.Inter _ -> None
  in
  ConstraintSet.equal
    (ConstraintSet.of_list
       (List.filter_map ~f:only_intra_constr any_constr_list_1))
    (ConstraintSet.of_list
       (List.filter_map ~f:only_intra_constr any_constr_list_2))
