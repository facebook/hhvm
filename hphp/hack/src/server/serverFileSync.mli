(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

val open_file : ServerEnv.env -> string -> string -> ServerEnv.env

val edit_file :
  ServerEnv.env ->
  string ->
  Ide_api_types.text_edit list ->
  ServerEnv.env

val close_file : ServerEnv.env -> string -> ServerEnv.env

val clear_sync_data : ServerEnv.env -> ServerEnv.env

val try_relativize_path : string -> Relative_path.t option

val get_file_content : ServerUtils.file_input -> string

val has_unsaved_changes : ServerEnv.env -> bool

(* Determine which files are different in the IDE and on disk.
 * Returns a map from filename to a tuple of ide contents and disk contents. *)
val get_unsaved_changes : ServerEnv.env -> (string * string) Relative_path.Map.t
