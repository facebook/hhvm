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

(** Specifies what files we want a file watching service (such as Watchman or
    Edenfs_watcher) to watch.

    A file is watched only if it matches one of the include_ conditions.

    Note that we always ignore files inside of directories owned by version control software.
    (for example .hg, .git .svn)
 *)
type watch_spec = {
  include_extensions: string list;
      (** Includes files based on their extension. Example "php" *)
  include_file_names: string list;
      (** Includes files based on their full name. Example: "PACKAGES.toml" *)
}

val server_watch_spec : watch_spec

val watchman_server_expression_terms : Hh_json.json list

val watchman_watcher_expression_terms : Hh_json.json list
