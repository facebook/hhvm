(**
 * Copyright (c) Facebook, Inc.
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
  | Type of Typing_defs.internal_type
  | SubtypeProp of Typing_logic.subtype_prop
[@@deriving eq]

val make_map : (SMap.key * value) list -> value

val bool_as_value : bool -> value

val string_as_value : string -> value

val smap_as_value : ('a -> value) -> 'a SMap.t -> value

val list_as_value : value list -> value

val locl_type_as_value : Typing_defs.locl_ty -> value

val internal_type_set_as_value : Internal_type_set.t -> value

val subtype_prop_as_value : Typing_logic.subtype_prop -> value

val pos_as_value : Pos.t -> value

val reason_as_value : 'a Typing_reason.t_ -> value

val local_id_as_string : Local_id.t -> string

val local_id_set_as_value : Local_id.Set.t -> value

val var_as_string : Tvid.t -> string

val varset_as_value : Tvid.Set.t -> value

val variant_as_value : SMap.key -> value -> value
