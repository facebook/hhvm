(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** enables logging to a file (in addition to stderr which is always enabled) *)
val set_log : string -> unit

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

  (** overwrites the set of categories that be checked for log level before
      being output; when logs are logged with no category, then only the log
      level is used to decide whether the log entry should be output;
      if a category is specified when logging, then this list of categories
      will be checked in addition to the log level *)
  val set_categories : string list -> unit

  (** returns true if t passes either stderr or file min level (regardless whether file is enabled) *)
  val passes_min_level : t -> bool

  (** logs the message and how long the presumed operation took, assuming that
      the float argument is the start time and that the end time is now *)
  val log_duration : t -> ?category:string -> string -> float -> float
end

val log :
  ?lvl:Level.t ->
  ?category:string ->
  ('a, unit, string, string, string, unit) format6 ->
  'a

val log_lazy : ?lvl:Level.t -> ?category:string -> string lazy_t -> unit

val log_duration : ?lvl:Level.t -> ?category:string -> string -> float -> float

val fatal :
  ?category:string ->
  ?exn:Exception.t ->
  ('a, unit, string, string, string, unit) format6 ->
  'a

val error :
  ?category:string ->
  ?exn:Exception.t ->
  ('a, unit, string, string, string, unit) format6 ->
  'a

val warn :
  ?category:string ->
  ?exn:Exception.t ->
  ('a, unit, string, string, string, unit) format6 ->
  'a

val info :
  ?category:string ->
  ?exn:Exception.t ->
  ('a, unit, string, string, string, unit) format6 ->
  'a

val debug :
  ?category:string ->
  ?exn:Exception.t ->
  ('a, unit, string, string, string, unit) format6 ->
  'a
