(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type config

external empty : unit -> config = "hh_config_file_empty"

external is_empty : config -> bool = "hh_config_file_is_empty" [@@noalloc]

external parse_contents : string -> config = "hh_config_file_parse_contents"

external print_to_stderr : config -> unit = "hh_config_file_print_to_stderr"
  [@@noalloc]

external apply_overrides : config -> config -> config
  = "hh_config_file_apply_overrides"

external to_json : config -> (string, string) result = "hh_config_file_to_json"

external of_list : (string * string) list -> config = "hh_config_file_of_list"

external keys : config -> string list = "hh_config_file_keys"

external get_string_opt : config -> string -> string option
  = "hh_config_file_get_string_opt"
