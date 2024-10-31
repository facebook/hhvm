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
  `ParserOptions.t` and `GleanOptions.t` which can
  then be retrieved using the `typechecker_options`,  `parser_options`,
  `glean_options` helpers. *)
val load :
  silent:bool ->
  from:string ->
  cli_config_overrides:(string * string) list ->
  ai_options:Ai_options.t option ->
  t * ServerLocalConfig.t * Errors.t

val load_config : Config_file_common.t -> GlobalOptions.t -> GlobalOptions.t

val set_parser_options : t -> ParserOptions.t -> t

val set_tc_options : t -> TypecheckerOptions.t -> t

val set_glean_options : t -> GleanOptions.t -> t

val set_symbol_write_options : t -> SymbolWriteOptions.t -> t

val repo_config_path : Relative_path.t

val is_compatible : t -> t -> bool

val default_config : t

val ignored_paths : t -> Str.regexp list

val extra_paths : t -> Path.t list

val gc_control : t -> Gc.control

val sharedmem_config : t -> SharedMem.config

val typechecker_options : t -> TypecheckerOptions.t

val parser_options : t -> ParserOptions.t

val glean_options : t -> GleanOptions.t

val symbol_write_options : t -> SymbolWriteOptions.t

val formatter_override : t -> Path.t option

val config_hash : t -> string option

val version : t -> Config_file.version

val warn_on_non_opt_build : t -> bool

val ide_fall_back_to_full_index : t -> bool

val convert_auto_namespace_to_map : string -> (string * string) list

val warnings_generated_files : t -> Str.regexp list

val make_sharedmem_config :
  ?ai_options:Ai_options.t ->
  Config_file.t ->
  ServerLocalConfig.t ->
  SharedMem.config
