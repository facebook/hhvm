(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
module Logic = Shape_analysis_logic

type constraints = {
  markers: Pos.t list;
  static_accesses: (entity_ * shape_key * Typing_defs.locl_ty) list;
  dynamic_accesses: entity_ list;
  subsets: (entity_ * entity_) list;
  joins: (entity_ * entity_ * entity_) list;
}

let constraints_init =
  {
    markers = [];
    static_accesses = [];
    dynamic_accesses = [];
    subsets = [];
    joins = [];
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

let partition_constraint constraints = function
  | Marks (_, entity) ->
    { constraints with markers = entity :: constraints.markers }
  | Has_static_key (entity, key, ty) ->
    {
      constraints with
      static_accesses = (entity, key, ty) :: constraints.static_accesses;
    }
  | Has_dynamic_key entity ->
    {
      constraints with
      dynamic_accesses = entity :: constraints.dynamic_accesses;
    }
  | Subsets (sub, sup) ->
    { constraints with subsets = (sub, sup) :: constraints.subsets }
  | Joins { left; right; join } ->
    { constraints with joins = (left, right, join) :: constraints.joins }

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

(* The following program roughly summarises the solver.

  // Reflexive closure
  subset'(A,A) :-
    subset(A,_); subset(_,A);
    union(A,_,_); union(_,A,_); union(_,_,A);
    has_static_key(A,_,_);
    has_dynamic_key(A,_,_).
  // Transitive closure closure
  subset'(A,B) :- subset(A,B); union(A,_,B); union(_,A,B).
  subset'(A,C) :- subset(A,B), subset'(B,C).

  has_static_key'(Entity, Key, Ty) :- has_static_key(Entity, Key, Ty).
  has_static_key'(B, Key, Ty) :- has_static_key(A, Key, Ty), subset'(A,B).

  has_dynamic_key'(Entity) :- has_dynamic_key(Entity).
  has_dynamic_key'(Entity') :-
    has_dynamic_key(Entity),
    (subset'(Entity,Entity'); subset'(Entity',Entity)).

  // Has optional key constraint only exists within the solver. When there is a
  // join but we cannot see a key appearing in both incoming branches, we mark
  // the key as optional.
  has_optional_key'(Entity') :-
    has_optional_key(Entity),
    subset'(Entity, Entity').

  static_shape_result(A) :- marks(A), not has_dynamic_key'(A).
  static_shape_result_key(A,K,Ty) :- has_static_key'(A,K,Ty)

  dynamic_shape_result(A) :- marks(A), has_dynamic_key'(A).
*)
let simplify (env : Typing_env_types.env) (constraints : constraint_ list) :
    shape_result list =
  let { markers; static_accesses; dynamic_accesses; subsets; joins } =
    List.fold ~init:constraints_init ~f:partition_constraint constraints
  in

  let subsets_reflexive =
    List.map ~f:(fun pos -> Literal pos) markers
    @ List.map static_accesses ~f:(fun (e, _, _) -> e)
    @ dynamic_accesses
    @ List.concat_map subsets ~f:(fun (e, e') -> [e; e'])
    @ List.concat_map joins ~f:(fun (e, e', e'') -> [e; e'; e''])
    |> List.map ~f:(fun e -> (e, e))
  in
  let subsets_through_joins =
    List.concat_map joins ~f:(fun (e1, e2, join) -> [(e1, join); (e2, join)])
  in
  let subsets = subsets_through_joins @ subsets_reflexive @ subsets in
  let subsets = PointsToSet.of_list subsets |> transitive_closure in
  let (collect_subsets, collect_supersets) = subset_lookups subsets in

  let static_accesses_upwards_closed =
    List.concat_map
      ~f:(fun (entity, key, ty) ->
        collect_supersets entity
        |> List.map ~f:(fun entity -> (entity, key, ty)))
      static_accesses
  in

  let optional_keys =
    let add_optional_key (left, right, join) =
      let filter_keys e =
        List.filter_map
          ~f:(fun (e', key, _) ->
            if equal_entity_ e e' then
              Some key
            else
              None)
          static_accesses_upwards_closed
      in
      let left_static_keys = filter_keys left |> ShapeKeySet.of_list in
      let right_static_keys = filter_keys right |> ShapeKeySet.of_list in
      ShapeKeySet.diff
        (ShapeKeySet.union left_static_keys right_static_keys)
        (ShapeKeySet.inter left_static_keys right_static_keys)
      |> ShapeKeySet.elements
      |> List.map ~f:(fun optional_key -> (join, optional_key))
    in
    List.concat_map ~f:add_optional_key joins
  in
  let optional_keys_upwards_closed =
    List.concat_map
      ~f:(fun (entity, key) ->
        collect_supersets entity |> List.map ~f:(fun entity -> (entity, key)))
      optional_keys
    |> List.fold
         ~f:(fun map (entity, key) ->
           EntityMap.update
             entity
             (function
               | None -> Some (ShapeKeySet.singleton key)
               | Some keys -> Some (ShapeKeySet.add key keys))
             map)
         ~init:EntityMap.empty
  in
  let is_optional entity key =
    match EntityMap.find_opt entity optional_keys_upwards_closed with
    | Some set ->
      if ShapeKeySet.mem key set then
        FOptional
      else
        FRequired
    | None -> FRequired
  in

  (* Start collecting shape results starting with empty shapes of candidates *)
  let static_shape_results : shape_keys Pos.Map.t =
    markers
    |> List.fold
         ~f:(fun map pos -> Pos.Map.add pos ShapeKeyMap.empty map)
         ~init:Pos.Map.empty
  in

  (* Invalidate candidates that are observed to experience dynamic access *)
  let dynamic_accesses =
    let upward_dynamic_accesses =
      dynamic_accesses |> List.concat_map ~f:collect_supersets
    in
    let downward_dynamic_accesses =
      dynamic_accesses |> List.concat_map ~f:collect_subsets
    in

    EntitySet.of_list @@ upward_dynamic_accesses @ downward_dynamic_accesses
  in
  let static_shape_results : shape_keys Pos.Map.t =
    static_shape_results
    |> Pos.Map.filter (fun pos _ ->
           not @@ EntitySet.mem (Literal pos) dynamic_accesses)
  in

  (* Add known keys *)
  let static_shape_results : shape_keys Pos.Map.t =
    let update_entity entity key ty = function
      | None -> None
      | Some shape_keys' ->
        let optional_field = is_optional entity key in
        Some (Logic.(singleton key ty optional_field <> shape_keys') ~env)
    in
    static_accesses_upwards_closed
    |> List.fold ~init:static_shape_results ~f:(fun pos_map (entity, key, ty) ->
           match entity with
           | Literal pos ->
             Pos.Map.update pos (update_entity entity key ty) pos_map
           | Variable _ -> pos_map)
  in

  (* Convert to individual statically accessed dict results *)
  let static_shape_results : shape_result list =
    static_shape_results
    |> Pos.Map.bindings
    |> List.map ~f:(fun (pos, keys_and_types) ->
           Shape_like_dict
             ( pos,
               ShapeKeyMap.bindings keys_and_types
               |> List.map ~f:(fun (a, (b, c)) -> (a, b, c)) ))
  in

  let dynamic_shape_results =
    markers
    |> List.map ~f:(fun pos -> Literal pos)
    |> EntitySet.of_list
    |> EntitySet.inter dynamic_accesses
    |> EntitySet.elements
    |> List.map ~f:(fun entity_ -> Dynamically_accessed_dict entity_)
  in

  static_shape_results @ dynamic_shape_results
