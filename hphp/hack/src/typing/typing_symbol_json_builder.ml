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

let default_json =
  { classDeclaration = []; declarationLocation = []; fileXRefs = [] }

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

let update_json_data predicate json json_data_progress =
  match predicate with
  | ClassDeclaration ->
    {
      json_data_progress with
      classDeclaration = json :: json_data_progress.classDeclaration;
    }
  | DeclarationLocation ->
    {
      json_data_progress with
      declarationLocation = json :: json_data_progress.declarationLocation;
    }
  | FileXRefs ->
    { json_data_progress with fileXRefs = json :: json_data_progress.fileXRefs }

let glean_json predicate json json_data_progress =
  let id = json_element_id () in
  let json_facts =
    JSON_Object [("id", JSON_Number (string_of_int id)); ("key", json)]
  in
  (json_facts, id, update_json_data predicate json_facts json_data_progress)

let json_of_bytespan pos =
  let start = fst (Pos.info_raw pos) in
  let length = Pos.length pos in
  JSON_Object
    [
      ("start", JSON_Number (string_of_int start));
      ("length", JSON_Number (string_of_int length));
    ]

let json_of_class _ class_name clss json_data_progress =
  let is_abstract =
    match clss.c_kind with
    | Cabstract -> true
    | _ -> false
  in
  let json_facts =
    JSON_Object
      [
        ("name", JSON_Object [("key", JSON_String class_name)]);
        ("is_abstract", JSON_Bool is_abstract);
        ("is_final", JSON_Bool clss.c_final);
      ]
  in
  glean_json ClassDeclaration json_facts json_data_progress

let json_of_decl_loc tcopt decl_type json_fun pos id elem json_data_progress =
  let (_, fact_id, progress) = json_fun tcopt id elem json_data_progress in
  let filepath = Relative_path.S.to_string (Pos.filename pos) in
  let json_facts =
    JSON_Object
      [
        ( "declaration",
          JSON_Object [(decl_type, JSON_Number (string_of_int fact_id))] );
        ("file", JSON_Object [("key", JSON_String filepath)]);
        ("span", json_of_bytespan pos);
      ]
  in
  glean_json DeclarationLocation json_facts progress

let build_json tcopt symbols =
  let json_data_progress =
    List.fold symbols.decls ~init:default_json ~f:(fun acc symbol ->
        match symbol with
        | Class cd ->
          let (pos, id) = cd.c_name in
          let (_, _, res) =
            json_of_decl_loc tcopt "class_" json_of_class pos id cd acc
          in
          res
        | _ -> acc)
  in
  let preds_and_records =
    (* The order is the reverse of how these items appear in the JSON,
    which is significant because later entries can refer to earlier ones
    by id only *)
    [
      ("hack.FileXRefs.1", json_data_progress.fileXRefs);
      ("hack.DeclarationLocation.1", json_data_progress.declarationLocation);
      ("hack.ClassDeclaration.1", json_data_progress.classDeclaration);
    ]
  in
  let json_array =
    List.fold preds_and_records ~init:[] ~f:(fun acc (pred, json_lst) ->
        JSON_Object
          [("predicate", JSON_String pred); ("facts", JSON_Array json_lst)]
        :: acc)
  in
  json_array
