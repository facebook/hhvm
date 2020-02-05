(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module PositionedSyntaxTree : sig
  include module type of
      Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
end

(** The information associated with a given file. *)
type entry = {
  file_input: ServerCommandTypes.file_input;
  path: Relative_path.t;
  source_text: Full_fidelity_source_text.t;
  comments: Parser_return.comments;
  ast: Nast.program;
  ast_errors: Errors.t;
  mutable cst: PositionedSyntaxTree.t option;
  mutable tast: Tast.program option;
  mutable tast_errors: Errors.t option;
  mutable symbols: Relative_path.t SymbolOccurrence.t list option;
}

(** A context mapping from file to the [entry] for that file.

This acts as an "overlay" or "delta" on the state of the world, relative to the
files that exist in the repo on disk.

To load this state of the world for use in a given operation, use
[Provider_utils.respect_but_quarantine_unsaved_changes]. *)
type t = {
  tcopt: TypecheckerOptions.t;
  backend: Provider_backend.t;
  entries: entry Relative_path.Map.t;
}

(** The empty context, denoting no delta from the current state of the world. *)
val empty : tcopt:TypecheckerOptions.t -> t

(** Update the [TypecheckerOptions.t] contained within the [t]. *)
val map_tcopt : t -> f:(TypecheckerOptions.t -> TypecheckerOptions.t) -> t

(** Returns a [ServerCommandTypes.file_input] corresponding to the given [path].

If the [path] is in the context, returns its associated
[ServerCommandTypes.FileContent]. Otherwise returns the
[ServerCommandTypes.FileName] corresponding to that file on disk. *)
val get_file_input :
  ctx:t -> path:Relative_path.t -> ServerCommandTypes.file_input

(** Get the [FileInfo.t] associated with the given [entry]. *)
val get_fileinfo : entry:entry -> FileInfo.t

(** Get the current global context (which is set with
[Provider_utils.respect_but_quarantine_unsaved_changes]), if any. Only one
global context can be set at a time. *)
val get_global_context : unit -> t option

(** Like [get_global_context], but returns the empty context if no global
context was set. *)
val get_global_context_or_empty_FOR_MIGRATION : unit -> t

(** Internal functions **)

(** Set the current global context. Should not be used directly; use
[Provider_utils.respect_but_quarantine_unsaved_changes] instead. *)
val set_global_context_internal : t -> unit

(** Unset the current global context. Should not be used directly; use
[Provider_utils.respect_but_quarantine_unsaved_changes] instead. *)
val unset_global_context_internal : unit -> unit
