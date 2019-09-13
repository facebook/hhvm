(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Hh_json
open Aast
open Ast_defs
open Decl_env
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
  | Symbol
  | Parameter
  | Filename
  | FunctionDeclaration
  | ClassDeclaration
  | TypedefDeclaration
  | GconstDeclaration
  | SymbolOccurrence

type glean_json = {
  symbol: json list;
  parameter: json list;
  filename: json list;
  functionDeclaration: json list;
  classDeclaration: json list;
  typedefDeclaration: json list;
  gconstDeclaration: json list;
  symbolOccurrence: json list;
}

let default_json =
  {
    symbol = [];
    parameter = [];
    filename = [];
    functionDeclaration = [];
    classDeclaration = [];
    typedefDeclaration = [];
    gconstDeclaration = [];
    symbolOccurrence = [];
  }

let hint tcopt h =
  let mode = FileInfo.Mdecl in
  let decl_env = { mode; droot = None; decl_tcopt = tcopt } in
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
  | Symbol ->
    { json_data_progress with symbol = json :: json_data_progress.symbol }
  | Parameter ->
    {
      json_data_progress with
      parameter = json :: json_data_progress.parameter;
    }
  | Filename ->
    { json_data_progress with filename = json :: json_data_progress.filename }
  | FunctionDeclaration ->
    {
      json_data_progress with
      functionDeclaration = json :: json_data_progress.functionDeclaration;
    }
  | ClassDeclaration ->
    {
      json_data_progress with
      classDeclaration = json :: json_data_progress.classDeclaration;
    }
  | TypedefDeclaration ->
    {
      json_data_progress with
      typedefDeclaration = json :: json_data_progress.typedefDeclaration;
    }
  | GconstDeclaration ->
    {
      json_data_progress with
      gconstDeclaration = json :: json_data_progress.gconstDeclaration;
    }
  | SymbolOccurrence ->
    {
      json_data_progress with
      symbolOccurrence = json :: json_data_progress.symbolOccurrence;
    }

let glean_json predicate json json_data_progress =
  let id = json_element_id () in
  let json_facts =
    JSON_Object [("id", JSON_Number (string_of_int id)); ("key", json)]
  in
  (json_facts, update_json_data predicate json_facts json_data_progress)

let json_of_gconst tcopt gc_id gc json_data_progress =
  let (pos, _) = gc.cst_name in
  let ty =
    match gc.cst_type with
    | None -> (Typing_reason.Rhint pos, Typing_defs.Tnothing)
    | Some t -> hint tcopt t
  in
  let json =
    JSON_Object
      [
        ("name", JSON_Object [("key", JSON_String gc_id)]);
        ("type", JSON_Object [("key", JSON_String (type_ tcopt ty))]);
      ]
  in
  glean_json GconstDeclaration json json_data_progress

let json_of_typedef _ td_id td json_data_progress =
  let visibility =
    match td.t_vis with
    | Transparent -> true
    | Opaque -> false
  in
  let json =
    JSON_Object
      [
        ("name", JSON_Object [("key", JSON_String td_id)]);
        ("is_visible", JSON_Bool visibility);
      ]
  in
  glean_json TypedefDeclaration json json_data_progress

let json_of_fun tcopt fn_id fn json_data_progress =
  let ret_type =
    match hint_of_type_hint fn.f_ret with
    | None -> (Typing_reason.Rnone, Typing_defs.Tnothing)
    | Some h -> hint tcopt h
  in
  let (params, progress) =
    List.fold
      fn.f_params
      ~init:([], json_data_progress)
      ~f:(fun (param_acc, progress_acc) f_param ->
        let ty =
          match hint_of_type_hint f_param.param_type_hint with
          | None ->
            (Typing_reason.Rhint f_param.param_pos, Typing_defs.Tnothing)
          | Some h -> hint tcopt h
        in
        let json =
          JSON_Object
            [
              ("name", JSON_Object [("key", JSON_String f_param.param_name)]);
              ("type", JSON_Object [("key", JSON_String (type_ tcopt ty))]);
            ]
        in
        let (json_facts, progress) = glean_json Parameter json progress_acc in
        (json_facts :: param_acc, progress))
  in
  let json_facts =
    JSON_Object
      [
        ("name", JSON_Object [("key", JSON_String fn_id)]);
        ("params", JSON_Array params);
        ( "return_type",
          JSON_Object [("key", JSON_String (type_ tcopt ret_type))] );
      ]
  in
  glean_json FunctionDeclaration json_facts progress

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

let json_of_pos pos json_data_progress =
  let (line_start, char_start, line_end, char_end) = Pos.destruct_range pos in
  let fn = Relative_path.S.to_string (Pos.filename pos) in
  let file_hash = SharedMem.get_hash fn in
  let file_json =
    JSON_Object
      [
        ("filename", JSON_String fn);
        ("filehash_id", JSON_String (Int64.to_string file_hash));
      ]
  in
  let (json_facts, progress) =
    glean_json Filename file_json json_data_progress
  in
  let json =
    Hh_json.JSON_Object
      [
        ("filename", json_facts);
        ("line_start", int_ line_start);
        ("char_start", int_ char_start);
        ("line_end", int_ line_end);
        ("char_end", int_ (char_end - 1));
      ]
  in
  (json, progress)

let json_of_symbol tcopt decl_type json_fun pos id elem json_data_progress =
  let (json_facts, progress) = json_fun tcopt id elem json_data_progress in
  let (pos_json, progress) = json_of_pos pos progress in
  let name_lowercase = String.lowercase id in
  let json_facts =
    JSON_Object
      [
        ("name_lowercase", JSON_String name_lowercase);
        ("position", pos_json);
        ("declaration", JSON_Object [(decl_type, json_facts)]);
      ]
  in
  glean_json Symbol json_facts progress

let json_of_symbol_occurrence_shared symbol_occurrence progress symbol_def =
  let key_ json = JSON_Object [("key", json)] in
  let name_lowercase = String.lowercase symbol_occurrence.name in
  let (symbol_kind, class_attribute) =
    match symbol_occurrence.type_ with
    | Class -> (0, JSON_Null)
    | Function -> (1, JSON_Null)
    | Method (s1, s2) ->
      ( 2,
        JSON_Object
          [
            ("class_name", key_ (JSON_String s1));
            ("property_name", key_ (JSON_String s2));
          ] )
    | LocalVar -> (3, JSON_Null)
    | Property (s1, s2) ->
      ( 4,
        JSON_Object
          [
            ("class_name", key_ (JSON_String s1));
            ("property_name", key_ (JSON_String s2));
          ] )
    | ClassConst (s1, s2) ->
      ( 5,
        JSON_Object
          [
            ("class_name", key_ (JSON_String s1));
            ("property_name", key_ (JSON_String s2));
          ] )
    | Typeconst (s1, s2) ->
      ( 6,
        JSON_Object
          [
            ("class_name", key_ (JSON_String s1));
            ("property_name", key_ (JSON_String s2));
          ] )
    | GConst -> (7, JSON_Null)
  in
  let type_object =
    match class_attribute with
    | JSON_Null -> JSON_Object [("symbol_kind", int_ symbol_kind)]
    | _ ->
      JSON_Object
        [
          ("symbol_kind", int_ symbol_kind);
          ("class_attribute", class_attribute);
        ]
  in
  let (pos_json, progress) = json_of_pos symbol_occurrence.pos progress in
  let (def_pos_json, progress) =
    match symbol_def with
    | None -> (None, progress)
    | Some symbol_def ->
      let (def_pos_json, progress) = json_of_pos symbol_def progress in
      (Some def_pos_json, progress)
  in
  let json_facts =
    match def_pos_json with
    | None ->
      JSON_Object
        [
          ("name_lowercase", JSON_String name_lowercase);
          ("type", type_object);
          ("position", pos_json);
        ]
    | Some def_pos_json ->
      JSON_Object
        [
          ("name_lowercase", JSON_String name_lowercase);
          ("type", type_object);
          ("position", pos_json);
          ("definition_pos", def_pos_json);
        ]
  in
  glean_json SymbolOccurrence json_facts progress

let json_of_symbol_occurrence symbol_occurrence progress =
  let symbol_def = ServerSymbolDefinition.go None symbol_occurrence in
  let symbol_def_pos =
    match symbol_def with
    | None -> None
    | Some symbol_def -> Some symbol_def.pos
  in
  json_of_symbol_occurrence_shared symbol_occurrence progress symbol_def_pos

let json_of_localvars progress lv =
  let symbol_def = Some lv.lv_definition in
  List.fold lv.lvs ~init:progress ~f:(fun acc sym_occ ->
      snd (json_of_symbol_occurrence_shared sym_occ acc symbol_def))

let build_json tcopt symbols =
  let json_data_progress =
    List.fold symbols.decls ~init:default_json ~f:(fun acc symbol ->
        match symbol with
        | Fun fd ->
          let (pos, id) = fd.f_name in
          snd (json_of_symbol tcopt "function_" json_of_fun pos id fd acc)
        | Class cd ->
          let (pos, id) = cd.c_name in
          snd (json_of_symbol tcopt "class_" json_of_class pos id cd acc)
        | Typedef td ->
          let (pos, id) = td.t_name in
          snd (json_of_symbol tcopt "typedef_" json_of_typedef pos id td acc)
        | Constant gd ->
          let (pos, id) = gd.cst_name in
          snd (json_of_symbol tcopt "gconst_" json_of_gconst pos id gd acc)
        | _ -> acc)
  in
  let json_data_progress =
    List.fold symbols.occurrences ~init:json_data_progress ~f:(fun acc occ ->
        if occ.is_declaration then
          acc
        else
          snd (json_of_symbol_occurrence occ acc))
  in
  let json_data_progress =
    List.fold symbols.localvars ~init:json_data_progress ~f:json_of_localvars
  in
  let preds_and_records =
    [
      ("hackfull.symbol.1", json_data_progress.symbol);
      ("hackfull.functionParameter.1", json_data_progress.parameter);
      ("hackfull.filename.1", json_data_progress.filename);
      ("hackfull.functionDeclaration.1", json_data_progress.functionDeclaration);
      ("hackfull.classDeclaration.1", json_data_progress.classDeclaration);
      ("hackfull.typedefDeclaration.1", json_data_progress.typedefDeclaration);
      ("hackfull.gconstDeclaration.1", json_data_progress.gconstDeclaration);
      ("hackfull.symbolOccurrence.1", json_data_progress.symbolOccurrence);
    ]
  in
  let json_array =
    List.fold preds_and_records ~init:[] ~f:(fun acc (pred, json_lst) ->
        JSON_Object
          [("predicate", JSON_String pred); ("facts", JSON_Array json_lst)]
        :: acc)
  in
  json_array
