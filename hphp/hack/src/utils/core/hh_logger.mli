(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val timestamp_string : unit -> string

(** enables logging to a file (in addition to stderr which is always enabled) *)
val set_log : string -> out_channel -> unit

val set_id : string -> unit

val print_with_newline :
  ?exn:Exception.t -> ('a, unit, string, unit) format4 -> 'a

val print_duration : string -> float -> float

val exc : ?prefix:string -> stack:string -> exn -> unit

val exception_ : ?prefix:string -> Exception.t -> unit

module Level : sig
  type t =
    | Off
    | Fatal
    | Error
    | Warn
    | Info
    | Debug
  [@@deriving enum]

  val of_enum_string : string -> t option

  val to_enum_string : t -> string

  (** returns the min file log level *)
  val min_level_file : unit -> t

  (** returns the min stderr log level *)
  val min_level_stderr : unit -> t

  (** overwrites min level for both stderr and file (if enabled) *)
  val set_min_level : t -> unit

  (** overwrites min level for file (if enabled), but leaves stderr as is *)
  val set_min_level_file : t -> unit

  (** overwrites min level for stderr, but leaves file (if enabled) as is *)
  val set_min_level_stderr : t -> unit

  (** returns true if t passes either stderr or file min level (regardless whether file is enabled) *)
  val passes_min_level : t -> bool

  val log_duration : t -> string -> float -> float
end

val log : ?lvl:Level.t -> ('a, unit, string, string, string, unit) format6 -> 'a

val log_duration : ?lvl:Level.t -> string -> float -> float

val fatal :
  ?exn:Exception.t -> ('a, unit, string, string, string, unit) format6 -> 'a

val error :
  ?exn:Exception.t -> ('a, unit, string, string, string, unit) format6 -> 'a

val warn :
  ?exn:Exception.t -> ('a, unit, string, string, string, unit) format6 -> 'a

val info :
  ?exn:Exception.t -> ('a, unit, string, string, string, unit) format6 -> 'a

val debug :
  ?exn:Exception.t -> ('a, unit, string, string, string, unit) format6 -> 'a
