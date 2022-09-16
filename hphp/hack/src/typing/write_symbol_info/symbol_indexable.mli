(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = Relative_path.t

(** Compute files to index based on list of files provided on the CLI, and
    from a file containing paths. Paths in path files are of the form
      "prefix|rel_path" for instance

        hhi|builtins_fb.hhi
        root|scripts/whatsapp/erl/regd/cfg/list_fb_car *)
val from_options : paths:string list -> paths_file:string option -> t list

(* Get the list of files from the naming table, with possible exclusions *)
val from_naming_table :
  Naming_table.t ->
  failed_parsing:Relative_path.Set.t ->
  exclude_hhi:bool ->
  ignore_paths:string list ->
  t list
