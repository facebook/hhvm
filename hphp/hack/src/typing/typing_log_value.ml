(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Universal representation of an environment, for pretty-printing and delta computation
 *)
type value =
  | Bool of bool
  | Atom of string
  | List of value list
  | Set of SSet.t
  | Map of value SMap.t
[@@deriving eq]

let make_map l = Map (SMap.of_list l)

let bool_as_value v = Bool v

let string_as_value s = Atom s

let smap_as_value f m = Map (SMap.map f m)

let pos_as_value p = string_as_value (Pos.string (Pos.to_absolute p))

let local_id_as_string id =
  Printf.sprintf "%s[#%d]" (Local_id.get_name id) (Local_id.to_int id)

let local_id_set_as_value s =
  Set
    (Local_id.Set.fold
       (fun id s -> SSet.add (local_id_as_string id) s)
       s
       SSet.empty)

let var_as_string v = Printf.sprintf "#%d" v

let varset_as_value s =
  Set (ISet.fold (fun v s -> SSet.add (var_as_string v) s) s SSet.empty)
