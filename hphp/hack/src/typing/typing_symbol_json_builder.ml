(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Ast_defs
open Decl_env
open Hh_json
open Hh_prelude
open SymbolDefinition
open SymbolOccurrence

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
  | ClassDefinition
  | DeclarationLocation
  | FileXRefs

type glean_json = {
  classDeclaration: json list;
  classDefinition: json list;
  declarationLocation: json list;
  fileXRefs: json list;
}

type result_progress = {
  resultJson: glean_json;
  (* Maps fact JSON to fact id *)
  factIds: int JMap.t;
}

let init_progress =
  let default_json =
    {
      classDeclaration = [];
      classDefinition = [];
      declarationLocation = [];
      fileXRefs = [];
    }
  in
  { resultJson = default_json; factIds = JMap.empty }

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
    | ClassDefinition ->
      {
        progress.resultJson with
        classDefinition = json :: progress.resultJson.classDefinition;
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
  let (id, is_new, progress) =
    match JMap.find_opt json progress.factIds with
    | Some fid -> (fid, false, progress)
    | None ->
      let newFactId = json_element_id () in
      let progress =
        {
          resultJson = progress.resultJson;
          factIds = JMap.add json newFactId progress.factIds;
        }
      in
      (newFactId, true, progress)
  in
  let json_fact =
    JSON_Object [("id", JSON_Number (string_of_int id)); ("key", json)]
  in
  let progress =
    if is_new then
      update_json_data predicate json_fact progress
    else
      progress
  in
  (json_fact, id, progress)

let json_of_bytespan pos =
  let start = fst (Pos.info_raw pos) in
  let length = Pos.length pos in
  JSON_Object
    [
      ("start", JSON_Number (string_of_int start));
      ("length", JSON_Number (string_of_int length));
    ]

let json_of_rel_bytespan offset len =
  JSON_Object
    [
      ("offset", JSON_Number (string_of_int offset));
      ("length", JSON_Number (string_of_int len));
    ]

let json_of_file filepath = JSON_Object [("key", JSON_String filepath)]

let json_of_class_defn clss decl_id progress =
  let is_abstract =
    match clss.c_kind with
    | Cabstract -> true
    | _ -> false
  in
  let json_fact =
    JSON_Object
      [
        ("declaration", JSON_Number (string_of_int decl_id));
        ("is_abstract", JSON_Bool is_abstract);
        ("is_final", JSON_Bool clss.c_final);
      ]
  in
  glean_json ClassDefinition json_fact progress

let json_of_class_decl name progress =
  let json_fact =
    JSON_Object [("name", JSON_Object [("key", JSON_String name)])]
  in
  glean_json ClassDeclaration json_fact progress

let json_of_container_decl container_type decl_fun _ name _elem progress =
  let (_, fact_id, progress) = decl_fun name progress in
  let container_decl =
    JSON_Object [(container_type, JSON_Number (string_of_int fact_id))]
  in
  (container_decl, fact_id, progress)

let json_of_decl ctx decl_type json_fun id elem progress =
  let (decl_json, fact_id, progress) = json_fun ctx id elem progress in
  let json = JSON_Object [(decl_type, decl_json)] in
  (json, fact_id, progress)

let json_of_decl_loc ctx decl_type pos decl_fun defn_fun id elem progress =
  let (decl_json, decl_id, progress) =
    json_of_decl ctx decl_type decl_fun id elem progress
  in
  let (_, _, progress) = defn_fun elem decl_id progress in
  let filepath = Relative_path.S.to_string (Pos.filename pos) in
  let json_fact =
    JSON_Object
      [
        ("declaration", decl_json);
        ("file", json_of_file filepath);
        ("span", json_of_bytespan pos);
      ]
  in
  glean_json DeclarationLocation json_fact progress

let json_of_xrefs xref_map =
  let xrefs =
    IMap.fold
      (fun _id (target_json, pos_list) acc ->
        let sorted_pos = List.sort Pos.compare pos_list in
        let (byte_spans, _) =
          List.fold sorted_pos ~init:([], 0) ~f:(fun (spans, last_start) pos ->
              let start = fst (Pos.info_raw pos) in
              let length = Pos.length pos in
              let span = json_of_rel_bytespan (start - last_start) length in
              (span :: spans, start))
        in
        let xref =
          JSON_Object
            [("target", target_json); ("ranges", JSON_Array byte_spans)]
        in
        xref :: acc)
      xref_map
      []
  in
  JSON_Array xrefs

let json_of_file_xrefs filepath xref_map progress =
  let json_fact =
    JSON_Object
      [("file", json_of_file filepath); ("xrefs", json_of_xrefs xref_map)]
  in
  glean_json FileXRefs json_fact progress

let build_json ctx symbols =
  let progress =
    List.fold symbols.decls ~init:init_progress ~f:(fun acc symbol ->
        match symbol with
        | Class cd ->
          let (pos, id) = cd.c_name in
          let (_, _, res) =
            json_of_decl_loc
              ctx
              "container"
              pos
              (json_of_container_decl "class_" json_of_class_decl)
              json_of_class_defn
              id
              cd
              acc
          in
          res
        | _ -> acc)
  in
  (* file_xrefs : Hh_json.json * Relative_path.t Pos.pos list) IMap.t SMap.t *)
  let (progress, file_xrefs) =
    List.fold
      symbols.occurrences
      ~init:(progress, SMap.empty)
      ~f:(fun (prog, xrefs) occ ->
        if occ.is_declaration then
          (prog, xrefs)
        else
          let symbol_def_res = ServerSymbolDefinition.go ctx None occ in
          let filepath = Relative_path.S.to_string (Pos.filename occ.pos) in
          match symbol_def_res with
          | None -> (prog, xrefs)
          | Some symbol_def ->
            (match symbol_def.kind with
            | Class ->
              let (decl_json, target_id, prog) =
                json_of_decl
                  ctx
                  "container"
                  (json_of_container_decl "class_" json_of_class_decl)
                  symbol_def.name
                  symbol_def
                  prog
              in
              let xrefs =
                SMap.update
                  filepath
                  (fun file_map ->
                    let target_json =
                      JSON_Object [("declaration", decl_json)]
                    in
                    let new_ref = (target_json, [occ.pos]) in
                    match file_map with
                    | None -> Some (IMap.singleton target_id new_ref)
                    | Some map ->
                      let updated_xref_map =
                        IMap.update
                          target_id
                          (fun target_tuple ->
                            match target_tuple with
                            | None -> Some new_ref
                            | Some (json, refs) -> Some (json, occ.pos :: refs))
                          map
                      in
                      Some updated_xref_map)
                  xrefs
              in
              (prog, xrefs)
            | _ -> (prog, xrefs)))
  in
  let progress =
    SMap.fold
      (fun fp target_map acc ->
        let (_, _, res) = json_of_file_xrefs fp target_map acc in
        res)
      file_xrefs
      progress
  in
  let preds_and_records =
    (* The order is the reverse of how these items appear in the JSON,
    which is significant because later entries can refer to earlier ones
    by id only *)
    [
      ("hack.FileXRefs.1", progress.resultJson.fileXRefs);
      ("hack.ClassDefinition.1", progress.resultJson.classDefinition);
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
