(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Hh_json
open Tast
open Ast_defs
open Decl_env

type decls = {
  funs: Tast.fun_def list;
  typedefs: Tast.typedef list;
  classes: Tast.class_ list;
  consts: Tast.gconst list;
}

let default_decls = { funs = []; classes = []; typedefs = []; consts = []}

type predicate =
  | Symbol
  | Parameter
  | Filename
  | FunctionDeclaration
  | ClassDeclaration
  | TypedefDeclaration
  | GconstDeclaration

type glean_json = {
  symbol: json list;
  parameter: json list;
  filename: json list;
  functionDeclaration: json list;
  classDeclaration: json list;
  typedefDeclaration: json list;
  gconstDeclaration: json list;
}

let default_json = {
  symbol = [];
  parameter = [];
  filename = [];
  functionDeclaration = [];
  classDeclaration = [];
  typedefDeclaration = [];
  gconstDeclaration = [];
}

let hint tcopt h =
  let mode = FileInfo.Mdecl in
  let decl_env = {
    mode = mode;
    droot = None;
    decl_tcopt = tcopt;
  } in
  Decl_hint.hint decl_env h

let get_next_elem_id () =
  let x = ref 500_000 in (* Glean requires IDs to start with high numbers *)
  (fun () -> let r = !x in x := !x + 1; r)

let json_element_id = get_next_elem_id ()

let type_ = Typing_print.suggest

let update_json_data predicate json json_data_progress =
  match predicate with
  | Symbol  ->
      { json_data_progress with symbol = json::json_data_progress.symbol }
  | Parameter ->
      { json_data_progress with parameter = json::json_data_progress.parameter }
  | Filename ->
      { json_data_progress with filename = json::json_data_progress.filename }
  | FunctionDeclaration ->
      { json_data_progress with functionDeclaration = json::json_data_progress.functionDeclaration }
  | ClassDeclaration ->
      { json_data_progress with classDeclaration = json::json_data_progress.classDeclaration }
  | TypedefDeclaration ->
      { json_data_progress with typedefDeclaration = json::json_data_progress.typedefDeclaration }
  | GconstDeclaration ->
      { json_data_progress with gconstDeclaration = json::json_data_progress.gconstDeclaration }

let glean_json predicate json json_data_progress =
  let id = json_element_id () in
  let json_facts = JSON_Object([
      "id", JSON_Number (string_of_int id);
      "key", json;
    ])
  in
  json_facts, update_json_data predicate json_facts json_data_progress

let json_of_gconst tcopt gc_id gc json_data_progress =
  let pos, _ = gc.cst_name in
  let ty = match gc.cst_type with
    | None -> Typing_reason.Rhint pos, Typing_defs.Tnothing
    | Some t -> hint tcopt t
  in
  let json = JSON_Object([
      "name", JSON_Object([ "key", JSON_String gc_id; ]);
      "type", JSON_Object([ "key", JSON_String (type_ ty); ]);
    ])
  in
  glean_json GconstDeclaration json json_data_progress

let json_of_typedef _ td_id td json_data_progress =
  let visibility = match td.t_vis with
    | Transparent -> true
    | Opaque -> false
  in
  let json = JSON_Object([
      "name", JSON_Object([ "key", JSON_String td_id; ]);
      "is_visible", JSON_Bool(visibility);
    ])
  in
  glean_json TypedefDeclaration json json_data_progress

let json_of_fun tcopt fn_id fn json_data_progress =
  let ret_type = match fn.f_ret with
    | None -> Typing_reason.Rnone, Typing_defs.Tnothing
    | Some h -> hint tcopt h
  in
  let params, progress =
    List.fold fn.f_params
      ~init:([], json_data_progress)
      ~f:begin fun (param_acc, progress_acc) f_param ->
        let ty = match f_param.param_hint with
          | None -> Typing_reason.Rhint f_param.param_pos, Typing_defs.Tnothing
          | Some h -> hint tcopt h
        in
        let json = JSON_Object([
            "name", JSON_Object( [ "key", JSON_String f_param.param_name; ]);
            "type", JSON_Object( [ "key", JSON_String (type_ ty); ]);
          ])
        in
        let json_facts, progress = glean_json Parameter json progress_acc in
        json_facts::param_acc, progress
      end
  in
  let json_facts = JSON_Object([
      "name", JSON_Object([ "key", JSON_String(fn_id); ]);
      "params", JSON_Array(params);
      "return_type", JSON_Object([ "key", JSON_String(type_ ret_type); ]);
    ])
  in glean_json FunctionDeclaration json_facts progress

let json_of_class _ class_name clss json_data_progress =
  let is_abstract = match clss.c_kind with
    | Cabstract -> true
    | _ -> false
  in
  let json_facts = JSON_Object([
    "name", JSON_Object([ "key", JSON_String(class_name); ]);
    "is_abstract", JSON_Bool(is_abstract);
    "is_final", JSON_Bool(clss.c_final);
  ]) in
  glean_json ClassDeclaration json_facts json_data_progress

let json_of_pos pos json_data_progress =
  let builtin_json = Pos.multiline_json (Pos.to_absolute pos) in
  (* the first value in the json must be the filename *)
  match builtin_json with
  | JSON_Object((_, JSON_String(filename))::t) ->
      let file_hash = SharedMem.get_hash filename in
      let file_json = JSON_Object([
          "filename", JSON_String(filename);
          "filehash_id", JSON_String(Int64.to_string file_hash);
        ])
      in
      let json_facts, progress = glean_json Filename file_json json_data_progress in
      let pos_js = JSON_Object(("filename", json_facts)::t) in
      pos_js, progress
  | _ -> failwith "Unexpected error: mismatched position json format"

let json_of_symbol tcopt decl_type json_fun pos id elem json_data_progress =
  let json_facts, progress = json_fun tcopt id elem json_data_progress in
  let pos_json, progress = json_of_pos pos progress in
  let json_facts = JSON_Object([
    "position", pos_json;
    "declaration", JSON_Object([
        decl_type, json_facts
    ]);
  ]) in
  glean_json Symbol json_facts progress

let build_json tcopt decls =
  let json_data_progress =
    List.fold decls.typedefs ~init:default_json ~f:begin fun acc td ->
      let (pos, id) = td.t_name in
      let (_, progress) = json_of_symbol tcopt "typdef_" json_of_typedef pos id td acc in
      progress
    end in
  let json_data_progress =
    List.fold decls.funs ~init:json_data_progress ~f:begin fun acc fd ->
      let (pos, id) = fd.f_name in
      let (_, progress) = json_of_symbol tcopt "function_" json_of_fun pos id fd acc in
      progress
    end in
  let json_data_progress =
    List.fold decls.classes ~init:json_data_progress ~f:begin fun acc cd ->
      let (pos, id) = cd.c_name in
      let (_, progress) = json_of_symbol tcopt "class_" json_of_class pos id cd acc in
      progress
    end in
  let json_data_progress =
    List.fold decls.consts ~init:json_data_progress ~f:begin fun acc gd ->
      let (pos, id) = gd.cst_name in
      let (_, progress) = json_of_symbol tcopt "gconst_" json_of_gconst pos id gd acc in
      progress
    end in
  (* breaking facts into chunks to reduce file size during upload *)
  let preds_and_records = [
    "hack.symbol.2", json_data_progress.symbol;
    "hack.functionParameter.1", json_data_progress.parameter;
    "hack.filename.1", json_data_progress.filename;
    "hack.functionDeclaration.1", json_data_progress.functionDeclaration;
    "hack.classDeclaration.1", json_data_progress.classDeclaration;
    "hack.typedefDeclaration.1", json_data_progress.typedefDeclaration;
    "hack.gconstDeclaration.1", json_data_progress.gconstDeclaration  ]
  in
  let json_array =
    List.fold preds_and_records ~init:[] ~f:begin fun acc (pred, json_lst) ->
      JSON_Object([
        "predicate", JSON_String(pred);
        "facts", JSON_Array(json_lst);
      ])::acc
    end
  in json_array
