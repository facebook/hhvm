(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Perform necessary hooks on opening a file. *)
val open_file :
  predeclare:bool -> ServerEnv.env -> string -> string -> ServerEnv.env

val edit_file :
  predeclare:bool ->
  ServerEnv.env ->
  string ->
  File_content.text_edit list ->
  ServerEnv.env

val close_file : ServerEnv.env -> string -> ServerEnv.env

val clear_sync_data : ServerEnv.env -> ServerEnv.env

val get_file_content : ServerCommandTypes.file_input -> string

val has_unsaved_changes : ServerEnv.env -> bool

(* Determine which files are different in the IDE and on disk.
 * Returns a map from filename to a tuple of ide contents and disk contents. *)
val get_unsaved_changes : ServerEnv.env -> (string * string) Relative_path.Map.t

val toggle_dynamic_view : ServerEnv.env -> bool -> ServerEnv.env
