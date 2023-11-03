(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* HashSet is just a HashTable where the keys are actually the values, and we
 * ignore the actual values inside the HashTable. *)
type 'a t

val create : unit -> 'a t

val clear : 'a t -> unit

val copy : 'a t -> 'a t

val add : 'a t -> 'a -> unit

val union : 'a t -> other:'a t -> unit

val mem : 'a t -> 'a -> bool

val remove : 'a t -> 'a -> unit

val filter : 'a t -> f:('a -> bool) -> unit

val intersect : 'a t -> other:'a t -> unit

val iter : 'a t -> f:('a -> unit) -> unit

val fold : 'a t -> init:'b -> f:('a -> 'b -> 'b) -> 'b

val length : 'a t -> int

val is_empty : 'a t -> bool

val to_list : 'a t -> 'a list

val of_list : 'a list -> 'a t

val yojson_of_t :
  ('a -> 'a -> int) -> ('a -> Yojson.Safe.t) -> 'a t -> Yojson.Safe.t
