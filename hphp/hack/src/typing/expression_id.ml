(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = Ident_provider.Ident.t [@@deriving eq, hash, ord, show]

type provider = Ident_provider.t

let make provider = Ident_provider.provide provider

module Map = Ident_provider.Ident.Map

(** This is a mapping from internal expression ids to a standardized int.
    Used for outputting cleaner error messages to users.
    Thanks to this map, error messages will start counting expression IDs out 1
    and use consecutive integers instead of using seemingly random
    integers. *)
let display_id_map = ref Map.empty

let make_provider () =
  display_id_map := Map.empty;
  Ident_provider.init ()

let make_immutable t = Ident_provider.Ident.make_immutable t

let is_immutable t = Ident_provider.Ident.is_immutable t

let display_in_error id =
  let id =
    let map = !display_id_map in
    match Map.find_opt id map with
    | Some n -> n
    | None ->
      let n = Map.cardinal map + 1 in
      display_id_map := Map.add id n map;
      n
  in
  Printf.sprintf "<expr#%d>" id

let get_expr_display_id_map () = !display_id_map

let debug x = show x
