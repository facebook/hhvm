(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** We maintains state about editor open files and their contents
    - some in ServerEnv.env e.g. editor_open_files, some in global sharedmem like FileProvider.
    Functions in this module update those states;
    they get invoked when an open/edit/close event happens in the editor.
    These file events are made known to the server via RPC commands. *)

(** Notify hh of a file opening. *)
val open_file :
  predeclare:bool -> ServerEnv.env -> string -> string -> ServerEnv.env

(** Notify hh when first editing a file. *)
val edit_file :
  predeclare:bool ->
  ServerEnv.env ->
  string ->
  File_content.text_edit list ->
  ServerEnv.env

(** Notify hh of closing a file. *)
val close_file : ServerEnv.env -> string -> ServerEnv.env

(** Call this when losing connection with IDE to clear data
    that we have about this IDE. *)
val clear_sync_data : ServerEnv.env -> ServerEnv.env

(** Get file content from File_provider if present, otherwise from disk.
If the argument is [ServerCommandTypes.FileName path] for a path that's not
under Root, or if we get an exception trying to read the file (e.g. because
it has been deleted), then return the empty string. See also
[get_file_content_from_disk] which skips File_provider. *)
val get_file_content : ServerCommandTypes.file_input -> string

(** Get file content from disk. If the path isn't under Root, or we get
an exception trying to read the file (e.g. because it has been deleted) then
return the empty string. See also [get_file_content] which looks up
File_provider first. *)
val get_file_content_from_disk : string -> string

val has_unsaved_changes : ServerEnv.env -> bool

(** Determine which files are different in the IDE and on disk.
    Returns a map from filename to a tuple of ide contents and disk contents. *)
val get_unsaved_changes : ServerEnv.env -> (string * string) Relative_path.Map.t
