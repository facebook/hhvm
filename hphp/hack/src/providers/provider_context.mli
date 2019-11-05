(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** The information associated with a given file. *)
type entry = {
  file_input: ServerCommandTypes.file_input;
  path: Relative_path.t;
  ast: Nast.program;
  tast: Tast.program option ref;
  errors: Errors.t option ref;
}

(** A context mapping from file to the [entry] for that file.

This acts as an "overlay" or "delta" on the state of the world, relative to the
files that exist in the repo on disk.

To load this state of the world for use in a given operation, use
[ServerIdeUtils.with_context]. *)
type t = {
  tcopt: TypecheckerOptions.t;
  entries: entry Relative_path.Map.t;
}

(** The empty context, denoting no delta from the current state of the world. *)
val empty : tcopt:TypecheckerOptions.t -> t

(** Returns a [ServerCommandTypes.file_input] corresponding to the given [path].

If the [path] is in the context, returns its associated
[ServerCommandTypes.FileContent]. Otherwise returns the
[ServerCommandTypes.FileName] corresponding to that file on disk. *)
val get_file_input :
  ctx:t -> path:Relative_path.t -> ServerCommandTypes.file_input

(** Get the [FileInfo.t] associated with the given [entry]. *)
val get_fileinfo : entry:entry -> FileInfo.t

(** Get the current global context (which is set with
[ServerIdeUtils.with_context]), if any. Only one global context can be set at a
time. *)
val get_global_context : unit -> t option

(** Internal functions **)

(** Set the current global context. Should not be used directly; use
[ServerIdeUtils.with_context] instead. *)
val set_global_context_internal : t -> unit

(** Unset the current global context. Should not be used directly; use
[ServerIdeUtils.with_context] instead. *)
val unset_global_context_internal : unit -> unit
