(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** An OCaml measurement record. Functions with an optional [record] argument
    default to the current OCaml global record. To include active Rust
    measurements, pass a record returned by [peek_global] or [pop_global]. *)
type record

type record_data

val create : unit -> record

(** Start a fresh measurement scope in both OCaml and Rust. *)
val push_global : unit -> unit

(** Remove the current OCaml and Rust scopes and return their combined record.
    The returned measurements are not merged into the enclosing scope. *)
val pop_global : unit -> record

(** Copy and combine the current OCaml and Rust records without modifying
    either record or stack. The returned record is independent of both. *)
val peek_global : unit -> record

val serialize : record -> record_data

val deserialize : record_data -> record

val track_distribution : ?record:record -> string -> bucket_size:float -> unit

val sample : ?record:record -> ?weight:float -> string -> float -> unit

val time : ?record:record -> string -> (unit -> 'a) -> 'a

val delete : ?record:record -> string -> unit

val merge : ?record:record -> record -> unit

val get_sum : ?record:record -> string -> float option

val get_mean : ?record:record -> string -> float option

val get_count : ?record:record -> string -> float option

val get_max : ?record:record -> string -> float option

val print_entry_stats :
  ?record:record -> ?print_raw:(string -> unit) -> string -> unit

val print_stats : ?record:record -> ?print_raw:(string -> unit) -> unit -> unit

val stats_to_telemetry : ?record:record -> unit -> Telemetry.t

val print_entry_distribution : ?record:record -> string -> unit

val print_distributions : ?record:record -> unit -> unit
