(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type search_result_type =
  | Class of Ast.class_kind option
  | Method of bool * string
  | ClassVar of bool * string
  | Function
  | Typedef
  | Constant

type symbol = (Pos.absolute, search_result_type) SearchUtils.term
type result = symbol list

type info =
  | Full of FileInfo.t
  | Fast of FileInfo.names

val fuzzy_search_enabled : unit -> bool
val set_fuzzy_search_enabled : bool -> unit

val query :
  MultiWorker.worker list option ->
  string ->
  string ->
  fuzzy:bool ->
  (Pos.t, search_result_type) SearchUtils.term list

val query_for_autocomplete :
  string ->
  limit:int option ->
  filter_map:(
    string ->
    string ->
    (FileInfo.pos, search_result_type) SearchUtils.term ->
    'a option) ->
  'a list Utils.With_complete_flag.t

val query_class_methods :
  string ->
  string ->
  (Pos.t, search_result_type) SearchUtils.term list

val update :
  MultiWorker.worker list option ->
  (Relative_path.t * info) list ->
  unit

val remove_files :
  Relative_path.Set.t ->
  unit
