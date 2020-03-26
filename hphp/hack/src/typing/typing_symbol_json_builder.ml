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

(* Predicate types for the JSON facts emitted *)
type predicate =
  | ClassDeclaration
  | ClassDefinition
  | DeclarationLocation
  | EnumDeclaration
  | EnumDefinition
  | FileXRefs
  | FunctionDeclaration
  | FunctionDefinition
  | GlobalConstDeclaration
  | GlobalConstDefinition
  | InterfaceDeclaration
  | InterfaceDefinition
  | PropertyDeclaration
  | PropertyDefinition
  | TraitDeclaration
  | TraitDefinition
  | TypedefDeclaration

(* Containers that can be in inheritance relationships *)
type container_type =
  | ClassContainer
  | InterfaceContainer
  | TraitContainer

type glean_json = {
  classDeclaration: json list;
  classDefinition: json list;
  declarationLocation: json list;
  enumDeclaration: json list;
  enumDefinition: json list;
  fileXRefs: json list;
  functionDeclaration: json list;
  functionDefinition: json list;
  globalConstDeclaration: json list;
  globalConstDefinition: json list;
  interfaceDeclaration: json list;
  interfaceDefinition: json list;
  propertyDeclaration: json list;
  propertyDefinition: json list;
  traitDeclaration: json list;
  traitDefinition: json list;
  typedefDeclaration: json list;
}

type result_progress = {
  resultJson: glean_json;
  (* Maps fact JSON key to a list of predicate/fact id pairs *)
  factIds: (predicate * int) list JMap.t;
}

let init_progress =
  let default_json =
    {
      classDeclaration = [];
      classDefinition = [];
      declarationLocation = [];
      enumDeclaration = [];
      enumDefinition = [];
      fileXRefs = [];
      functionDeclaration = [];
      functionDefinition = [];
      globalConstDeclaration = [];
      globalConstDefinition = [];
      interfaceDeclaration = [];
      interfaceDefinition = [];
      propertyDeclaration = [];
      propertyDefinition = [];
      traitDeclaration = [];
      traitDefinition = [];
      typedefDeclaration = [];
    }
  in
  { resultJson = default_json; factIds = JMap.empty }

let rec find_fid fid_list pred =
  match fid_list with
  | [] -> None
  | (p, fid) :: tail ->
    if phys_equal p pred then
      Some fid
    else
      find_fid tail pred

let get_type_from_hint ctx h =
  let mode = FileInfo.Mdecl in
  let decl_env = { mode; droot = None; ctx } in
  Typing_print.full_decl ctx (Decl_hint.hint decl_env h)

let get_next_elem_id () =
  let x = ref 500_000 in
  (* Glean requires IDs to start with high numbers *)
  fun () ->
    let r = !x in
    x := !x + 1;
    r

let json_element_id = get_next_elem_id ()

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
    | EnumDeclaration ->
      {
        progress.resultJson with
        enumDeclaration = json :: progress.resultJson.enumDeclaration;
      }
    | EnumDefinition ->
      {
        progress.resultJson with
        enumDefinition = json :: progress.resultJson.enumDefinition;
      }
    | FileXRefs ->
      {
        progress.resultJson with
        fileXRefs = json :: progress.resultJson.fileXRefs;
      }
    | FunctionDeclaration ->
      {
        progress.resultJson with
        functionDeclaration = json :: progress.resultJson.functionDeclaration;
      }
    | FunctionDefinition ->
      {
        progress.resultJson with
        functionDefinition = json :: progress.resultJson.functionDefinition;
      }
    | GlobalConstDeclaration ->
      {
        progress.resultJson with
        globalConstDeclaration =
          json :: progress.resultJson.globalConstDeclaration;
      }
    | GlobalConstDefinition ->
      {
        progress.resultJson with
        globalConstDefinition =
          json :: progress.resultJson.globalConstDefinition;
      }
    | InterfaceDeclaration ->
      {
        progress.resultJson with
        interfaceDeclaration = json :: progress.resultJson.interfaceDeclaration;
      }
    | InterfaceDefinition ->
      {
        progress.resultJson with
        interfaceDefinition = json :: progress.resultJson.interfaceDefinition;
      }
    | PropertyDeclaration ->
      {
        progress.resultJson with
        propertyDeclaration = json :: progress.resultJson.propertyDeclaration;
      }
    | PropertyDefinition ->
      {
        progress.resultJson with
        propertyDefinition = json :: progress.resultJson.propertyDefinition;
      }
    | TraitDeclaration ->
      {
        progress.resultJson with
        traitDeclaration = json :: progress.resultJson.traitDeclaration;
      }
    | TraitDefinition ->
      {
        progress.resultJson with
        traitDefinition = json :: progress.resultJson.traitDefinition;
      }
    | TypedefDeclaration ->
      {
        progress.resultJson with
        typedefDeclaration = json :: progress.resultJson.typedefDeclaration;
      }
  in
  { resultJson = json; factIds = progress.factIds }

