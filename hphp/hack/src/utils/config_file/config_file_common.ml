(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Option.Monad_infix

type t = Config_file_ffi_externs.config

let file_path_relative_to_repo_root = ".hhconfig"

let empty = Config_file_ffi_externs.empty

let is_empty = Config_file_ffi_externs.is_empty

let print_to_stderr (config : t) : unit =
  Config_file_ffi_externs.print_to_stderr config

let apply_overrides ~(config : t) ~(overrides : t) ~(log_reason : string option)
    : t =
  if is_empty overrides then
    config
  else
    let config = Config_file_ffi_externs.apply_overrides config overrides in
    Option.iter log_reason ~f:(fun from ->
        Printf.eprintf "*** Overrides from %s:\n%!" from;
        print_to_stderr overrides;
        Printf.eprintf "\n%!");
    config

(*
 * Config file format:
 * # Some comment. Indicate by a pound sign at the start of a new line
 * key = a possibly space-separated value
 *)
let parse_contents (contents : string) : t =
  Config_file_ffi_externs.parse_contents contents

let parse (fn : string) : string * t =
  let contents = Sys_utils.cat fn in
  let parsed = parse_contents contents in
  let hash = Sha1.digest contents in
  (hash, parsed)

let parse_local_config (fn : string) : t =
  try
    let (_hash, config) = parse fn in
    config
  with
  | e ->
    Hh_logger.log "Loading config exception: %s" (Exn.to_string e);
    Hh_logger.log "Could not load config at %s" fn;
    empty ()

let to_json t =
  match Config_file_ffi_externs.to_json t with
  | Ok json -> Hh_json.json_of_string json
  | Error e -> failwith e

let of_list = Config_file_ffi_externs.of_list

let keys = Config_file_ffi_externs.keys

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

module Getters : Getters_S = struct
  let ok_or_invalid_arg = function
    | Ok x -> x
    | Error e -> invalid_arg e

  let string_opt key config = Config_file_ffi_externs.get_string_opt config key

  (* Does not immediately raise if key is not an int *)
  let int_opt_result key config = Config_file_ffi_externs.get_int_opt config key

  let int_opt key config =
    Config_file_ffi_externs.get_int_opt config key
    |> Option.map ~f:ok_or_invalid_arg

  let float_opt key config =
    Config_file_ffi_externs.get_float_opt config key
    |> Option.map ~f:ok_or_invalid_arg

  let bool_opt key config =
    Config_file_ffi_externs.get_bool_opt config key
    |> Option.map ~f:ok_or_invalid_arg

  let string_list_opt key config =
    Config_file_ffi_externs.get_string_list_opt config key

  let int_list_opt key config : int list option =
    try string_list_opt key config >>| List.map ~f:Int.of_string with
    | Failure _ -> None

  let string_ key ~default config =
    Option.value (string_opt key config) ~default

  let int_ key ~default config = Option.value (int_opt key config) ~default

  let float_ key ~default config = Option.value (float_opt key config) ~default

  let bool_ key ~default config = Option.value (bool_opt key config) ~default

  let string_list key ~default config =
    Option.value (string_list_opt key config) ~default

  let bool_if_min_version key ~default ~current_version config : bool =
    let version_value = string_ key ~default:(string_of_bool default) config in
    match version_value with
    | "true" -> true
    | "false" -> false
    | version_value ->
      let version_value =
        Config_file_version.parse_version (Some version_value)
      in
      if Config_file_version.compare_versions current_version version_value >= 0
      then
        true
      else
        false
end
