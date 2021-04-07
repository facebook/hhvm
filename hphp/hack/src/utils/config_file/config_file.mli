(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = string SMap.t

type version_components = {
  major: int;
  minor: int;
  build: int;
}

type version =
  | Opaque_version of string option
  | Version_components of version_components

val version_to_string_opt : ?pad:bool -> version -> string option

val file_path_relative_to_repo_root : string

val compare_versions : version -> version -> int

val parse_version : string option -> version

val parse_hhconfig : silent:bool -> string -> string * string SMap.t

val parse_local_config : silent:bool -> string -> string SMap.t

val apply_overrides :
  silent:bool ->
  config:string SMap.t ->
  overrides:string SMap.t ->
  string SMap.t

module Getters : sig
  val string_opt :
    string -> ?prefix:string option -> string SMap.t -> string option

  val string_ :
    string -> ?prefix:string option -> default:string -> string SMap.t -> string

  val int_opt : string -> ?prefix:string option -> string SMap.t -> int option

  val int_ :
    string -> ?prefix:string option -> default:int -> string SMap.t -> int

  val bool_opt : string -> ?prefix:string option -> string SMap.t -> bool option

  val bool_ :
    string -> ?prefix:string option -> default:bool -> string SMap.t -> bool

  val float_opt :
    string -> ?prefix:string option -> string SMap.t -> float option

  val float_ :
    string -> ?prefix:string option -> default:float -> string SMap.t -> float

  val string_list_opt :
    string -> ?prefix:string option -> string SMap.t -> string list option

  val string_list :
    delim:Str.regexp ->
    string ->
    ?prefix:string option ->
    default:string list ->
    string SMap.t ->
    string list

  val bool_if_min_version :
    string ->
    ?prefix:string option ->
    default:bool ->
    current_version:version ->
    string SMap.t ->
    bool
end

module Utils : sig
  val parse_hhconfig_and_hh_conf_to_json :
    root:Path.t -> server_local_config_path:string -> Hh_json.json
end