(* Add a fact of the given predicate type to the running result, if an identical
 fact has not yet been added. Return the fact's id (which can be referenced in
 other facts), and the updated result. *)
let add_fact predicate json_key progress =
  let add_id =
    let newFactId = json_element_id () in
    let progress =
      {
        resultJson = progress.resultJson;
        factIds =
          JMap.add
            json_key
            [(predicate, newFactId)]
            progress.factIds
            ~combine:List.append;
      }
    in
    (newFactId, true, progress)
  in
  let (id, is_new, progress) =
    match JMap.find_opt json_key progress.factIds with
    | None -> add_id
    | Some fid_list ->
      (match find_fid fid_list predicate with
      | None -> add_id
      | Some fid -> (fid, false, progress))
  in
  let json_fact =
    JSON_Object [("id", JSON_Number (string_of_int id)); ("key", json_key)]
  in
  let progress =
    if is_new then
      update_json_data predicate json_fact progress
    else
      progress
  in
  (id, progress)

(* Get the container name and predicate type for a given container kind. *)
let container_decl_predicate container_type =
  match container_type with
  | ClassContainer -> ("class_", ClassDeclaration)
  | InterfaceContainer -> ("interface_", InterfaceDeclaration)
  | TraitContainer -> ("trait", TraitDeclaration)

let get_container_kind clss =
  match clss.c_kind with
  | Cenum -> raise (Failure "Unexpected enum as container kind")
  | Cinterface -> InterfaceContainer
  | Ctrait -> TraitContainer
  | _ -> ClassContainer

(* JSON builder functions. These all return JSON objects, which
may be used to build up larger objects. The functions with prefix
_nested include the key field because they are used for writing
nested facts. *)
let build_name_json_nested name =
  (* Remove leading slash, if present, so names such as
  Exception and \Exception are captured by the same fact *)
  let basename = Utils.strip_ns name in
  JSON_Object [("key", JSON_String basename)]

let build_type_json_nested type_name =
  (* Remove namespace slash from type, if present *)
  let ty = Utils.strip_ns type_name in
  JSON_Object [("key", JSON_String ty)]

let build_enumerator_json_nested const_name decl_ref =
  JSON_Object
    [
      ( "key",
        JSON_Object
          [
            ("name", build_name_json_nested const_name);
            ("enumeration", decl_ref);
          ] );
    ]

let build_signature_json_nested parameters return_type_name =
  let fields =
    let params = [("parameters", JSON_Array parameters)] in
    match return_type_name with
    | None -> params
    | Some ty -> ("returns", build_type_json_nested ty) :: params
  in
  JSON_Object [("key", JSON_Object fields)]

let build_id_json fact_id =
  JSON_Object [("id", JSON_Number (string_of_int fact_id))]

let build_parameter_json param_name param_type_name =
  let fields =
    let name_field = [("name", build_name_json_nested param_name)] in
    match param_type_name with
    | None -> name_field
    | Some ty -> ("type", build_type_json_nested ty) :: name_field
  in
  JSON_Object fields

