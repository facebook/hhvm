(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type db_path = Db_path of string [@@deriving show]

type insertion_error = {
  canon_hash: Int64.t option;
  hash: Int64.t;
  kind_of_type: Naming_types.kind_of_type option;
  name: string;
  origin_exception: Exception.t;
      [@printer (fun fmt e -> fprintf fmt "%s" (Exception.get_ctor_string e))]
}
[@@deriving show]

type save_result = {
  files_added: int;
  symbols_added: int;
  errors: insertion_error list;
}
[@@deriving show]

val empty_save_result : save_result

type 'a forward_naming_table_delta =
  | Modified of 'a
  | Deleted
[@@deriving show]

type file_deltas = FileInfo.t forward_naming_table_delta Relative_path.Map.t

type local_changes = {
  file_deltas: file_deltas;
  base_content_version: string;
}
[@@deriving show]

val validate_can_open_db : db_path -> unit

val free_db_cache : unit -> unit

val save_file_infos :
  string ->
  FileInfo.t Relative_path.Map.t ->
  base_content_version:string ->
  save_result

val copy_and_update :
  existing_db:db_path -> new_db:db_path -> local_changes -> unit

val get_local_changes : db_path -> local_changes

val fold :
  db_path:db_path ->
  init:'a ->
  f:(Relative_path.t -> FileInfo.t -> 'a -> 'a) ->
  file_deltas:file_deltas ->
  'a

val get_file_info : db_path -> Relative_path.t -> FileInfo.t option

val get_type_pos :
  db_path -> string -> (Relative_path.t * Naming_types.kind_of_type) option

val get_itype_pos :
  db_path -> string -> (Relative_path.t * Naming_types.kind_of_type) option

val get_type_paths_by_dep_hash :
  db_path -> Typing_deps.Dep.t -> Relative_path.Set.t

val get_fun_pos : db_path -> string -> Relative_path.t option

val get_ifun_pos : db_path -> string -> Relative_path.t option

val get_fun_paths_by_dep_hash :
  db_path -> Typing_deps.Dep.t -> Relative_path.Set.t

val get_const_pos : db_path -> string -> Relative_path.t option

val get_const_paths_by_dep_hash :
  db_path -> Typing_deps.Dep.t -> Relative_path.Set.t

(** The canonical name (and assorted *Canon heaps) store the canonical name for a
    symbol, keyed off of the lowercase version of its name. We use the canon
    heaps to check for symbols which are redefined using different
    capitalizations so we can throw proper Hack errors for them. *)
val to_canon_name_key : string -> string
