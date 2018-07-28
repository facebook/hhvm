(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

type t = (string, SSet.t) Hashtbl.t

let make () = Hashtbl.create 100

let get index search_term =
  match Hashtbl.find_opt index search_term with
  | Some set -> set
  | None -> SSet.empty

let update index name terms =
  List.iter terms ~f:(fun term ->
    let functions = SSet.add name (get index term) in
    Hashtbl.replace index term functions
  )