let build_bytespan_json pos =
  let start = fst (Pos.info_raw pos) in
  let length = Pos.length pos in
  JSON_Object
    [
      ("start", JSON_Number (string_of_int start));
      ("length", JSON_Number (string_of_int length));
    ]

let build_rel_bytespan_json offset len =
  JSON_Object
    [
      ("offset", JSON_Number (string_of_int offset));
      ("length", JSON_Number (string_of_int len));
    ]

let build_file_json filepath = JSON_Object [("key", JSON_String filepath)]

let build_decl_target_json json = JSON_Object [("declaration", json)]

let build_visibility_json (visibility : Aast.visibility) =
  let num =
    match visibility with
    | Private -> 0
    | Protected -> 1
    | Public -> 2
  in
  JSON_Number (string_of_int num)

let build_xrefs_json xref_map =
  let xrefs =
    IMap.fold
      (fun _id (target_json, pos_list) acc ->
        let sorted_pos = List.sort Pos.compare pos_list in
        let (byte_spans, _) =
          List.fold sorted_pos ~init:([], 0) ~f:(fun (spans, last_start) pos ->
              let start = fst (Pos.info_raw pos) in
              let length = Pos.length pos in
              let span = build_rel_bytespan_json (start - last_start) length in
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

(* These are functions for building JSON to reference some
existing fact. *)

let build_container_json_ref container_type fact_id =
  JSON_Object [(container_type, build_id_json fact_id)]

let build_container_decl_json_ref container_type fact_id =
  let container_json = build_container_json_ref container_type fact_id in
  JSON_Object [("container", container_json)]

let build_enum_decl_json_ref fact_id =
  JSON_Object [("enum_", build_id_json fact_id)]

let build_func_decl_json_ref fact_id =
  JSON_Object [("function_", build_id_json fact_id)]

let build_typedef_decl_json_ref fact_id =
  JSON_Object [("typedef_", build_id_json fact_id)]

let build_gconst_decl_json_ref fact_id =
  JSON_Object [("global_const", build_id_json fact_id)]

let build_property_decl_json_ref fact_id =
  JSON_Object [("property_", build_id_json fact_id)]

(* These functions build up the JSON necessary and then add facts
to the running result. *)

let add_container_defn_fact clss decl_id progress =
  let base_defn defn_pred =
    let json_fact = JSON_Object [("declaration", build_id_json decl_id)] in
    add_fact defn_pred json_fact progress
  in
  match get_container_kind clss with
  | InterfaceContainer -> base_defn InterfaceDefinition
  | TraitContainer -> base_defn TraitDefinition
  | ClassContainer ->
    let is_abstract =
      match clss.c_kind with
      | Cabstract -> true
      | _ -> false
    in
    let json_fact =
      JSON_Object
        [
          ("declaration", build_id_json decl_id);
          ("is_abstract", JSON_Bool is_abstract);
          ("is_final", JSON_Bool clss.c_final);
        ]
    in
    add_fact ClassDefinition json_fact progress

let add_container_decl_fact decl_pred name _elem progress =
  let json_fact = JSON_Object [("name", build_name_json_nested name)] in
  add_fact decl_pred json_fact progress

let add_property_decl_fact con_type decl_id name _prop progress =
  let json_fact =
    JSON_Object
      [
        ("name", build_name_json_nested name);
        ("container", build_container_json_ref con_type decl_id);
      ]
  in
  add_fact PropertyDeclaration json_fact progress

let add_property_defn_fact ctx prop decl_id progress =
  let base_fields =
    [
      ("declaration", build_id_json decl_id);
      ("visibility", build_visibility_json prop.cv_visibility);
      ("isFinal", JSON_Bool prop.cv_final);
      ("isAbstract", JSON_Bool prop.cv_abstract);
      ("isStatic", JSON_Bool prop.cv_is_static);
    ]
  in
  let json_fields =
    match hint_of_type_hint prop.cv_type with
    | None -> base_fields
    | Some h ->
      let ty = get_type_from_hint ctx h in
      ("type", build_type_json_nested ty) :: base_fields
  in
  add_fact PropertyDefinition (JSON_Object json_fields) progress

let add_enum_decl_fact name _elem progress =
  let json_fact = JSON_Object [("name", build_name_json_nested name)] in
  add_fact EnumDeclaration json_fact progress

let add_enum_defn_fact ctx elem decl_id progress =
  let decl_ref = build_id_json decl_id in
  let enumerators =
    List.map elem.c_consts (fun const ->
        build_enumerator_json_nested (snd const.cc_id) decl_ref)
  in
  let json_fields =
    let json_fields =
      [("declaration", decl_ref); ("enumerators", JSON_Array enumerators)]
    in
    match elem.c_enum with
    | None -> json_fields
    | Some en ->
      let json_fields =
        ("enumBase", build_type_json_nested (get_type_from_hint ctx en.e_base))
        :: json_fields
      in
      (match en.e_constraint with
      | None -> json_fields
      | Some c ->
        ("enumConstraint", build_type_json_nested (get_type_from_hint ctx c))
        :: json_fields)
  in
  add_fact EnumDefinition (JSON_Object json_fields) progress

let add_func_decl_fact name _elem progress =
  let json_fact = JSON_Object [("name", build_name_json_nested name)] in
  add_fact FunctionDeclaration json_fact progress

let add_func_defn_fact ctx elem decl_id progress =
  let parameters =
    List.map elem.f_params (fun param ->
        let ty =
          match hint_of_type_hint param.param_type_hint with
          | None -> None
          | Some h -> Some (get_type_from_hint ctx h)
        in
        build_parameter_json param.param_name ty)
  in
  let return_type_name =
    match hint_of_type_hint elem.f_ret with
    | None -> None
    | Some h -> Some (get_type_from_hint ctx h)
  in
  let signature = build_signature_json_nested parameters return_type_name in
  let is_async =
    match elem.f_fun_kind with
    | FAsync -> true
    | FAsyncGenerator -> true
    | _ -> false
  in
  let json_fact =
    JSON_Object
      [
        ("declaration", build_id_json decl_id);
        ("signature", signature);
        ("isAsync", JSON_Bool is_async);
      ]
  in
  add_fact FunctionDefinition json_fact progress

let add_typedef_decl_fact name elem progress =
  let is_transparent =
    match elem.t_vis with
    | Transparent -> true
    | Opaque -> false
  in
  let json_fact =
    JSON_Object
      [
        ("name", build_name_json_nested name);
        ("is_transparent", JSON_Bool is_transparent);
      ]
  in
  add_fact TypedefDeclaration json_fact progress

let add_gconst_decl_fact name _elem progress =
  let json_fact = JSON_Object [("name", build_name_json_nested name)] in
  add_fact GlobalConstDeclaration json_fact progress

let add_gconst_defn_fact ctx elem decl_id progress =
  let req_fields = [("declaration", build_id_json decl_id)] in
  let json_fields =
    match elem.cst_type with
    | None -> req_fields
    | Some h ->
      let ty = get_type_from_hint ctx h in
      ("type", build_type_json_nested ty) :: req_fields
  in
  let json_fact = JSON_Object json_fields in
  add_fact GlobalConstDefinition json_fact progress

let add_decl_loc_fact pos decl_json progress =
  let filepath = Relative_path.to_absolute (Pos.filename pos) in
  let json_fact =
    JSON_Object
      [
        ("declaration", decl_json);
        ("file", build_file_json filepath);
        ("span", build_bytespan_json pos);
      ]
  in
  add_fact DeclarationLocation json_fact progress

let add_file_xrefs_fact filepath xref_map progress =
  let json_fact =
    JSON_Object
      [("file", build_file_json filepath); ("xrefs", build_xrefs_json xref_map)]
  in
  add_fact FileXRefs json_fact progress

(* For building the map of cross-references *)
let add_xref target_json target_id ref_pos xrefs =
  let filepath = Relative_path.to_absolute (Pos.filename ref_pos) in
  SMap.update
    filepath
    (fun file_map ->
      let new_ref = (target_json, [ref_pos]) in
      match file_map with
      | None -> Some (IMap.singleton target_id new_ref)
      | Some map ->
        let updated_xref_map =
          IMap.update
            target_id
            (fun target_tuple ->
              match target_tuple with
              | None -> Some new_ref
              | Some (json, refs) -> Some (json, ref_pos :: refs))
            map
        in
        Some updated_xref_map)
    xrefs

(* These functions define the process to go through when
encountering symbols of a given type. *)

let process_xref
    decl_fun
    decl_ref_fun
    (symbol_def : Relative_path.t SymbolDefinition.t)
    symbol_pos
    (xrefs, progress) =
  let (target_id, prog) = decl_fun symbol_def.name symbol_def progress in
  let xref_json = decl_ref_fun target_id in
  let target_json = build_decl_target_json xref_json in
  let xrefs = add_xref target_json target_id symbol_pos xrefs in
  (xrefs, prog)

let process_container_xref
    (con_type, decl_pred) symbol_def symbol_pos (xrefs, progress) =
  process_xref
    (add_container_decl_fact decl_pred)
    (build_container_decl_json_ref con_type)
    symbol_def
    symbol_pos
    (xrefs, progress)

let process_decl_loc decl_fun defn_fun decl_ref_fun pos id elem progress =
  let (decl_id, prog) = decl_fun id elem progress in
  let (_, prog) = defn_fun elem decl_id prog in
  let ref_json = decl_ref_fun decl_id in
  let (_, prog) = add_decl_loc_fact pos ref_json prog in
  (decl_id, prog)

let process_container_decl ctx elem progress =
  let (pos, id) = elem.c_name in
  let (con_type, decl_pred) =
    container_decl_predicate (get_container_kind elem)
  in
  let (decl_id, prog) =
    process_decl_loc
      (add_container_decl_fact decl_pred)
      add_container_defn_fact
      (build_container_decl_json_ref con_type)
      pos
      id
      elem
      progress
  in
  List.fold elem.c_vars ~init:prog ~f:(fun prog prop ->
      let (pos, id) = prop.cv_id in
      let (_, prog) =
        process_decl_loc
          (add_property_decl_fact con_type decl_id)
          (add_property_defn_fact ctx)
          build_property_decl_json_ref
          pos
          id
          prop
          prog
      in
      prog)

let process_enum_decl ctx elem progress =
  let (pos, id) = elem.c_name in
  let (_, prog) =
    process_decl_loc
      add_enum_decl_fact
      (add_enum_defn_fact ctx)
      build_enum_decl_json_ref
      pos
      id
      elem
      progress
  in
  prog

let process_gconst_decl ctx elem progress =
  let (pos, id) = elem.cst_name in
  let (_, prog) =
    process_decl_loc
      add_gconst_decl_fact
      (add_gconst_defn_fact ctx)
      build_gconst_decl_json_ref
      pos
      id
      elem
      progress
  in
  prog

let process_func_decl ctx elem progress =
  let (pos, id) = elem.f_name in
  let (_, prog) =
    process_decl_loc
      add_func_decl_fact
      (add_func_defn_fact ctx)
      build_func_decl_json_ref
      pos
      id
      elem
      progress
  in
  prog

let process_typedef_decl elem progress =
  let (pos, id) = elem.t_name in
  let (decl_id, prog) = add_typedef_decl_fact id elem progress in
  let ref_json = build_typedef_decl_json_ref decl_id in
  let (_, prog) = add_decl_loc_fact pos ref_json prog in
  prog

(* This function walks over the symbols in each file and gleans
 facts along the way. *)
let build_json ctx symbols =
  let progress =
    List.fold symbols.decls ~init:init_progress ~f:(fun acc symbol ->
        match symbol with
        | Class en when phys_equal en.c_kind Cenum ->
          process_enum_decl ctx en acc
        | Class cd -> process_container_decl ctx cd acc
        | Constant gd -> process_gconst_decl ctx gd acc
        | Fun fd -> process_func_decl ctx fd acc
        | Typedef td -> process_typedef_decl td acc
        | _ -> acc)
  in
  (* file_xrefs : (Hh_json.json * Relative_path.t Pos.pos list) IMap.t SMap.t *)
  let (file_xrefs, progress) =
    List.fold
      symbols.occurrences
      ~init:(SMap.empty, progress)
      ~f:(fun (xrefs, prog) occ ->
        if occ.is_declaration then
          (xrefs, prog)
        else
          let symbol_def_res = ServerSymbolDefinition.go ctx None occ in
          match symbol_def_res with
          | None -> (xrefs, prog)
          | Some symbol_def ->
            (match symbol_def.kind with
            | Class ->
              let con_kind = container_decl_predicate ClassContainer in
              process_container_xref con_kind symbol_def occ.pos (xrefs, prog)
            | Interface ->
              let con_kind = container_decl_predicate InterfaceContainer in
              process_container_xref con_kind symbol_def occ.pos (xrefs, prog)
            | Trait ->
              let con_kind = container_decl_predicate TraitContainer in
              process_container_xref con_kind symbol_def occ.pos (xrefs, prog)
            | Const when phys_equal occ.type_ GConst ->
              process_xref
                add_gconst_decl_fact
                build_gconst_decl_json_ref
                symbol_def
                occ.pos
                (xrefs, prog)
            | Enum ->
              process_xref
                add_enum_decl_fact
                build_enum_decl_json_ref
                symbol_def
                occ.pos
                (xrefs, prog)
            | Function ->
              process_xref
                add_func_decl_fact
                build_func_decl_json_ref
                symbol_def
                occ.pos
                (xrefs, prog)
            | _ -> (xrefs, prog)))
  in
  let progress =
    SMap.fold
      (fun fp target_map acc ->
        let (_, res) = add_file_xrefs_fact fp target_map acc in
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
      ("hack.FunctionDefinition.1", progress.resultJson.functionDefinition);
      ("hack.EnumDefinition.1", progress.resultJson.enumDefinition);
      ("hack.PropertyDefinition.1", progress.resultJson.propertyDefinition);
      ("hack.ClassDefinition.1", progress.resultJson.classDefinition);
      ("hack.TraitDefinition.1", progress.resultJson.traitDefinition);
      ("hack.InterfaceDefinition.1", progress.resultJson.interfaceDefinition);
      ("hack.GlobalConstDefinition.1", progress.resultJson.globalConstDefinition);
      ("hack.DeclarationLocation.1", progress.resultJson.declarationLocation);
      ("hack.PropertyDeclaration.1", progress.resultJson.propertyDeclaration);
      ("hack.FunctionDeclaration.1", progress.resultJson.functionDeclaration);
      ("hack.EnumDeclaration.1", progress.resultJson.enumDeclaration);
      ("hack.ClassDeclaration.1", progress.resultJson.classDeclaration);
      ("hack.TraitDeclaration.1", progress.resultJson.traitDeclaration);
      ("hack.InterfaceDeclaration.1", progress.resultJson.interfaceDeclaration);
      ("hack.TypedefDeclaration.1", progress.resultJson.typedefDeclaration);
      ( "hack.GlobalConstDeclaration.1",
        progress.resultJson.globalConstDeclaration );
    ]
  in
  let json_array =
    List.fold preds_and_records ~init:[] ~f:(fun acc (pred, json_lst) ->
        JSON_Object
          [("predicate", JSON_String pred); ("facts", JSON_Array json_lst)]
        :: acc)
  in
  json_array
