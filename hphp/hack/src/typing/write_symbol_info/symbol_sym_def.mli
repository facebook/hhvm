(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This is mostly a wrapper around ServerSymbolDefinition, specialized to
    what the indexer needs. Currently, resolve, based on ServerSymbolDefinition.go,
    is very slow, and should be implemented more efficiently.

    TODO document this interface *)

type t = {
  kind: SymbolDefinition.kind;
  name: string;
  full_name: string;
  path: Relative_path.t option;
}

val resolve :
  Provider_context.t ->
  Relative_path.t SymbolOccurrence.t ->
  sym_path:bool ->
  t option

val get_class_by_name :
  Provider_context.t -> string -> [ `None | `Enum | `Class of Nast.class_ ]

val get_kind : Provider_context.t -> string -> Ast_defs.classish_kind option
