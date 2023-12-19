(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type value =
  | Bool of bool
  | Atom of string
  | List of value list
  | Set of SSet.t
  | Map of value SMap.t
  | Type of Typing_defs.internal_type
  | SubtypeProp of Typing_logic.subtype_prop
[@@deriving eq]

let make_map l = Map (SMap.of_list l)

let bool_as_value v = Bool v

let string_as_value s = Atom s

let smap_as_value f m = Map (SMap.map f m)

let list_as_value l = List l

let internal_type_as_value ty = Type ty

let locl_type_as_value ty = internal_type_as_value (Typing_defs.LoclType ty)

let internal_type_set_as_value tys =
  Internal_type_set.elements tys
  |> List.map internal_type_as_value
  |> list_as_value

let subtype_prop_as_value prop = SubtypeProp prop

let pos_as_value p = string_as_value (Pos.string (Pos.to_absolute p))

let reason_as_value r =
  string_as_value (Format.asprintf "%a" Typing_reason.pp_t_ r)

let local_id_as_string id =
  Printf.sprintf "%s[#%d]" (Local_id.get_name id) (Local_id.to_int id)

let local_id_set_as_value s =
  Set
    (Local_id.Set.fold
       (fun id s -> SSet.add (local_id_as_string id) s)
       s
       SSet.empty)

let var_as_string (v : Tvid.t) = Printf.sprintf "#%s" (Tvid.show v)

let varset_as_value s =
  Set (Tvid.Set.fold (fun v s -> SSet.add (var_as_string v) s) s SSet.empty)

let variant_as_value name v = make_map [(name, v)]
