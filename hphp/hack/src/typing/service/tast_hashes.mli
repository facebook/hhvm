(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type by_names

type t [@@deriving yojson_of]

val hash_tasts : Tast.by_names -> by_names

(** Return a by_names structure with all hash values equal to -1 *)
val error_while_hashing : Tast.by_names -> by_names

val empty : t

val union : t -> t -> t

val add : t -> key:Relative_path.t -> data:by_names option -> t

val is_enabled : TypecheckerOptions.t -> bool

val map : Relative_path.t -> Tast.by_names -> t

val reduce : t -> t -> t
