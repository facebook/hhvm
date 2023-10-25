(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = private {
  path: Relative_path.t;
  fanout: bool;
}

val from_file : Relative_path.t -> t

(** Compute files to index based on list of files provided on the CLI, and
    from a paths file. Two forms are accepted for lines in [paths_file].

    Legacy format: "prefix|rel_path", e.g.

        hhi|builtins_fb.hhi
        root|scripts/whatsapp/erl/regd/cfg/list_fb_car

    JSON: { path = "prefix|rel_path", fanout = bool }

    If [with_hhi] is provided, results include the built-in hhi files.

    See also [symbol_file_info.mli] for role of [fanout] flag *)
val from_options :
  paths:string list -> paths_file:string option -> include_hhi:bool -> t list

(** Get the list of files from the naming table, with possible exclusions.
    hh should be run in full-init mode if we want the list of all files to index *)
val from_naming_table :
  Naming_table.t -> include_hhi:bool -> ignore_paths:string list -> t list
