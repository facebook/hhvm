(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = Pos.t [@@deriving eq, ord, show]

let none : t = Pos.none

let btw = Pos.btw

let get_raw_pos : t -> Pos.t option = (fun p -> Some p)

let of_raw_pos : Pos.t -> t = (fun p -> p)

let make_decl_pos : Pos.t -> Decl_reference.t -> t =
 (fun p _decl -> (* TODO *) of_raw_pos p)

let is_hhi : t -> bool =
 fun p ->
  match get_raw_pos p with
  | None -> (* TODO T81321312 *) false
  | Some p -> Pos.is_hhi p

let set_from_reason : t -> t = Pos.set_from_reason
