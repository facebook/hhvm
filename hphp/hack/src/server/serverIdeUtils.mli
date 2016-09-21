(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* When typechecking a content buffer in IDE mode,
* this is the path that will be assigned to it *)
val path: Relative_path.t

(* Runs the typecheck phase on a single file. *)
val check_file_input :
  TypecheckerOptions.t ->
  (* What are the definitions in each file. Most likely coming from
   * ServerEnv.env.files_info *)
  FileInfo.t Relative_path.Map.t ->
  (* File path or content buffer. When given file path, the mapping in file_info
   * is used to find the declarations and run only the typechecking phase.
   * When given content buffer it will be parsed, named, and declared before
   * that. The declarations will be removed from shared memory afterwards. *)
  ServerUtils.file_input ->
  Relative_path.t

(* Parses, names, declares and typechecks the content buffer, then run f
 * while the declared definitions are still available in shared memory.
 * The declarations will be removed from shared memory afterwards. *)
val declare_and_check : string ->
  f:(Relative_path.t -> FileInfo.t -> 'a) -> 'a

(* Run the typing phase on a list of files and definitions they contain. *)
val recheck :
  TypecheckerOptions.t ->
  (Relative_path.t * FileInfo.t) list -> unit
