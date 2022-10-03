(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Indexable = Symbol_indexable

type t = {
  path: string;
  cst: Full_fidelity_positioned_syntax.t;
  tast: Tast.program;
  source_text: Full_fidelity_source_text.t;
  symbols: Relative_path.t SymbolOccurrence.t list;
  sym_hash: Md5.t option;
  fanout: bool;
}

(* TODO we hash the string representation of the symbol types. We
   should move a more robust scheme and make sure this is enough to
   identify files which need reindexing *)
let compute_sym_hash path symbols =
  let concat hash str = Md5.digest_string (Md5.to_binary hash ^ str) in
  let f cur occ =
    concat cur SymbolOccurrence.(occ.name ^ show_kind occ.type_)
  in
  let hash = List.fold ~init:(Md5.digest_string "") ~f symbols in
  concat hash path

let create ctx Indexable.{ path; fanout } ~gen_sym_hash ~root_path ~hhi_path =
  let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
  let path_str =
    Relative_path.to_absolute_with_prefix
      ~www:(Path.make root_path)
      ~hhi:(Path.make hhi_path)
      path
  in
  let source_text = Ast_provider.compute_source_text ~entry in
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_unquarantined ~ctx ~entry
  in
  let cst =
    Provider_context.PositionedSyntaxTree.root
      (Ast_provider.compute_cst ~ctx ~entry)
  in
  let symbols = IdentifySymbolService.all_symbols ctx tast in
  let sym_hash =
    if gen_sym_hash then
      Some (compute_sym_hash path_str symbols)
    else
      None
  in
  { path = path_str; tast; source_text; cst; symbols; sym_hash; fanout }
