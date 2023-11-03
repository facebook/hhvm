(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core

(* Avoids warning 66 about unused open Ppx_yojson_conv_lib.Yojson_conv.Primitives *)
let _ = yojson_of_unit

module type S = WrappedMap_sig.S

module Make (Ord : Caml.Map.OrderedType) : S with type key = Ord.t = struct
  include Caml.Map.Make (Ord)

  let union ?combine x y =
    let combine =
      match combine with
      | None -> (fun _ fst _ -> Some fst)
      | Some f -> f
    in
    union combine x y

  let merge_env env s1 s2 ~combine =
    let (env, map) =
      Common.List.fold_left_env
        env
        ~init:empty
        ~f:(fun env map (key, v2) ->
          let v1opt = find_opt key s1 in
          let (env, vopt) = combine env key v1opt (Some v2) in
          let map =
            match vopt with
            | None -> map
            | Some v -> add key v map
          in
          (env, map))
        (bindings s2)
    in
    Common.List.fold_left_env
      env
      ~init:map
      ~f:(fun env map (key, v1) ->
        let v2opt = find_opt key s2 in
        match v2opt with
        | None ->
          let (env, vopt) = combine env key (Some v1) None in
          let map =
            match vopt with
            | None -> map
            | Some v -> add key v map
          in
          (env, map)
        | Some _ -> (env, map))
      (bindings s1)

  let union_env env s1 s2 ~combine =
    let f env key o1 o2 =
      match (o1, o2) with
      | (None, None) -> (env, None)
      | (Some v, None)
      | (None, Some v) ->
        (env, Some v)
      | (Some v1, Some v2) -> combine env key v1 v2
    in
    merge_env env s1 s2 ~combine:f

  let keys m = fold (fun k _ acc -> k :: acc) m []

  let ordered_keys m = Base.List.map ~f:fst (bindings m)

  let values m = fold (fun _ v acc -> v :: acc) m []

  let fold_env env f m init =
    fold (fun key v (env, acc) -> f env key v acc) m (env, init)

  let fold_env_ty_err_opt env f m init =
    fold
      (fun key v ((env, errs), acc) ->
        match f env key v acc with
        | ((env, Some err), acc) -> ((env, err :: errs), acc)
        | ((env, _), acc) -> ((env, errs), acc))
      m
      ((env, []), init)

  let elements m = fold (fun k v acc -> (k, v) :: acc) m []

  let map_env f env m =
    fold_env
      env
      (fun env key v map ->
        let (env, v) = f env key v in
        (env, add key v map))
      m
      empty

  let map_env_ty_err_opt f env m ~combine_ty_errs =
    let ((env, errs), res) =
      fold_env_ty_err_opt
        env
        (fun env key v map ->
          let (env, v) = f env key v in
          (env, add key v map))
        m
        empty
    in
    ((env, combine_ty_errs errs), res)

  let filter_opt m = filter_map (fun _key x -> x) m

  let of_list elts =
    List.fold ~f:(fun acc (key, value) -> add key value acc) ~init:empty elts

  let of_function domain f =
    List.fold ~f:(fun acc key -> add key (f key) acc) ~init:empty domain

  let add ?combine key new_value map =
    match combine with
    | None -> add key new_value map
    | Some combine -> begin
      match find_opt key map with
      | None -> add key new_value map
      | Some old_value -> add key (combine old_value new_value) map
    end

  let ident_map f map =
    let (map_, changed) =
      fold
        (fun key item (map_, changed) ->
          let item_ = f item in
          (add key item_ map_, changed || not (phys_equal item_ item)))
        map
        (empty, false)
    in
    if changed then
      map_
    else
      map

  let ident_map_key ?combine f map =
    let (map_, changed) =
      fold
        (fun key item (map_, changed) ->
          let new_key = f key in
          ( add ?combine new_key item map_,
            changed || not (phys_equal new_key key) ))
        map
        (empty, false)
    in
    if changed then
      map_
    else
      map

  let for_all2 ~f m1 m2 =
    let key_bool_map =
      merge (fun k v1opt v2opt -> Some (f k v1opt v2opt)) m1 m2
    in
    for_all (fun _k b -> b) key_bool_map

  let make_pp pp_key pp_data fmt x =
    Format.fprintf fmt "@[<hv 2>{";
    let bindings = bindings x in
    (match bindings with
    | [] -> ()
    | _ -> Format.fprintf fmt " ");
    ignore
      (List.fold
         ~f:(fun sep (key, data) ->
           if sep then Format.fprintf fmt ";@ ";
           Format.fprintf fmt "@[";
           pp_key fmt key;
           Format.fprintf fmt " ->@ ";
           pp_data fmt data;
           Format.fprintf fmt "@]";
           true)
         ~init:false
         bindings);
    (match bindings with
    | [] -> ()
    | _ -> Format.fprintf fmt " ");
    Format.fprintf fmt "}@]"

  type ('a, 'b) tuple = 'a * 'b [@@deriving hash]

  let make_hash_fold_t
      (hash_fold_ord : Hash.state -> Ord.t -> Hash.state)
      (hash_fold_a : Hash.state -> 'a -> Hash.state)
      (hsv : Hash.state)
      (map : 'a t) =
    hash_fold_list
      (hash_fold_tuple hash_fold_ord hash_fold_a)
      hsv
      (bindings map)

  let make_yojson_of_t
      (show_ord : Ord.t -> string)
      (yojson_of_a : 'a -> Yojson.Safe.t)
      (map : 'a t) : Yojson.Safe.t =
    `Assoc
      (bindings map
      |> List.map ~f:(fun (key, value) -> (show_ord key, yojson_of_a value)))
end
