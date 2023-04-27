(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Lsp.CallHierarchyItem

let item_matches_symocc
    (item : Lsp.CallHierarchyItem.t)
    (symbol : Relative_path.t SymbolOccurrence.t) : bool =
  let open SymbolOccurrence in
  item.name = symbol.name
  && Lsp_helpers.sym_occ_kind_to_lsp_sym_info_kind symbol.type_ = item.kind
  &&
  let selection = Lsp_helpers.hack_pos_to_lsp_range_adjusted symbol.pos in
  let squiggle = item.selectionRange in
  Lsp_helpers.get_range_overlap selection squiggle
  = Lsp_helpers.Selection_covers_whole_squiggle

let call_item_to_symbol_occ_list
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(item : Lsp.CallHierarchyItem.t) : Relative_path.t SymbolOccurrence.t list
    =
  let all_symbols = IdentifySymbolService.all_symbols_ctx ~ctx ~entry in
  List.filter (item_matches_symocc item) all_symbols
