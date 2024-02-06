(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type pattern

type result

(** Compile JSON input into a pattern that can be searched for. *)
val compile_pattern :
  Provider_context.t -> Hh_json.json -> (pattern, string) Result.t

(** Convert the result of a search into JSON output that can be sent back to the
    user. *)
val result_to_json : sort_results:bool -> result option -> Hh_json.json

(** Search for the given pattern across the given set of files. *)
val go :
  ServerEnv.genv ->
  ServerEnv.env ->
  sort_results:bool ->
  files_to_search:string list option ->
  Hh_json.json ->
  (Hh_json.json, string) Result.t

(** Execute a search on a single syntax tree. This is most useful in debugging
    utilities like `hh_single_type_check`. *)
val search :
  Provider_context.t -> Provider_context.entry -> pattern -> result option
