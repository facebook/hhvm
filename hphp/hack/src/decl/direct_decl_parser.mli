(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type decls = (string * Shallow_decl_defs.decl) list [@@deriving show]

val parse_decls_ffi : DeclParserOptions.t -> Relative_path.t -> string -> decls

val decls_hash : decls -> Int64.t

val parse_decls_and_mode_ffi :
  DeclParserOptions.t ->
  Relative_path.t ->
  string ->
  bool ->
  decls * FileInfo.mode option * Int64.t option
