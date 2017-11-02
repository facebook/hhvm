(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open Option.Monad_infix
open ServerEnv
open Reordered_argument_collections

let to_json input =
  let entries = List.map input begin fun (name, pos) ->
    let filename = Pos.filename pos in
    let line, start, end_ = Pos.info_pos pos in
    Hh_json.JSON_Object [
      "name", Hh_json.JSON_String name;
      "filename", Hh_json.JSON_String filename;
      "line", Hh_json.int_ line;
      "char_start", Hh_json.int_ start;
      "char_end", Hh_json.int_ end_;
    ]
  end in
  Hh_json.JSON_Array entries

let add_ns name =
  if name.[0] = '\\' then name else "\\" ^ name

let strip_ns results =
  List.map results (fun (s, p) -> ((Utils.strip_ns s), p))

let search target include_defs files genv env =
  (* Get all the references to the provided target in the files *)
  let res = FindRefsService.find_references env.tcopt genv.workers
    target include_defs env.files_info files in
  strip_ns res

let search_function function_name include_defs genv env =
  let function_name = add_ns function_name in
  let files = FindRefsService.get_dependent_files_function
    genv.ServerEnv.workers function_name in
  search (FindRefsService.IFunction function_name) include_defs files genv env

let search_member class_name member include_defs genv env =
  let class_name = add_ns class_name in
  (* Find all the classes that extend this one *)
  let files = FindRefsService.get_child_classes_files class_name in
  let all_classes = FindRefsService.find_child_classes env.tcopt
      class_name env.files_info files in
  let all_classes = SSet.add all_classes class_name in
  (* Get all the files that reference those classes *)
  let files = FindRefsService.get_dependent_files
      genv.ServerEnv.workers all_classes in
  let target =
    FindRefsService.IMember (FindRefsService.Class_set all_classes, member)
  in
  search target include_defs files genv env

let search_gconst cst_name include_defs genv env =
  let cst_name = add_ns cst_name in
  let files = FindRefsService.get_dependent_files_gconst
    genv.ServerEnv.workers cst_name in
  search (FindRefsService.IGConst cst_name) include_defs files genv env

let search_class class_name include_defs genv env =
  let class_name = add_ns class_name in
  let files = FindRefsService.get_dependent_files
      genv.ServerEnv.workers (SSet.singleton class_name) in
  search (FindRefsService.IClass class_name) include_defs files genv env

let get_refs action include_defs genv env =
  match action with
  | FindRefsService.Member (class_name, member) ->
      search_member class_name member include_defs genv env
  | FindRefsService.Function function_name ->
      search_function function_name include_defs genv env
  | FindRefsService.Class class_name ->
      search_class class_name include_defs genv env
  | FindRefsService.GConst cst_name ->
      search_gconst cst_name include_defs genv env

let go action include_defs genv env =
  let res = get_refs action include_defs genv env in
  let res = List.map res (fun (r, pos) -> (r, Pos.to_absolute pos)) in
  res

let get_action symbol =
  let name = symbol.SymbolOccurrence.name in
  begin match symbol.SymbolOccurrence.type_ with
    | SymbolOccurrence.Class -> Some (FindRefsService.Class name)
    | SymbolOccurrence.Function -> Some (FindRefsService.Function name)
    | SymbolOccurrence.Method (class_name, method_name) ->
        Some (FindRefsService.Member
          (class_name, FindRefsService.Method method_name))
    | SymbolOccurrence.Property (class_name, prop_name) ->
        Some (FindRefsService.Member
          (class_name, FindRefsService.Property prop_name))
    | SymbolOccurrence.ClassConst (class_name, const_name) ->
        Some (FindRefsService.Member
          (class_name, FindRefsService.Class_const const_name))
    | SymbolOccurrence.Typeconst (class_name, tconst_name) ->
        Some (FindRefsService.Member
          (class_name, FindRefsService.Typeconst tconst_name))
    | SymbolOccurrence.GConst -> Some (FindRefsService.GConst name)
    | _ -> None
  end

let go_from_file (content, line, char, include_defs) genv env =
  (* Find the symbol at given position *)
  ServerIdentifyFunction.go content line char env.ServerEnv.tcopt |>
  (* If there are few, arbitrarily pick the first *)
  List.hd >>= fun (occurrence, definition) ->
  (* Ignore symbols that lack definitions *)
  definition >>= fun definition ->
  get_action occurrence >>= fun action ->
  let results = go action include_defs genv env |> List.map ~f:snd in
  Some (definition.SymbolDefinition.full_name, results)
