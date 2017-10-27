(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t

val set_parser_options : t -> ParserOptions.t -> t
val set_tc_options : t -> TypecheckerOptions.t -> t
val filename : Relative_path.t
val load : Relative_path.t -> ServerArgs.options -> t * ServerLocalConfig.t
val is_compatible : t -> t -> bool

val default_config : t

val load_script         : t -> Path.t option
val load_script_timeout : t -> int
val load_mini_script    : t -> Path.t option
val ignored_paths       : t -> string list
val gc_control          : t -> Gc.control
val sharedmem_config    : t -> SharedMem.config
val state_prefetcher_script : t -> Path.t option
val typechecker_options : t -> TypecheckerOptions.t
val parser_options      : t -> ParserOptions.t
val formatter_override  : t -> Path.t option
val config_hash         : t -> string option
