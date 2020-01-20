(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hh_json
open Aast
open Ast_defs
open Decl_env

type localvar = {
  lv_name: string;
  lv_definition: Relative_path.t Pos.pos;
  lvs: Relative_path.t SymbolOccurrence.t list;
}

type symbol_occurrences = {
  decls: Tast.def list;
  occurrences: Relative_path.t SymbolOccurrence.t list;
  localvars: localvar list;
}

type predicate =
  | ClassDeclaration
  | DeclarationLocation
  | FileXRefs

type glean_json = {
  classDeclaration: json list;
  declarationLocation: json list;
  fileXRefs: json list;
}

type result_progress = {
  resultJson: glean_json;
  (* Maps fact JSON to fact id *)
  factIds: (json, int) Hashtbl.t;
}

let init_progress =
  let default_json =
    { classDeclaration = []; declarationLocation = []; fileXRefs = [] }
  in
  (* TODO: The default poly function does not fully examine data structures;
  this will probably require a custom function *)
  let table = Hashtbl.Poly.create () in
  { resultJson = default_json; factIds = table }

let hint ctx h =
  let mode = FileInfo.Mdecl in
  let decl_env = { mode; droot = None; ctx } in
  Decl_hint.hint decl_env h

let get_next_elem_id () =
  let x = ref 500_000 in
  (* Glean requires IDs to start with high numbers *)
  fun () ->
    let r = !x in
    x := !x + 1;
    r

let json_element_id = get_next_elem_id ()

let type_ = Typing_print.full_decl

let update_json_data predicate json progress =
  let json =
    match predicate with
    | ClassDeclaration ->
      {
        progress.resultJson with
        classDeclaration = json :: progress.resultJson.classDeclaration;
      }
    | DeclarationLocation ->
      {
        progress.resultJson with
        declarationLocation = json :: progress.resultJson.declarationLocation;
      }
    | FileXRefs ->
      {
        progress.resultJson with
        fileXRefs = json :: progress.resultJson.fileXRefs;
      }
  in
  { resultJson = json; factIds = progress.factIds }

let glean_json predicate json progress =
  let id =
    match Hashtbl.find progress.factIds json with
    | Some fid -> fid
    | None ->
      let newFactId = json_element_id () in
      Hashtbl.add_exn progress.factIds json newFactId;
      newFactId
  in
  let json_fact =
    JSON_Object [("id", JSON_Number (string_of_int id)); ("key", json)]
  in
  (json_fact, id, update_json_data predicate json_fact progress)

let json_of_bytespan pos =
  let start = fst (Pos.info_raw pos) in
  let length = Pos.length pos in
  JSON_Object
    [
      ("start", JSON_Number (string_of_int start));
      ("length", JSON_Number (string_of_int length));
    ]

let json_of_class _ class_name clss progress =
  let is_abstract =
    match clss.c_kind with
    | Cabstract -> true
    | _ -> false
  in
  let json_fact =
    JSON_Object
      [
        ("name", JSON_Object [("key", JSON_String class_name)]);
        ("is_abstract", JSON_Bool is_abstract);
        ("is_final", JSON_Bool clss.c_final);
      ]
  in
  glean_json ClassDeclaration json_fact progress

let json_of_decl tcopt decl_type json_fun id elem progress =
  let (_, fact_id, progress) = json_fun tcopt id elem progress in
  let json = JSON_Object [(decl_type, JSON_Number (string_of_int fact_id))] in
  (json, fact_id, progress)

let json_of_decl_loc tcopt decl_type pos json_fun id elem progress =
  let (decl_json, _, progress) =
    json_of_decl tcopt decl_type json_fun id elem progress
  in
  let filepath = Relative_path.S.to_string (Pos.filename pos) in
  let json_fact =
    JSON_Object
      [
        ("declaration", decl_json);
        ("file", JSON_Object [("key", JSON_String filepath)]);
        ("span", json_of_bytespan pos);
      ]
  in
  glean_json DeclarationLocation json_fact progress

let build_json tcopt symbols =
  let progress =
    List.fold symbols.decls ~init:init_progress ~f:(fun acc symbol ->
        match symbol with
        | Class cd ->
          let (pos, id) = cd.c_name in
          let (_, _, res) =
            json_of_decl_loc tcopt "class_" pos json_of_class id cd acc
          in
          res
        | _ -> acc)
  in
  let preds_and_records =
    (* The order is the reverse of how these items appear in the JSON,
    which is significant because later entries can refer to earlier ones
    by id only *)
    [
      ("hack.FileXRefs.1", progress.resultJson.fileXRefs);
      ("hack.DeclarationLocation.1", progress.resultJson.declarationLocation);
      ("hack.ClassDeclaration.1", progress.resultJson.classDeclaration);
    ]
  in
  let json_array =
    List.fold preds_and_records ~init:[] ~f:(fun acc (pred, json_lst) ->
        JSON_Object
          [("predicate", JSON_String pred); ("facts", JSON_Array json_lst)]
        :: acc)
  in
  json_array
