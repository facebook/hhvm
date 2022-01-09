(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types

type constraints = {
  exists: Pos.t list;
  static_accesses: (entity_ * shape_key * Typing_defs.locl_ty) list;
  dynamic_accesses: entity_ list;
  subsets: (entity_ * entity_) list;
}

let constraints_init =
  { exists = []; static_accesses = []; dynamic_accesses = []; subsets = [] }

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
  | Exists (_, entity) ->
    { constraints with exists = entity :: constraints.exists }
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
  | Subset (sub, sup) ->
    { constraints with subsets = (sub, sup) :: constraints.subsets }

(* The following program roughly summarises the solver.

  subset'(A,B) :- subset(A,B).
  subset'(A,C) :- subset(A,B), subset'(B,C).

  subset''(A, Literal Pos) :- subset'(A, Literal Pos).

  has_static_key'(Literal Pos, Key, Ty) :- has_static_key(Literal Pos, Key, Ty).
  has_static_key'(B, Key, Ty) :- has_static_key(A, Key, Ty), subset''(A,B).

  has_dynamic_key'(Literal Pos) :- has_dynamic_key(Literal Pos).
  has_dynamic_key'(B) :- has_dynamic_key(A), subset''(A,B).

  static_shape_result(A) :- exists(A), not has_dynamic_key'(A).
  static_shape_result_key(A,K,Ty) :- has_static_key'(A,K,Ty)

  dynamic_shape_result(A) :- exists(A), has_dynamic_key'(A).
*)
let simplify (env : Typing_env_types.env) (constraints : constraint_ list) :
    shape_result list =
  let { exists; static_accesses; dynamic_accesses; subsets } =
    List.fold ~init:constraints_init ~f:partition_constraint constraints
  in

  let subsets = PointsToSet.of_list subsets |> transitive_closure in
  let subset_of_literal_index =
    let f elt map =
      match elt with
      | (_, Variable _) -> map
      | (e1, Literal pos) ->
        EntityMap.update
          e1
          (function
            | None -> Some (Pos.Set.singleton pos)
            | Some s -> Some (Pos.Set.add pos s))
          map
    in
    PointsToSet.fold f subsets EntityMap.empty
  in
  (* Find all concrete supersets. This means all concrete positions that are
     supersets of a variable, or in the case of a literal all concrete
     positions that are supersets of a variable + the concrete position we have
     at hand. *)
  let all_concrete_supersets entity =
    let find_super_poss entity =
      match EntityMap.find_opt entity subset_of_literal_index with
      | Some poss -> Pos.Set.elements poss
      | None -> []
    in
    match entity with
    | Literal pos -> pos :: find_super_poss entity
    | Variable _ -> find_super_poss entity
  in

  let static_accesses =
    List.concat_map
      ~f:(fun (entity, key, ty) ->
        all_concrete_supersets entity |> List.map ~f:(fun pos -> (pos, key, ty)))
      static_accesses
  in

  (* Start collecting shape results starting with empty shapes of candidates *)
  let static_shape_results : Typing_defs.locl_ty ShapeKeyMap.t Pos.Map.t =
    exists
    |> List.fold
         ~f:(fun map pos -> Pos.Map.add pos ShapeKeyMap.empty map)
         ~init:Pos.Map.empty
  in

  (* Invalidate candidates that are observed to experience dynamic access *)
  let dynamic_accesses =
    dynamic_accesses
    |> List.concat_map ~f:all_concrete_supersets
    |> Pos.Set.of_list
  in
  let static_shape_results : Typing_defs.locl_ty ShapeKeyMap.t Pos.Map.t =
    static_shape_results
    |> Pos.Map.filter (fun pos _ -> not @@ Pos.Set.mem pos dynamic_accesses)
  in

  (* Add known keys *)
  let static_shape_results : Typing_defs.locl_ty ShapeKeyMap.t Pos.Map.t =
    let update_shape_key ty = function
      | None -> Some ty
      | Some ty' ->
        let (_env, ty) = Typing_union.union env ty ty' in
        Some ty
    in
    let update_entity key ty = function
      | None -> None
      | Some shape_key_map ->
        Some (ShapeKeyMap.update key (update_shape_key ty) shape_key_map)
    in
    static_accesses
    |> List.fold ~init:static_shape_results ~f:(fun pos_map (pos, key, ty) ->
           Pos.Map.update pos (update_entity key ty) pos_map)
  in

  (* Convert to individual statically accessed dict results *)
  let static_shape_results : shape_result list =
    static_shape_results
    |> Pos.Map.bindings
    |> List.map ~f:(fun (pos, keys_and_types) ->
           Shape_like_dict (pos, ShapeKeyMap.bindings keys_and_types))
  in

  (* TODO: only consider existing candidates to report a summary *)
  let dynamic_shape_results =
    Pos.Set.elements dynamic_accesses
    |> List.map ~f:(fun entity_ -> Dynamically_accessed_dict (Literal entity_))
  in

  static_shape_results @ dynamic_shape_results
