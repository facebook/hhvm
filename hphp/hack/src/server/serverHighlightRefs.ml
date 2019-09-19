(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

let get_target symbol =
  SymbolOccurrence.(
    let module Types = ServerCommandTypes.Find_refs in
    FindRefsService.(
      match symbol.type_ with
      | SymbolOccurrence.Class -> Some (IClass symbol.name)
      | SymbolOccurrence.Function -> Some (IFunction symbol.name)
      | SymbolOccurrence.Method (class_name, member_name) ->
        Some (IMember (Subclasses_of class_name, Types.Method member_name))
      | SymbolOccurrence.Property (class_name, member_name) ->
        Some (IMember (Subclasses_of class_name, Types.Property member_name))
      | SymbolOccurrence.ClassConst (class_name, member_name) ->
        Some
          (IMember (Subclasses_of class_name, Types.Class_const member_name))
      | SymbolOccurrence.Typeconst (class_name, member_name) ->
        Some (IMember (Subclasses_of class_name, Types.Typeconst member_name))
      | SymbolOccurrence.GConst -> Some (IGConst symbol.name)
      | _ -> None))

let highlight_symbol tcopt ast (line, char) path file_info symbol =
  let res =
    match get_target symbol with
    | Some target ->
      let results =
        FindRefsService.find_refs tcopt target [] [(path, file_info)]
      in
      List.rev (List.map results snd)
    | None when symbol.SymbolOccurrence.type_ = SymbolOccurrence.LocalVar ->
      ServerFindLocals.go_from_ast ast line char
    | None -> []
  in
  List.map res Ide_api_types.pos_to_range

let filter_result symbols result =
  let result =
    List.fold symbols ~init:result ~f:(fun result symbol ->
        if
          Pos.length symbol.SymbolOccurrence.pos
          > Pos.length result.SymbolOccurrence.pos
        then
          result
        else
          symbol)
  in
  List.filter symbols ~f:(fun symbol ->
      symbol.SymbolOccurrence.pos = result.SymbolOccurrence.pos)

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

let rec combine_result l l1 l2 =
  match (l1, l2) with
  | (l1, []) -> l @ l1
  | ([], l2) -> l @ l2
  | (h1 :: l1_, h2 :: l2_) ->
    begin
      match compare h1 h2 with
      | -1 -> combine_result (l @ [h1]) l1_ l2
      | 1 -> combine_result (l @ [h2]) l1 l2_
      | 0 -> combine_result (l @ [h1]) l1_ l2_
      | _ -> l
    end

let go (content, line, char) tcopt =
  ServerIdentifyFunction.get_occurrence_and_map
    tcopt
    content
    line
    char
    ~f:(fun path file_info symbols ->
      let ast = Ast_provider.get_ast path in
      match symbols with
      | symbol :: _ ->
        let symbols = filter_result symbols symbol in
        List.fold symbols ~init:[] ~f:(fun acc symbol ->
            combine_result
              []
              acc
              (highlight_symbol tcopt ast (line, char) path file_info symbol))
      | _ -> [])

let go_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int)
    ~(tcopt : TypecheckerOptions.t) : ServerHighlightRefsTypes.result =
  let { Provider_context.path; ast; _ } = entry in
  let symbol_to_highlight =
    IdentifySymbolService.go
      (Provider_utils.compute_tast ~ctx ~entry)
      line
      column
  in
  let file_info = Provider_context.get_fileinfo entry in
  let results =
    List.fold symbol_to_highlight ~init:[] ~f:(fun acc s ->
        let stuff =
          highlight_symbol tcopt ast (line, column) path file_info s
        in
        List.append stuff acc)
  in
  results
