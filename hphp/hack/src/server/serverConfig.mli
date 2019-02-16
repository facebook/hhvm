(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
type t

val set_parser_options : t -> ParserOptions.t -> t
val set_tc_options : t -> TypecheckerOptions.t -> t
val filename : Relative_path.t
val load : Relative_path.t -> ServerArgs.options -> t * ServerLocalConfig.t
val is_compatible : t -> t -> bool

val default_config : t

val ignored_paths       : t -> string list
val extra_paths         : t -> Path.t list
val coroutine_whitelist_paths : t -> string list
val gc_control          : t -> Gc.control
val sharedmem_config    : t -> SharedMem.config
val typechecker_options : t -> TypecheckerOptions.t
val parser_options      : t -> ParserOptions.t
val formatter_override  : t -> Path.t option
val config_hash         : t -> string option
val version             : t -> string option

val convert_auto_namespace_to_map : string -> (string * string) list
