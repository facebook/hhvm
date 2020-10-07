(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Category : sig
  type t =
    | Decl_accessors
    | Decling
    | Disk_cat
    | Get_ast
    | Typecheck
end

module CategorySet : Set.S with type elt = Category.t

type time_in_sec = float

(** state is a global mutable variable, accumulating all counts *)
type t

(** reset will zero all counters, adjust the global mutable state,
    and return the previous state. You should 'restore_state' when done,
    in case your caller had been doing their own count.
    Categories from [enabled_categories] will be enabled
    and all others will be disabled. *)
val reset : enabled_categories:CategorySet.t -> t

(** restores global mutable state to what it was before you called 'reset' *)
val restore_state : t -> unit

val count : Category.t -> (unit -> 'a) -> 'a

val count_decl_accessor : (unit -> 'a) -> 'a

val count_disk_cat : (unit -> 'a) -> 'a

val count_get_ast : (unit -> 'a) -> 'a

val count_typecheck : (unit -> 'a) -> 'a

val get_counters : unit -> Telemetry.t

val read_time : Category.t -> time_in_sec
