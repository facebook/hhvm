(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core

type 'a t = ('a, unit) Hashtbl.t

let create () = Hashtbl.Poly.create ()

let clear = Hashtbl.clear

let copy = Hashtbl.copy

let add set x = Hashtbl.set set ~key:x ~data:()

let mem = Hashtbl.mem

let remove = Hashtbl.remove

let iter = Hashtbl.iter_keys

let union set ~other = iter other ~f:(add set)

let fold set ~init ~f =
  Hashtbl.fold set ~init ~f:(fun ~key ~data:_ acc -> f key acc)

let filter set ~f =
  let to_remove =
    fold set ~init:[] ~f:(fun elt acc ->
        if not (f elt) then
          elt :: acc
        else
          acc)
  in
  List.iter to_remove ~f:(remove set)

let intersect set ~other = filter ~f:(mem other) set

let length = Hashtbl.length

let is_empty = Hashtbl.is_empty

let to_list = Hashtbl.keys

let of_list list =
  let set = Hashtbl.Poly.create ~size:(List.length list) () in
  List.iter list ~f:(add set);
  set

let yojson_of_t compare_a yojson_of_a t =
  to_list t |> List.sort ~compare:compare_a |> yojson_of_list yojson_of_a
