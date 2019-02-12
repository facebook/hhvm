(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
type pattern
type result

val compile_pattern:
  Hh_json.json ->
  (pattern, string) Result.t
(** Compile JSON input into a pattern that can be searched for. *)

val result_to_json: sort_results:bool -> result option -> Hh_json.json
(** Convert the result of a search into JSON output that can be sent back to the
    user. *)

val go:
  ServerEnv.genv ->
  ServerEnv.env ->
  sort_results:bool ->
  files_to_search:string list option  ->
  Hh_json.json ->
  (Hh_json.json, string) Result.t
(** Search for the given pattern across the given set of files. *)

val search:
  TypecheckerOptions.t ->
  Relative_path.t ->
  FileInfo.t ->
  pattern ->
  result option
(** Execute a search on a single syntax tree. This is most useful in debugging
    utilities like `hh_single_type_check`. *)
