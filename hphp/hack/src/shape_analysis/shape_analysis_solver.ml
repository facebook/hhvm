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
  definite_static_accesses: StaticAccess.Set.t;
  maybe_static_accesses: StaticAccess.Set.t;
  dynamic_accesses: EntitySet.t;
  subsets: (entity_ * entity_) list;
}

let constraints_init =
  {
    markers = [];
    definite_static_accesses = StaticAccess.Set.empty;
    maybe_static_accesses = StaticAccess.Set.empty;
    dynamic_accesses = EntitySet.empty;
    subsets = [];
  }

let disassemble constraints =
  let partition_constraint constraints = function
    | Marks (kind, entity) ->
      { constraints with markers = (kind, entity) :: constraints.markers }
    | Has_static_key (Definite, entity, key, ty) ->
      {
        constraints with
        definite_static_accesses =
          StaticAccess.Set.add
            (entity, key, ty)
            constraints.definite_static_accesses;
      }
    | Has_static_key (Maybe, entity, key, ty) ->
      {
        constraints with
        maybe_static_accesses =
          StaticAccess.Set.add
            (entity, key, ty)
            constraints.maybe_static_accesses;
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
      definite_static_accesses;
      maybe_static_accesses;
      dynamic_accesses;
      subsets;
    } =
  List.map ~f:(fun (kind, entity) -> Marks (kind, entity)) markers
  @ List.map
      ~f:(fun (entity, key, ty) -> Has_static_key (Definite, entity, key, ty))
      (StaticAccess.Set.elements definite_static_accesses)
  @ List.map
      ~f:(fun (entity, key, ty) -> Has_static_key (Maybe, entity, key, ty))
      (StaticAccess.Set.elements maybe_static_accesses)
  @ List.map
      ~f:(fun entity -> Has_dynamic_key entity)
      (EntitySet.elements dynamic_accesses)
  @ List.map ~f:(fun (sub, sup) -> Subsets (sub, sup)) subsets

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
  Propagates the `Definite` variety of `Has_static_key` constraints forward
  through the dataflow graph. The propagation only happens if all incoming
  edges have the relevant static key, e.g.,

    ('a', int) -------\               ('a', int) -------\
    ('b', string)      \              ('b', string)      \
                        \                                 \
    ('a', float) -------- .   ====>   ('a', float) -------- ('a', int)
                        /                                 / ('a', float)
                       /                                 /
    ('a', int) -------/               ('a', int) -------/

  We use universal quantification over incoming edges which in general is not
  monotonic (hence not necessarily convergent in a fixpoint setting), however,
  the newly introduced edges in a growing adjacency map due to fixpoint
  computation cannot change the incident edges of any vertices. So, there is no
  way of invalidating a definite derived has static key constraint once it is
  established.

  TODO(T129093160): The monotonicity argument above is not strictly true due to
  parameters with default values.

  The implementation is semi-naïve, i.e., at each iteration it only considers
  what newly generated facts can affect.

    has_static_key_u(definite, E, K, Ty) :- has_static_key(definite, E, K, Ty)
    has_static_key_u(definite, E, K, Ty) :-
      common_definite_predecessor_key(E, K),
      subsets(E', E),
      has_static_key(E', K, Ty).

    common_definite_predecessor_key(E, K) :-
      forall subsets(E', E).
      has_static_key_u(definite, E', K, _).
*)
let derive_definite_static_accesses adjacency_map definite_static_accesses =
  let open StaticAccess.Set in
  (* All successors to the entities in recently generated facts can potentially
     obtain a new static key constraint. *)
  let find_candidates delta =
    let accum (e, _, _) =
      let forwards =
        Option.value_map
          ~default:EntitySet.empty
          ~f:(fun a -> a.forwards)
          (EntityMap.find_opt e adjacency_map)
      in
      EntitySet.union forwards
    in
    fold accum delta EntitySet.empty
  in
  let rec close_upwards ~delta ~acc =
    if is_empty delta then
      acc
    else
      (* The following collects the intersection of all keys that occur in
         predecessors of an entity along with possible types of those keys. *)
      let common_predecessor_keys e =
        let predecessors =
          EntityMap.find_opt e adjacency_map
          |> Option.value_map ~default:EntitySet.empty ~f:(fun a -> a.backwards)
        in
        let collect_common_keys e common_keys =
          let collect_keys (e', k, ty) =
            let add_ty = function
              | None -> Some [ty]
              | Some tys -> Some (ty :: tys)
            in
            if equal_entity_ e e' then
              T.TShapeMap.update k add_ty
            else
              Fn.id
          in
          let keys = fold collect_keys acc T.TShapeMap.empty in
          (* Here `None` represents the universe set *)
          match common_keys with
          | None -> Some keys
          | Some keys' ->
            let combine _ tys_opt tys'_opt =
              match (tys_opt, tys'_opt) with
              | (Some tys, Some tys') -> Some (tys @ tys')
              | _ -> None
            in
            Some (T.TShapeMap.merge combine keys keys')
        in
        EntitySet.fold collect_common_keys predecessors None
        |> Option.value ~default:T.TShapeMap.empty
      in
      (* Propagate all common keys forward *)
      let propagate e acc =
        let common_predecessor_keys = common_predecessor_keys e in
        T.TShapeMap.fold
          (fun k tys ->
            union (List.map ~f:(fun ty -> (e, k, ty)) tys |> of_list))
          common_predecessor_keys
          acc
      in
      let candidates = find_candidates delta in
      let delta = EntitySet.fold propagate candidates StaticAccess.Set.empty in
      let delta = diff delta acc in
      let acc = union delta acc in
      close_upwards ~delta ~acc
  in
  close_upwards ~delta:definite_static_accesses ~acc:definite_static_accesses

(*
  Propagates the `Maybe` variety of `Has_static_key` constraints forward
  through the dataflow graph.

  The implementation is semi-naïve, i.e., at each iteration it only considers
  newly generated facts.

    has_static_key_u(maybe, E, Key, Ty) :- has_static_key(_, E, Key, Ty).
    has_static_key_u(maybe, F, Key, Ty) :- has_static_key(maybe, E, Key, Ty), subsets(E,F).
*)
let derive_maybe_static_accesses
    adjacency_map ~maybe_static_accesses ~definite_static_accesses =
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
  let all =
    StaticAccess.Set.union maybe_static_accesses definite_static_accesses
  in
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
*)
let deduce (constraints : constraint_ list) : constraint_ list =
  let {
    markers;
    definite_static_accesses;
    maybe_static_accesses;
    dynamic_accesses;
    subsets;
  } =
    disassemble constraints
  in
  let adjacency_map = mk_adjacency_map subsets in

  (* Close upwards *)
  let maybe_static_accesses =
    derive_maybe_static_accesses
      adjacency_map
      ~maybe_static_accesses
      ~definite_static_accesses
  in

  (* Close upwards *)
  let definite_static_accesses =
    derive_definite_static_accesses adjacency_map definite_static_accesses
  in

  (* Close upwards and downwards *)
  let dynamic_accesses =
    derive_dynamic_accesses adjacency_map dynamic_accesses
  in
  assemble
    {
      markers;
      definite_static_accesses;
      maybe_static_accesses;
      dynamic_accesses;
      subsets;
    }

(*
  static_shape_result(E) :- marks(E), not has_dynamic_key_ud(E).
  static_shape_result_key(E,Key,Ty) :- has_static_key_u(_,E,Key,Ty).
  static_shape_result_key_optional(E,Key) :-
    has_static_key_u(_, E, Key, _),
    not has_static_key_u(definite, E, Key, _).

  dynamic_shape_result(E) :- marks(E), has_dynamic_key_ud(E).
*)
let produce_results
    (env : Typing_env_types.env) (constraints : constraint_ list) :
    shape_result list =
  let {
    markers;
    definite_static_accesses;
    maybe_static_accesses;
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

  (* Add known keys (maybe variant) *)
  let add_known_keys ~is_optional static_accesses static_shape_results :
      (marker_kind * shape_keys) Pos.Map.t =
    let update_entity _ key ty = function
      | None -> None
      | Some (kind, shape_keys') ->
        Some (kind, Logic.(singleton key ty is_optional <> shape_keys') ~env)
    in
    let add_entity (entity, key, ty) pos_map =
      match entity with
      | Literal pos
      | Inter (HT.Param (_, _, pos)) ->
        Pos.Map.update pos (update_entity entity key ty) pos_map
      | Variable _ -> pos_map
    in
    StaticAccess.Set.fold add_entity static_accesses static_shape_results
  in
  let static_shape_results =
    static_shape_results
    |> add_known_keys maybe_static_accesses ~is_optional:true
    |> add_known_keys definite_static_accesses ~is_optional:false
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
      | Has_dynamic_key ent -> only_inter_ent intra_constr ent
      | _ -> None)
    | HT.Inter _ -> None
  in
  ConstraintSet.equal
    (ConstraintSet.of_list
       (List.filter_map ~f:only_intra_constr any_constr_list_1))
    (ConstraintSet.of_list
       (List.filter_map ~f:only_intra_constr any_constr_list_2))
