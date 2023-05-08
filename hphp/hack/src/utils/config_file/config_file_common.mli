(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

val file_path_relative_to_repo_root : string

val pkgs_config_path_relative_to_repo_root : string

val get_packages_absolute_path : hhconfig_path:string -> string

val empty : unit -> t

val print_to_stderr : t -> unit

(** Apply overrides using provided overrides.
[log_reason] is solely used for logging, so we can write to stderr indicating where
these overrides came from and what they were. *)
val apply_overrides : config:t -> overrides:t -> log_reason:string option -> t

val parse_contents : string -> t

val parse_local_config : string -> t

val to_json : t -> Hh_json.json

val of_list : (string * string) list -> t

val keys : t -> string list

module type Getters_S = sig
  val string_opt : string -> t -> string option

  val string_ : string -> default:string -> t -> string

  val int_ : string -> default:int -> t -> int

  val int_opt_result : string -> t -> (int, string) result option

  val int_opt : string -> t -> int option

  val float_ : string -> default:float -> t -> float

  val float_opt : string -> t -> float option

  val bool_ : string -> default:bool -> t -> bool

  val bool_opt : string -> t -> bool option

  val string_list_opt : string -> t -> string list option

  val string_list : string -> default:string list -> t -> string list

  val int_list_opt : string -> t -> int list option

  val bool_if_min_version :
    string ->
    default:bool ->
    current_version:Config_file_version.version ->
    t ->
    bool
end

module Getters : Getters_S
