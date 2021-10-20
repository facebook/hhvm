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

let simplify (env : Typing_env_types.env) (constraints : constraint_ list) :
    shape_result list =
  let (exists, static_accesses, dynamic_accesses) =
    List.partition3_map constraints ~f:(function
        | Exists entity -> `Fst entity
        | Has_static_key (entity, key, ty) -> `Snd (entity, key, ty)
        | Has_dynamic_key entity -> `Trd entity)
  in

  (* Eliminate static access constraints for entities that aren't `dict`s *)
  let exists =
    exists |> List.map ~f:(function Literal pos -> pos) |> PosSet.of_list
  in
  let static_accesses =
    List.filter static_accesses ~f:(fun (Literal pos, _, _) ->
        PosSet.mem pos exists)
  in

  (* Eliminate static access constraints for entities that are dynamically
     accessed *)
  let dynamic_accesses =
    dynamic_accesses |> List.map ~f:(fun (Literal pos) -> pos) |> PosSet.of_list
  in
  let static_accesses =
    List.filter static_accesses ~f:(fun (Literal pos, _, _) ->
        not @@ PosSet.mem pos dynamic_accesses)
  in

  (* Group static access constraints to determine the shape type *)
  let sorted_accesses : (entity_ * shape_key * Tast.ty) list =
    static_accesses
    |> List.sort
         ~compare:(fun (Literal pos1, key1, _) (Literal pos2, key2, _) ->
           match Pos.compare pos1 pos2 with
           | 0 -> compare_shape_key key1 key2
           | n -> n)
  in
  let accesses_grouped_by_entity : (entity_ * (shape_key * Tast.ty) list) list =
    let group_by_entity group =
      let (entity_, _, _) = List.hd_exn group in
      let keys_and_tys : (shape_key * Tast.ty) list =
        List.map group ~f:(fun (_, key, ty) -> (key, ty))
      in
      (entity_, keys_and_tys)
    in
    sorted_accesses
    |> List.group ~break:(fun (Literal pos1, _, _) (Literal pos2, _, _) ->
           not @@ Pos.equal pos1 pos2)
    |> List.map ~f:group_by_entity
  in
  let accesses_grouped_by_key : (entity_ * (shape_key * Tast.ty list) list) list
      =
    let group_by_key (entity_, group) =
      let group =
        group
        |> List.group ~break:(fun (key1, _) (key2, _) ->
               not @@ equal_shape_key key1 key2)
        |> List.map ~f:(fun group ->
               let (key, _) = List.hd_exn group in
               let tys = List.map ~f:snd group in
               (key, tys))
      in
      (entity_, group)
    in
    accesses_grouped_by_entity |> List.map ~f:group_by_key
  in

  let combined_types : (entity_ * (shape_key * Tast.ty) list) list =
    let fold_tys tys =
      snd
      @@ List.fold
           ~f:(fun (env, ty1) ty2 -> Typing_union.union env ty1 ty2)
           ~init:(env, Typing_make_type.nothing Typing_reason.Rnone)
           tys
    in
    accesses_grouped_by_key
    |> List.map ~f:(fun (entity_, keys_and_tys) ->
           ( entity_,
             List.map keys_and_tys ~f:(fun (key, tys) -> (key, fold_tys tys)) ))
  in

  let static_shape_results : shape_result list =
    combined_types
    |> List.map ~f:(fun (entity, keys_and_types) ->
           Shape_like_dict (entity, keys_and_types))
  in

  let dynamic_shape_results =
    PosSet.elements dynamic_accesses
    |> List.map ~f:(fun entity_ -> Dynamically_accessed_dict (Literal entity_))
  in

  static_shape_results @ dynamic_shape_results
