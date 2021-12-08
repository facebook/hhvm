(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

val file_path_relative_to_repo_root : string

val empty : unit -> t

val print_to_stderr : t -> unit

val apply_overrides : from:string option -> config:t -> overrides:t -> t

val parse_contents : string -> t

val parse : string -> string * t

val parse_local_config : string -> t

val to_json : t -> Hh_json.json

val of_list : (string * string) list -> t

val keys : t -> string list

module type Getters_S = sig
  val string_opt : string -> t -> string option

  val string_ : string -> default:string -> t -> string

  val int_ : string -> default:int -> t -> int

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
