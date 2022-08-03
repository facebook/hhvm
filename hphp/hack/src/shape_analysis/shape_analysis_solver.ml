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

type constraints = {
  markers: (marker_kind * Pos.t) list;
  static_accesses: (entity_ * T.TShapeMap.key * Typing_defs.locl_ty) list;
  optional_accesses: (entity_ * T.TShapeMap.key) list;
  dynamic_accesses: entity_ list;
  subsets: (entity_ * entity_) list;
  joins: (entity_ * entity_ * entity_) list;
}

let constraints_init =
  {
    markers = [];
    static_accesses = [];
    optional_accesses = [];
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

let disassemble constraints =
  let partition_constraint constraints = function
    | Marks (kind, entity) ->
      { constraints with markers = (kind, entity) :: constraints.markers }
    | Has_static_key (entity, key, ty) ->
      {
        constraints with
        static_accesses = (entity, key, ty) :: constraints.static_accesses;
      }
    | Has_optional_key (entity, key) ->
      {
        constraints with
        optional_accesses = (entity, key) :: constraints.optional_accesses;
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
  in
  List.fold ~init:constraints_init ~f:partition_constraint constraints

let assemble
    {
      markers;
      static_accesses;
      optional_accesses;
      dynamic_accesses;
      subsets;
      joins;
    } =
  List.map ~f:(fun (kind, entity) -> Marks (kind, entity)) markers
  @ List.map
      ~f:(fun (entity, key, ty) -> Has_static_key (entity, key, ty))
      static_accesses
  @ List.map
      ~f:(fun (entity, key) -> Has_optional_key (entity, key))
      optional_accesses
  @ List.map ~f:(fun entity -> Has_dynamic_key entity) dynamic_accesses
  @ List.map ~f:(fun (sub, sup) -> Subsets (sub, sup)) subsets
  @ List.map ~f:(fun (left, right, join) -> Joins { left; right; join }) joins

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
    joins(E,_,_); joins(_,E,_); joins(_,_,E);
    has_static_key(E,_,_);
    has_dynamic_key(E,_,_).
  // Transitive closure closure
  subsets_tr(E,F) :- subsets(E,F); joins(E,_,F); joins(_,E,F).
  subsets_tr(E,F) :- subsets(E,F), subsets_re(E,F).

  has_static_key_u(E, Key, Ty) :- has_static_key(E, Key, Ty).
  has_static_key_u(F, Key, Ty) :- has_static_key(E, Key, Ty), subsets_tr(E,F).

  // Close dynamic key access in both directions to later invalidate all
  // results that touch it.
  has_dynamic_key_ud(E) :- has_dynamic_key(E).
  has_dynamic_key_ud(F) :- has_dynamic_key(E), (subsets_tr(E,F); subsets_tr(F,E)).

  // We conclude that a key is optional either we explicitly observed it in the
  // source code, for example, through the use of `idx` or due to control flow
  // making the key exist in one branch of execution but not another.
  //
  // TODO(T125888579): Note that this treatment is imprecise as we can later
  // observe the key to be definitely in place, but it would still be marked as
  // optional.
  has_optional_key_base(E,Key) :- has_optional_key(E,Key).
  has_optional_key_base(E,Key) :-
    (joins(E,F,Join); joins(F,E,Join)),
    has_static_key_u(E,Key),
    not has_static_key_u(F,Key).
  has_optional_key_u(F,Key) :- has_optional_key_base(E,Key), subsets_tr(E,F).
*)
(* TODO(T125884349): Specially handle flows into return type hints *)
let deduce (constraints : constraint_ list) : constraint_ list =
  let {
    markers;
    static_accesses;
    optional_accesses;
    dynamic_accesses;
    subsets;
    joins;
  } =
    disassemble constraints
  in

  let subsets_reflexive =
    List.map ~f:(fun (_, pos) -> Literal pos) markers
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
  let subsets = PointsToSet.elements subsets in

  (* Close upwards *)
  let static_accesses =
    List.concat_map
      ~f:(fun (entity, key, ty) ->
        collect_supersets entity
        |> List.map ~f:(fun entity -> (entity, key, ty)))
      static_accesses
  in

  let optional_accesses =
    let add_optional_key (left, right, join) =
      let filter_keys e =
        List.filter_map
          ~f:(fun (e', key, _) ->
            if equal_entity_ e e' then
              Some key
            else
              None)
          static_accesses
      in
      let left_static_keys = filter_keys left |> T.TShapeSet.of_list in
      let right_static_keys = filter_keys right |> T.TShapeSet.of_list in
      T.TShapeSet.diff
        (T.TShapeSet.union left_static_keys right_static_keys)
        (T.TShapeSet.inter left_static_keys right_static_keys)
      |> T.TShapeSet.elements
      |> List.map ~f:(fun optional_key -> (join, optional_key))
    in
    List.concat_map ~f:add_optional_key joins @ optional_accesses
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
    let upward_dynamic_accesses =
      dynamic_accesses |> List.concat_map ~f:collect_supersets
    in
    let downward_dynamic_accesses =
      dynamic_accesses |> List.concat_map ~f:collect_subsets
    in

    upward_dynamic_accesses @ downward_dynamic_accesses
  in
  assemble
    {
      markers;
      static_accesses;
      optional_accesses;
      dynamic_accesses;
      subsets;
      joins;
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
  let { markers; static_accesses; optional_accesses; dynamic_accesses; _ } =
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
  let dynamic_accesses = EntitySet.of_list dynamic_accesses in
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
    static_accesses
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
