(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

let get_target symbol =
  let open SymbolOccurrence in
  let open FindRefsService in
  match symbol.type_ with
  | SymbolOccurrence.Class -> Some (IClass symbol.name)
  | SymbolOccurrence.Function -> Some (IFunction symbol.name)
  | SymbolOccurrence.Method (class_name, member_name) ->
      Some (IMember (Subclasses_of class_name,
        FindRefsService.Method member_name))
  | SymbolOccurrence.Property (class_name, member_name) ->
      Some (IMember (Subclasses_of class_name,
        FindRefsService.Property member_name))
  | SymbolOccurrence.ClassConst (class_name, member_name) ->
      Some (IMember (Subclasses_of class_name,
        FindRefsService.Class_const member_name))
  | SymbolOccurrence.Typeconst  (class_name, member_name) ->
      Some (IMember (Subclasses_of class_name,
        FindRefsService.Typeconst member_name))
  | SymbolOccurrence.GConst -> Some (IGConst symbol.name)
  | _ -> None

let highlight_symbol tcopt (line, char) path file_info symbol =
  let res = match get_target symbol with
    | Some target ->
      let results = FindRefsService.find_refs
         tcopt target [] [(path, file_info)] in
      List.rev (List.map results snd)
    | None when symbol.SymbolOccurrence.type_ = SymbolOccurrence.LocalVar ->
      begin match Parser_heap.ParserHeap.get path with
      | Some (ast, _) -> ServerFindLocals.go_from_ast ast line char
      | None -> []
      end
    | None -> []
  in
  List.map res Ide_api_types.pos_to_range

let filter_result symbols result =
  let result = List.fold symbols ~init:result ~f:(fun result symbol ->
    if (Pos.length symbol.SymbolOccurrence.pos >
      Pos.length result.SymbolOccurrence.pos)
    then result
    else symbol) in
  List.filter symbols ~f:(fun symbol ->
    symbol.SymbolOccurrence.pos = result.SymbolOccurrence.pos)

let compare r1 r2 =
  let open Ide_api_types in
  let s1, s2 = r1.st, r2.st in
  if s1.line < s2.line then -1
  else if s1.line > s2.line then 1
  else if s1.column < s2.column then -1
  else if s1.column > s2.column then 1
  else 0

let rec combine_result l l1 l2 =
  match l1, l2 with
  | l1, [] ->
    l @ l1
  | [], l2 ->
    l @ l2
  | h1 :: l1_, h2 :: l2_ ->
    begin
      match compare h1 h2 with
      | -1 -> combine_result (l @ [h1]) l1_ l2
      | 1 -> combine_result (l @ [h2]) l1 l2_
      | 0 -> combine_result (l @ [h1]) l1_ l2_
      | _ -> l
    end

let go (content, line, char) tcopt =
  ServerIdentifyFunction.get_occurrence_and_map tcopt content line char
    ~f:begin fun path file_info symbols ->
      match symbols with
      | symbol::_ ->
        let symbols = filter_result symbols symbol in
        List.fold symbols ~init:[] ~f:(fun acc symbol ->
          combine_result [] acc
            (highlight_symbol tcopt (line, char) path file_info symbol))
      | _ -> []
    end
