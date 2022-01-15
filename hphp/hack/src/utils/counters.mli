(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Category : sig
  (** Care: each of these entries adds up to telemetry in [HackEventLogger.ProfileTypeCheck.get_stats],
  and we should keep the number small... 5 to 10 items here are fine, but more will be a problem. *)
  type t =
    | Decling
        (** Decling is used when --config profile_decling=TopCounts; it's counted by all [Typing_classes_heap] decl accessors.
            It measures cpu-time. *)
    | Disk_cat
        (** Disk_cat is counted every use of [Disk.cat] and measures wall-time. *)
    | Get_ast
        (** Get_ast is counted for [Ast_provider.get_ast_with_error], fetching the full ASTs of the files we're typechecking.
            It is notionally inclusive of Disk_cat time, but it measures cpu-time and so
            doesn't count the time spent waiting for IO. *)
    | Get_decl
        (** Get_decl is counted for [Direct_decl_utils.direct_decl_parse], fetching decls off disk of the types we depend upon.
            It is notionally inclusive of Disk_cat time, but it measures cpu-time and so
            doesn't count the time spent waiting for IO. *)
    | Typecheck
        (** Typecheck is counted for [Typing_top_level], typechecking top-level entity. It is inclusive of Get_ast and Get_decl time.
            It measures cpu-time. *)
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
val reset : unit -> t

(** restores global mutable state to what it was before you called 'reset' *)
val restore_state : t -> unit

val count : Category.t -> (unit -> 'a) -> 'a

val get_counters : unit -> Telemetry.t

val read_time : Category.t -> time_in_sec
