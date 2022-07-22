(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let go_quarantined
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : Lsp.PrepareCallHierarchy.result =
  let all_sym_occs =
    IdentifySymbolService.go_quarantined ~ctx ~entry ~line ~column
  in
  let matching_sym_occs =
    List.filter
      (fun s -> Pos.inside s.SymbolOccurrence.pos line column)
      all_sym_occs
  in
  let (_, get_def) = ServerDepsUtil.get_ast_getdef ctx entry in
  let get_def_opt (sym_occ : Relative_path.t SymbolOccurrence.t) :
      Relative_path.t SymbolDefinition.t option =
    if sym_occ.SymbolOccurrence.is_declaration then
      get_def sym_occ
    else
      None
  in
  let def_opts = List.map get_def_opt matching_sym_occs in
  let items =
    List.map2 Lsp_helpers.symbol_to_lsp_call_item matching_sym_occs def_opts
  in
  Some items
