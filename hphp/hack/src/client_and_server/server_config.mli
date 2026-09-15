(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t [@@deriving show]

(** Load config from these sources (last overrides first):
  - .hhconfig file in the root directory.
  - Flags passed via CLI option `--config`. These are retrieved from [cli_config_overrides].

  Also calls into `ServerLocalConfigLoad.load`.

  Those two configs get mingled to make a `TypecheckerOptions.t`,
  `ParserOptions.t` and `Glean_options.t` which can
  then be retrieved using the `typechecker_options`,  `parser_options`,
  `glean_options` helpers. *)
val load :
  silent:bool ->
  from:string ->
  cli_config_overrides:(string * string) list ->
  t * Server_local_config.t

(** As [load], with [apply_dynamic_overrides] applied after JustKnobs and before
    SandboxExperiment while loading the local config. *)
val load_with_dynamic_overrides :
  apply_dynamic_overrides:
    (silent:bool -> Config_file_common.t -> Config_file_common.t) ->
  silent:bool ->
  from:string ->
  cli_config_overrides:(string * string) list ->
  t * Server_local_config.t

val load_config : Config_file_common.t -> GlobalOptions.t -> GlobalOptions.t

val set_parser_options : t -> Parser_options.t -> t

val set_tc_options : t -> Typechecker_options.t -> t

val set_glean_options : t -> Glean_options.t -> t

val set_symbol_write_options : t -> Symbol_write_options.t -> t

val repo_config_path : Relative_path.t

val is_compatible : t -> t -> bool

val default_config : t

val ignored_paths : t -> Str.regexp list

val extra_paths : t -> Path.t list

val gc_control : t -> Gc.control

val sharedmem_config : t -> SharedMem.config

val typechecker_options : t -> Typechecker_options.t

val parser_options : t -> Parser_options.t

val glean_options : t -> Glean_options.t

val symbol_write_options : t -> Symbol_write_options.t

val formatter_override : t -> Path.t option

val config_hash : t -> string option

val version : t -> Config_file.version

val warn_on_non_opt_build : t -> bool

val ide_fall_back_to_full_index : t -> bool

val convert_auto_namespace_to_map : string -> (string * string) list

val warnings_generated_files : t -> string list

val make_sharedmem_config :
  Config_file.t -> Server_local_config.t -> SharedMem.config

val update_config_with_ai_options :
  t -> Server_local_config.t -> Ai_options.t option -> t * Server_local_config.t

(** Validate CLI --config key=value overrides. Logs a warning via Hh_logger
    for each key that is not a recognized config option, with a "did you mean?"
    suggestion when there is a close match. Does not fail. *)
val warn_on_invalid_config_keys : (string * string) list -> unit

(** Validate keys in a parsed .hhconfig file. Logs a warning via Hh_logger
    for each key that is not a recognized .hhconfig option. Does not fail. *)
val warn_on_invalid_hhconfig_keys : Config_file_common.t -> unit
