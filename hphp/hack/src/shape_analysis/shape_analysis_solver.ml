(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
module PosSet = Caml.Set.Make (Pos)

type constraints = {
  exists: entity_ list;
  static_accesses: (entity_ * shape_key * Typing_defs.locl_ty) list;
  dynamic_accesses: entity_ list;
  points_tos: (entity_ * entity_) list;
}

let constraints_init =
  { exists = []; static_accesses = []; dynamic_accesses = []; points_tos = [] }

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

let simplify (env : Typing_env_types.env) (constraints : constraint_ list) :
    shape_result list =
  let partition_constraint constraints = function
    | Exists entity ->
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
    | Points_to (pointer_entity, pointed_entity) ->
      {
        constraints with
        points_tos = (pointer_entity, pointed_entity) :: constraints.points_tos;
      }
  in
  let { exists; static_accesses; dynamic_accesses; points_tos } =
    List.fold ~init:constraints_init ~f:partition_constraint constraints
  in

  let variable_to_literal_map =
    let add_pointer_to_literal points_to map =
      match points_to with
      | (Variable pointer, Literal pointed) ->
        IMap.add pointer (PosSet.singleton pointed) map ~combine:PosSet.union
      | _ -> map
    in
    PointsToSet.of_list points_tos |> transitive_closure |> fun points_to_set ->
    PointsToSet.fold add_pointer_to_literal points_to_set IMap.empty
  in

  let poss_of_entity = function
    | Literal pos -> [pos]
    | Variable var ->
      begin
        match IMap.find_opt var variable_to_literal_map with
        | Some poss -> PosSet.elements poss
        | None ->
          failwith
            (Format.sprintf "Could not find which entity %d points to" var)
      end
  in
  let static_accesses =
    List.concat_map
      ~f:(fun (entity, key, ty) ->
        poss_of_entity entity |> List.map ~f:(fun pos -> (pos, key, ty)))
      static_accesses
  in

  (* Eliminate static access constraints for entities that aren't `dict`s *)
  let exists = exists |> List.concat_map ~f:poss_of_entity |> PosSet.of_list in
  let static_accesses =
    List.filter static_accesses ~f:(fun (pos, _, _) -> PosSet.mem pos exists)
  in

  (* Eliminate static access constraints for entities that are dynamically
     accessed *)
  let dynamic_accesses =
    dynamic_accesses |> List.concat_map ~f:poss_of_entity |> PosSet.of_list
  in
  let static_accesses =
    List.filter static_accesses ~f:(fun (pos, _, _) ->
        not @@ PosSet.mem pos dynamic_accesses)
  in

  (* Group static access constraints to determine the shape type *)
  let sorted_accesses : (Pos.t * shape_key * Tast.ty) list =
    static_accesses
    |> List.sort ~compare:(fun (pos1, key1, _) (pos2, key2, _) ->
           match Pos.compare pos1 pos2 with
           | 0 -> compare_shape_key key1 key2
           | n -> n)
  in
  let accesses_grouped_by_entity : (Pos.t * (shape_key * Tast.ty) list) list =
    let group_by_entity group =
      let (pos, _, _) = List.hd_exn group in
      let keys_and_tys : (shape_key * Tast.ty) list =
        List.map group ~f:(fun (_, key, ty) -> (key, ty))
      in
      (pos, keys_and_tys)
    in
    sorted_accesses
    |> List.group ~break:(fun (pos1, _, _) (pos2, _, _) ->
           not @@ Pos.equal pos1 pos2)
    |> List.map ~f:group_by_entity
  in
  let accesses_grouped_by_key : (Pos.t * (shape_key * Tast.ty list) list) list =
    let group_by_key (pos, group) =
      let group =
        group
        |> List.group ~break:(fun (key1, _) (key2, _) ->
               not @@ equal_shape_key key1 key2)
        |> List.map ~f:(fun group ->
               let (key, _) = List.hd_exn group in
               let tys = List.map ~f:snd group in
               (key, tys))
      in
      (pos, group)
    in
    accesses_grouped_by_entity |> List.map ~f:group_by_key
  in

  let combined_types : (Pos.t * (shape_key * Tast.ty) list) list =
    let fold_tys tys =
      snd
      @@ List.fold
           ~f:(fun (env, ty1) ty2 -> Typing_union.union env ty1 ty2)
           ~init:(env, Typing_make_type.nothing Typing_reason.Rnone)
           tys
    in
    accesses_grouped_by_key
    |> List.map ~f:(fun (pos, keys_and_tys) ->
           ( pos,
             List.map keys_and_tys ~f:(fun (key, tys) -> (key, fold_tys tys)) ))
  in

  let static_shape_results : shape_result list =
    combined_types
    |> List.map ~f:(fun (pos, keys_and_types) ->
           Shape_like_dict (pos, keys_and_types))
  in

  let dynamic_shape_results =
    PosSet.elements dynamic_accesses
    |> List.map ~f:(fun entity_ -> Dynamically_accessed_dict (Literal entity_))
  in

  static_shape_results @ dynamic_shape_results
