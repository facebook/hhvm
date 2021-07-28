(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = string SMap.t

val file_path_relative_to_repo_root : string

val print_config : t -> unit

val apply_overrides : silent:bool -> config:t -> overrides:t -> t

val parse_contents : string -> t

val parse : silent:bool -> string -> string * t

val parse_local_config : silent:bool -> string -> t

module Getters : sig
  val make_key : string -> prefix:string option -> string

  val string_opt : string -> ?prefix:string option -> t -> string option

  val string_ : string -> ?prefix:string option -> default:string -> t -> string

  val int_ : string -> ?prefix:string option -> default:int -> t -> int

  val int_opt : string -> ?prefix:string option -> t -> int option

  val float_ : string -> ?prefix:string option -> default:float -> t -> float

  val float_opt : string -> ?prefix:string option -> t -> float option

  val bool_ : string -> ?prefix:string option -> default:bool -> t -> bool

  val bool_opt : string -> ?prefix:string option -> t -> bool option

  val string_list_opt :
    string -> ?prefix:string option -> t -> string list option

  val string_list :
    delim:Str.regexp ->
    string ->
    ?prefix:string option ->
    default:string list ->
    t ->
    string list

  val bool_if_min_version :
    string ->
    ?prefix:string option ->
    default:bool ->
    current_version:Config_file_version.version ->
    t ->
    bool
end
