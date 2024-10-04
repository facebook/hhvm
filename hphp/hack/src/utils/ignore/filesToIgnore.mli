(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This is invoked in serverMain and other entry points, upon reading .hhconfig ignore_paths=
directive. *)
val ignore_path : Str.regexp -> unit

(** FindUtils.file_filter calls [should_ignore], which consults the list of ignore_path regexps,
to determine whether hh_server recognizes a given file as part of the project. *)
val should_ignore : string -> bool

(** [get_paths_to_ignore] retrieves the current global mutable list of ignore paths,
in preparation for marshalling it to a different proess. *)
val get_paths_to_ignore : unit -> Str.regexp list

(** [set_paths_to_ignore] is for when we've unmarshalled a list of ignore paths,
and wish to store it in the global mutable list of ingore paths. *)
val set_paths_to_ignore : Str.regexp list -> unit

val watchman_monitor_expression_terms : Hh_json.json list

val watchman_server_expression_terms : Hh_json.json list

val watchman_watcher_expression_terms : Hh_json.json list
