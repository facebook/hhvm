(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type decls = (string * Shallow_decl_defs.decl) list [@@deriving show]

type parsed_file = {
  pf_mode: FileInfo.mode option;
  pf_file_attributes: Typing_defs.user_attribute list;
  pf_decls: decls;
  pf_has_first_pass_parse_errors: bool;
}

type parsed_file_with_hashes = {
  pfh_mode: FileInfo.mode option;
  pfh_hash: FileInfo.pfh_hash;
      (** position insensitive hash of all decls in the file *)
  pfh_decls: (string * Shallow_decl_defs.decl * Int64.t) list;
      (** (name, decl, position-sensitive hash of this decl) *)
}

(** NOTE: this doesn't respect deregister_php_lib, and has decls in reverse lexical order. *)
val parse_decls :
  DeclParserOptions.t -> Relative_path.t -> string -> parsed_file

(** NOTE: this doesn't respect deregister_php_lib, and has decls in reverse lexical order *)
val parse_and_hash_decls :
  DeclParserOptions.t ->
  bool ->
  Relative_path.t ->
  string ->
  parsed_file_with_hashes

(** NOTE: this takes input in reverse-lexical-order, and emits FileInfo.t in forward lexical order *)
val decls_to_fileinfo : Relative_path.t -> parsed_file_with_hashes -> FileInfo.t

val decls_to_addenda : parsed_file_with_hashes -> FileInfo.si_addendum list

module type Metadata = sig
  type t
end

module Concurrent (Metadata : Metadata) : sig
  type handle

  (** For concurrent parsing of files, you can either pass [content=None] to
  have the concurrent worker read the file off disk, or pass [content=Some]
  to have the worker use this content. This is useful for [Provider_context.entry]
  where we want to parse unsaved IDE content. *)
  type content = string option

  (** This does no work, and doesn't spawn any threads. It merely
  stores [opts, root, hhi, ...] in a structure, and allocates a concurrent channel. *)
  val start :
    opts:DeclParserOptions.t ->
    root:Path.t ->
    hhi:Path.t ->
    tmp:Path.t ->
    dummy:Path.t ->
    handle

  (** This causes the listed relative-paths to go in a per-handle queue to be
  direct-decl-parsed. You can trust that multiple cores will be used
  to do that parsing. This function will block until the next
  available path has been parsed -- it might be one of the paths
  we passed into this call, or it might even have been an earlier call.

  It is normal to invoke this function with an empty list, indicating that
  the caller has no further workitems they wish to add, but they still
  want to finish collecting results from previous calls.

  This function will return [Some] with a result, or [None] if there
  will be no further results (i.e. if all pending paths have been done).

  The caller can provide metadata. For a given [(path, metadata)] that's passed in,
  it will be returned (either on this call or a subsequent call) with the exact
  same [(path, metadata)]. *)
  val enqueue_next_and_get_earlier_results :
    handle ->
    (Relative_path.t * Metadata.t * content) list ->
    (Relative_path.t * Metadata.t * parsed_file) option
end
