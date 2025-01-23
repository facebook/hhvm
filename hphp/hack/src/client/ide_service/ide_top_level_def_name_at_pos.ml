(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let first_top_level_def symbols =
  List.fold symbols ~init:None ~f:(fun res symbol ->
      match res with
      | Some _ -> res
      | None ->
        if SymbolOccurrence.is_top_level_definition symbol then
          Some (Utils.strip_ns symbol.SymbolOccurrence.name)
        else
          None)

let go_quarantined ctx entry ~line ~column : string option =
  let (symbols : _ SymbolOccurrence.t list) =
    IdentifySymbolService.go_quarantined
      ~ctx
      ~entry
      ~line
      ~column
      ~use_declaration_spans:true
  in
  match first_top_level_def symbols with
  | Some def -> Some def
  | None ->
    let symbols = IdentifySymbolService.all_symbols_ctx ~ctx ~entry in
    first_top_level_def symbols
