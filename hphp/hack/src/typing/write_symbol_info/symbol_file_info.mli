(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Indexable = Symbol_indexable
module Sym_def = Symbol_sym_def

type symbol = private {
  occ: Relative_path.t SymbolOccurrence.t;
  def: Sym_def.t option;
}

type container = {
  name: string;
  kind: Ast_defs.classish_kind;
}

type member = { name: string }

type member_cluster = {
  container: container;
  methods: member list;
  properties: member list;
  class_constants: member list;
  type_constants: member list;
}

(** [fanout] flag is used in incremental mode. It identifies files which are
    unchanged compared to the base db. Such files don't need to be re-indexed
   if an identical [sym_hash] exists on the base db. [sym_hash] captures the
   file path and elements of this [File_info.t] which may be invalidated
   when *other* files are changed
   (e.g. referenced symbols, inherited members) *)
type t = private {
  (* path to be used in the glean index *)
  path: string;
  cst: Full_fidelity_positioned_syntax.t;
  tast: (Tast.def * member_cluster list) list;
  source_text: Full_fidelity_source_text.t;
  symbols: symbol list;
  sym_hash: Md5.t option;
  fanout: bool;
}

(** all the root (i.e. non-hhi) files referenced by t through xrefs. t
   must have been created using [sym_path] set to [true], otherwise
   the result set is empty *)
val referenced : t -> SSet.t

(** If [gen_sym_hash] is true, computes the [sym_hash] for this file. This
  is needed for incremental indexing, when indexing bases, or increments.
  If [gen_references] is true, computes the path of all referenced symbols. *)
val create :
  Provider_context.t ->
  Indexable.t ->
  gen_sym_hash:bool ->
  gen_references:bool ->
  root_path:string ->
  hhi_path:string ->
  t
