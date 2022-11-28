(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Indexable = Symbol_indexable

type def = {
  kind: SymbolDefinition.kind;
  name: string;
  full_name: string;
}

type symbol = {
  occ: Relative_path.t SymbolOccurrence.t;
  def: def option;
}

type t = {
  path: string;
  cst: Full_fidelity_positioned_syntax.t;
  tast: Tast.program;
  source_text: Full_fidelity_source_text.t;
  symbols: symbol list;
  sym_hash: Md5.t option;
  fanout: bool;
}

(* TODO we hash the string representation of the symbol types. We
   should move a more robust scheme and make sure this is enough to
   identify files which need reindexing *)
let compute_sym_hash path symbols =
  let concat hash str = Md5.digest_string (Md5.to_binary hash ^ str) in
  let f cur { occ; def } =
    let full_name =
      match def with
      | None -> ""
      | Some def -> SymbolDefinition.(def.full_name ^ string_of_kind def.kind)
    in
    concat cur SymbolOccurrence.(occ.name ^ show_kind occ.type_ ^ full_name)
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
  let symbol_occs = IdentifySymbolService.all_symbols ctx tast in
  let symbols =
    List.map symbol_occs ~f:(fun occ ->
        let def =
          match ServerSymbolDefinition.go ctx None occ with
          | None -> None
          | Some SymbolDefinition.{ name; full_name; kind; _ } ->
            Some { name; full_name; kind }
        in
        { occ; def })
  in
  let sym_hash =
    if gen_sym_hash then
      Some (compute_sym_hash path_str symbols)
    else
      None
  in
  { path = path_str; tast; source_text; cst; symbols; sym_hash; fanout }
