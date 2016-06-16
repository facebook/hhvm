(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

type result = Pos.absolute list

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
  | _ -> None

let highlight_symbol tcopt (line, char) path file_info symbol =
  let res = match get_target symbol with
    | Some target ->
      let results = FindRefsService.find_refs
        (Some tcopt) target [] [(path, file_info)] in
      List.rev (List.map results snd)
    | None when symbol.SymbolOccurrence.type_ = SymbolOccurrence.LocalVar ->
      begin match Parser_heap.ParserHeap.get path with
      | Some ast -> ServerFindLocals.go_from_ast ast line char
      | None -> []
      end
    | None -> []
  in
  List.map res Pos.to_absolute

let go (content, line, char) tcopt =
  ServerIdentifyFunction.get_occurrence_and_map content line char
    ~f:begin fun path file_info symbols ->
      match symbols with
      (* TODO: correctly handle multiple symbols in one place instead of
       * picking one of them *)
      | symbol::_ ->
        highlight_symbol tcopt (line, char) path file_info symbol
      | _ -> []
    end
