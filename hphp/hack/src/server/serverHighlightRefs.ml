(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let get_target symbol =
  SymbolOccurrence.(
    let module Types = ServerCommandTypes.Find_refs in
    FindRefsService.(
      match symbol.type_ with
      | SymbolOccurrence.Class _ -> Some (IClass symbol.name)
      | SymbolOccurrence.Function -> Some (IFunction symbol.name)
      | SymbolOccurrence.Method (class_name, member_name) ->
        Some (IMember (Subclasses_of class_name, Types.Method member_name))
      | SymbolOccurrence.Property (class_name, member_name)
      | SymbolOccurrence.XhpLiteralAttr (class_name, member_name) ->
        Some (IMember (Subclasses_of class_name, Types.Property member_name))
      | SymbolOccurrence.ClassConst (class_name, member_name) ->
        Some (IMember (Subclasses_of class_name, Types.Class_const member_name))
      | SymbolOccurrence.Typeconst (class_name, member_name) ->
        Some (IMember (Subclasses_of class_name, Types.Typeconst member_name))
      | SymbolOccurrence.GConst -> Some (IGConst symbol.name)
      | _ -> None))

let highlight_symbol ctx entry line char symbol =
  let res =
    match get_target symbol with
    | Some target ->
      let results = FindRefsService.find_refs_ctx ~ctx ~entry ~target in
      List.rev (List.map results ~f:snd)
    | None
      when SymbolOccurrence.equal_kind
             symbol.SymbolOccurrence.type_
             SymbolOccurrence.LocalVar ->
      ServerFindLocals.go ~ctx ~entry ~line ~char
    | None -> []
  in
  List.map res ~f:Ide_api_types.pos_to_range

let compare r1 r2 =
  Ide_api_types.(
    let (s1, s2) = (r1.st, r2.st) in
    if s1.line < s2.line then
      -1
    else if s1.line > s2.line then
      1
    else if s1.column < s2.column then
      -1
    else if s1.column > s2.column then
      1
    else
      0)

let go_quarantined
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : ServerHighlightRefsTypes.result =
  let symbol_to_highlight =
    IdentifySymbolService.go_quarantined ~ctx ~entry ~line ~column
  in
  let results =
    List.fold symbol_to_highlight ~init:[] ~f:(fun acc s ->
        let stuff = highlight_symbol ctx entry line column s in
        List.append stuff acc)
  in
  let results = List.dedup_and_sort ~compare results in
  results
