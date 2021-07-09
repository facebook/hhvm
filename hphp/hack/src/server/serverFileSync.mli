(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** We maintains state about editor open files and their contents
    - some in ServerEnv.env e.g. editor_open_files, some in global sharedmem like FileProvider,
    some in Diagnostic_subscription.
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

val get_file_content : ServerCommandTypes.file_input -> string

val has_unsaved_changes : ServerEnv.env -> bool

(** Determine which files are different in the IDE and on disk.
    Returns a map from filename to a tuple of ide contents and disk contents. *)
val get_unsaved_changes : ServerEnv.env -> (string * string) Relative_path.Map.t

val toggle_dynamic_view : ServerEnv.env -> bool -> ServerEnv.env
