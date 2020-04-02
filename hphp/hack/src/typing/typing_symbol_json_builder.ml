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

type symbol_occurrences = {
  decls: Tast.def list;
  occurrences: Relative_path.t SymbolOccurrence.t list;
}

type file_lines = {
  filepath: Relative_path.t;
  lineLengths: int list;
  endsInNewline: bool;
  hasUnicodeOrTabs: bool;
}

(* Predicate types for the JSON facts emitted *)
type predicate =
  | ClassConstDeclaration
  | ClassConstDefinition
  | ClassDeclaration
  | ClassDefinition
  | DeclarationLocation
  | EnumDeclaration
  | EnumDefinition
  | FileLines
  | FileXRefs
  | FunctionDeclaration
  | FunctionDefinition
  | GlobalConstDeclaration
  | GlobalConstDefinition
  | InterfaceDeclaration
  | InterfaceDefinition
  | MethodDeclaration
  | MethodDefinition
  | PropertyDeclaration
  | PropertyDefinition
  | TraitDeclaration
  | TraitDefinition
  | TypeConstDeclaration
  | TypeConstDefinition
  | TypedefDeclaration

(* Containers that can be in inheritance relationships *)
type container_type =
  | ClassContainer
  | InterfaceContainer
  | TraitContainer

type glean_json = {
  classConstDeclaration: json list;
  classConstDefinition: json list;
  classDeclaration: json list;
  classDefinition: json list;
  declarationLocation: json list;
  enumDeclaration: json list;
  enumDefinition: json list;
  fileLines: json list;
  fileXRefs: json list;
  functionDeclaration: json list;
  functionDefinition: json list;
  globalConstDeclaration: json list;
  globalConstDefinition: json list;
  interfaceDeclaration: json list;
  interfaceDefinition: json list;
  methodDeclaration: json list;
  methodDefinition: json list;
  propertyDeclaration: json list;
  propertyDefinition: json list;
  traitDeclaration: json list;
  traitDefinition: json list;
  typeConstDeclaration: json list;
  typeConstDefinition: json list;
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
      classConstDeclaration = [];
      classConstDefinition = [];
      classDeclaration = [];
      classDefinition = [];
      declarationLocation = [];
      enumDeclaration = [];
      enumDefinition = [];
      fileLines = [];
      fileXRefs = [];
      functionDeclaration = [];
      functionDefinition = [];
      globalConstDeclaration = [];
      globalConstDefinition = [];
      interfaceDeclaration = [];
      interfaceDefinition = [];
      methodDeclaration = [];
      methodDefinition = [];
      propertyDeclaration = [];
      propertyDefinition = [];
      traitDeclaration = [];
      traitDefinition = [];
      typeConstDeclaration = [];
      typeConstDefinition = [];
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
    | ClassConstDeclaration ->
      {
        progress.resultJson with
        classConstDeclaration =
          json :: progress.resultJson.classConstDeclaration;
      }
    | ClassConstDefinition ->
      {
        progress.resultJson with
        classConstDefinition = json :: progress.resultJson.classConstDefinition;
      }
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
    | FileLines ->
      {
        progress.resultJson with
        fileLines = json :: progress.resultJson.fileLines;
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
    | MethodDeclaration ->
      {
        progress.resultJson with
        methodDeclaration = json :: progress.resultJson.methodDeclaration;
      }
    | MethodDefinition ->
      {
        progress.resultJson with
        methodDefinition = json :: progress.resultJson.methodDefinition;
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
    | TypeConstDeclaration ->
      {
        progress.resultJson with
        typeConstDeclaration = json :: progress.resultJson.typeConstDeclaration;
      }
    | TypeConstDefinition ->
      {
        progress.resultJson with
        typeConstDefinition = json :: progress.resultJson.typeConstDefinition;
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

let build_is_async_json fun_kind =
  let is_async =
    match fun_kind with
    | FAsync -> true
    | FAsyncGenerator -> true
    | _ -> false
  in
  JSON_Bool is_async

let build_signature_json ctx params ret_ty =
  let parameters =
    List.map params (fun param ->
        let ty =
          match hint_of_type_hint param.param_type_hint with
          | None -> None
          | Some h -> Some (get_type_from_hint ctx h)
        in
        build_parameter_json param.param_name ty)
  in
  let return_type_name =
    match hint_of_type_hint ret_ty with
    | None -> None
    | Some h -> Some (get_type_from_hint ctx h)
  in
  build_signature_json_nested parameters return_type_name

let build_type_const_kind_json kind =
  let num =
    match kind with
    | TCAbstract _ -> 0
    | TCConcrete -> 1
    | TCPartiallyAbstract -> 2
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

let build_file_lines_json file_info =
  let file = build_file_json (Relative_path.to_absolute file_info.filepath) in
  let lengths =
    List.map file_info.lineLengths (fun len -> JSON_Number (string_of_int len))
  in
  JSON_Object
    [
      ("file", file);
      ("lengths", JSON_Array lengths);
      ("endsInNewline", JSON_Bool file_info.endsInNewline);
      ("hasUnicodeOrTabs", JSON_Bool file_info.hasUnicodeOrTabs);
    ]

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
  JSON_Object [("globalConst", build_id_json fact_id)]

let build_property_decl_json_ref fact_id =
  JSON_Object [("property_", build_id_json fact_id)]

let build_class_const_decl_json_ref fact_id =
  JSON_Object [("classConst", build_id_json fact_id)]

let build_type_const_decl_json_ref fact_id =
  JSON_Object [("typeConst", build_id_json fact_id)]

let build_method_decl_json_ref fact_id =
  JSON_Object [("method", build_id_json fact_id)]

(* These functions build up the JSON necessary and then add facts
to the running result. *)

let add_container_defn_fact clss decl_id member_decls progress =
  let base_fields =
    [
      ("declaration", build_id_json decl_id);
      ("members", JSON_Array member_decls);
    ]
  in
  let (defn_pred, json_fields) =
    match get_container_kind clss with
    | InterfaceContainer -> (InterfaceDefinition, base_fields)
    | TraitContainer -> (TraitDefinition, base_fields)
    | ClassContainer ->
      let is_abstract =
        match clss.c_kind with
        | Cabstract -> true
        | _ -> false
      in
      let class_fields =
        base_fields
        @ [
            ("isAbstract", JSON_Bool is_abstract);
            ("isFinal", JSON_Bool clss.c_final);
          ]
      in
      (ClassDefinition, class_fields)
  in
  add_fact defn_pred (JSON_Object json_fields) progress

let add_container_decl_fact decl_pred name progress =
  let json_fact = JSON_Object [("name", build_name_json_nested name)] in
  add_fact decl_pred json_fact progress

let add_property_decl_fact con_type decl_id name progress =
  let json_fact =
    JSON_Object
      [
        ("name", build_name_json_nested name);
        ("container", build_container_json_ref con_type decl_id);
      ]
  in
  add_fact PropertyDeclaration json_fact progress

let add_class_const_decl_fact con_type decl_id name progress =
  let json_fact =
    JSON_Object
      [
        ("name", build_name_json_nested name);
        ("container", build_container_json_ref con_type decl_id);
      ]
  in
  add_fact ClassConstDeclaration json_fact progress

let add_type_const_decl_fact con_type decl_id name progress =
  let json_fact =
    JSON_Object
      [
        ("name", build_name_json_nested name);
        ("container", build_container_json_ref con_type decl_id);
      ]
  in
  add_fact TypeConstDeclaration json_fact progress

let add_method_decl_fact con_type decl_id name progress =
  let json_fact =
    JSON_Object
      [
        ("name", build_name_json_nested name);
        ("container", build_container_json_ref con_type decl_id);
      ]
  in
  add_fact MethodDeclaration json_fact progress

let add_method_defn_fact ctx meth decl_id progress =
  let json_fact =
    JSON_Object
      [
        ("declaration", build_id_json decl_id);
        ("signature", build_signature_json ctx meth.m_params meth.m_ret);
        ("visibility", build_visibility_json meth.m_visibility);
        ("isAbstract", JSON_Bool meth.m_abstract);
        ("isAsync", build_is_async_json meth.m_fun_kind);
        ("isFinal", JSON_Bool meth.m_final);
        ("isStatic", JSON_Bool meth.m_static);
      ]
  in
  add_fact MethodDefinition json_fact progress

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

let add_class_const_defn_fact ctx const decl_id progress =
  let base_fields =
    [
      ("declaration", build_id_json decl_id);
      ("isAbstract", JSON_Bool (is_none const.cc_expr));
    ]
  in
  let json_fields =
    match const.cc_type with
    | None -> base_fields
    | Some h ->
      let ty = get_type_from_hint ctx h in
      ("type", build_type_json_nested ty) :: base_fields
  in
  add_fact ClassConstDefinition (JSON_Object json_fields) progress

let add_type_const_defn_fact ctx tc decl_id progress =
  let base_fields =
    [
      ("declaration", build_id_json decl_id);
      ("kind", build_type_const_kind_json tc.c_tconst_abstract);
    ]
  in
  let json_fields =
    match tc.c_tconst_type with
    | None -> base_fields
    | Some h ->
      let ty = get_type_from_hint ctx h in
      ("type", build_type_json_nested ty) :: base_fields
  in
  add_fact TypeConstDefinition (JSON_Object json_fields) progress

let add_enum_decl_fact name progress =
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

let add_func_decl_fact name progress =
  let json_fact = JSON_Object [("name", build_name_json_nested name)] in
  add_fact FunctionDeclaration json_fact progress

let add_func_defn_fact ctx elem decl_id progress =
  let json_fact =
    JSON_Object
      [
        ("declaration", build_id_json decl_id);
        ("signature", build_signature_json ctx elem.f_params elem.f_ret);
        ("isAsync", build_is_async_json elem.f_fun_kind);
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
        ("isTransparent", JSON_Bool is_transparent);
      ]
  in
  add_fact TypedefDeclaration json_fact progress

let add_gconst_decl_fact name progress =
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

let add_file_lines_fact file_info progress =
  let json_fact = build_file_lines_json file_info in
  add_fact FileLines json_fact progress

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
  let (target_id, prog) = decl_fun symbol_def.name progress in
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

let process_member_xref
    ctx member pos con_name mem_decl_fun ref_fun (xrefs, prog) =
  match ServerSymbolDefinition.get_class_by_name ctx con_name with
  | None -> (xrefs, prog)
  | Some cls ->
    let con_kind = get_container_kind cls in
    let (con_type, decl_pred) = container_decl_predicate con_kind in
    let (con_decl_id, prog) = add_container_decl_fact decl_pred con_type prog in
    process_xref
      (mem_decl_fun con_type con_decl_id)
      ref_fun
      member
      pos
      (xrefs, prog)

let process_gconst_xref symbol_def pos (xrefs, progress) =
  process_xref
    add_gconst_decl_fact
    build_gconst_decl_json_ref
    symbol_def
    pos
    (xrefs, progress)

let process_enum_xref symbol_def pos (xrefs, progress) =
  process_xref
    add_enum_decl_fact
    build_enum_decl_json_ref
    symbol_def
    pos
    (xrefs, progress)

let process_function_xref symbol_def pos (xrefs, progress) =
  process_xref
    add_func_decl_fact
    build_func_decl_json_ref
    symbol_def
    pos
    (xrefs, progress)

let process_decl_loc decl_fun defn_fun decl_ref_fun pos id elem progress =
  let (decl_id, prog) = decl_fun id progress in
  let (_, prog) = defn_fun elem decl_id prog in
  let ref_json = decl_ref_fun decl_id in
  let (_, prog) = add_decl_loc_fact pos ref_json prog in
  (decl_id, prog)

let process_container_decl ctx elem progress =
  let (pos, id) = elem.c_name in
  let (con_type, decl_pred) =
    container_decl_predicate (get_container_kind elem)
  in
  let (decl_id, prog) = add_container_decl_fact decl_pred id progress in
  let (prop_decls, prog) =
    List.fold elem.c_vars ~init:([], prog) ~f:(fun (decls, prog) prop ->
        let (pos, id) = prop.cv_id in
        let (decl_id, prog) =
          process_decl_loc
            (add_property_decl_fact con_type decl_id)
            (add_property_defn_fact ctx)
            build_property_decl_json_ref
            pos
            id
            prop
            prog
        in
        (build_property_decl_json_ref decl_id :: decls, prog))
  in
  let (class_const_decls, prog) =
    List.fold elem.c_consts ~init:([], prog) ~f:(fun (decls, prog) const ->
        let (pos, id) = const.cc_id in
        let (decl_id, prog) =
          process_decl_loc
            (add_class_const_decl_fact con_type decl_id)
            (add_class_const_defn_fact ctx)
            build_class_const_decl_json_ref
            pos
            id
            const
            prog
        in
        (build_class_const_decl_json_ref decl_id :: decls, prog))
  in
  let (type_const_decls, prog) =
    List.fold elem.c_typeconsts ~init:([], prog) ~f:(fun (decls, prog) tc ->
        let (pos, id) = tc.c_tconst_name in
        let (decl_id, prog) =
          process_decl_loc
            (add_type_const_decl_fact con_type decl_id)
            (add_type_const_defn_fact ctx)
            build_type_const_decl_json_ref
            pos
            id
            tc
            prog
        in
        (build_type_const_decl_json_ref decl_id :: decls, prog))
  in
  let (method_decls, prog) =
    List.fold elem.c_methods ~init:([], prog) ~f:(fun (decls, prog) meth ->
        let (pos, id) = meth.m_name in
        let (decl_id, prog) =
          process_decl_loc
            (add_method_decl_fact con_type decl_id)
            (add_method_defn_fact ctx)
            build_method_decl_json_ref
            pos
            id
            meth
            prog
        in
        (build_method_decl_json_ref decl_id :: decls, prog))
  in
  let members =
    prop_decls @ class_const_decls @ type_const_decls @ method_decls
  in
  let (_, prog) = add_container_defn_fact elem decl_id members prog in
  let ref_json = build_container_decl_json_ref con_type decl_id in
  let (_, prog) = add_decl_loc_fact pos ref_json prog in
  prog

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
let build_json ctx symbols files_info =
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
          | Some sym_def ->
            let proc_mem = process_member_xref ctx sym_def occ.pos in
            (match sym_def.kind with
            | Class ->
              let con_kind = container_decl_predicate ClassContainer in
              process_container_xref con_kind sym_def occ.pos (xrefs, prog)
            | Const ->
              (match occ.type_ with
              | ClassConst (cn, _) ->
                let ref_fun = build_class_const_decl_json_ref in
                proc_mem cn add_class_const_decl_fact ref_fun (xrefs, prog)
              | GConst -> process_gconst_xref sym_def occ.pos (xrefs, prog)
              | _ -> (xrefs, prog))
            | Enum -> process_enum_xref sym_def occ.pos (xrefs, prog)
            | Function -> process_function_xref sym_def occ.pos (xrefs, prog)
            | Interface ->
              let con_kind = container_decl_predicate InterfaceContainer in
              process_container_xref con_kind sym_def occ.pos (xrefs, prog)
            | Method ->
              (match occ.type_ with
              | Method (cn, _) ->
                let ref_fun = build_method_decl_json_ref in
                proc_mem cn add_method_decl_fact ref_fun (xrefs, prog)
              | _ -> (xrefs, prog))
            | Property ->
              (match occ.type_ with
              | Property (cn, _) ->
                let ref_fun = build_property_decl_json_ref in
                proc_mem cn add_property_decl_fact ref_fun (xrefs, prog)
              | _ -> (xrefs, prog))
            | Typeconst ->
              (match occ.type_ with
              | Typeconst (cn, _) ->
                let ref_fun = build_type_const_decl_json_ref in
                proc_mem cn add_type_const_decl_fact ref_fun (xrefs, prog)
              | _ -> (xrefs, prog))
            | Trait ->
              let con_kind = container_decl_predicate TraitContainer in
              process_container_xref con_kind sym_def occ.pos (xrefs, prog)
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
  let progress =
    List.fold files_info ~init:progress ~f:(fun prog file_info ->
        let (_, prog) = add_file_lines_fact file_info prog in
        prog)
  in
  let preds_and_records =
    (* The order is the reverse of how these items appear in the JSON,
    which is significant because later entries can refer to earlier ones
    by id only *)
    [
      ("src.FileLines.1", progress.resultJson.fileLines);
      ("hack.FileXRefs.1", progress.resultJson.fileXRefs);
      ("hack.MethodDefinition.1", progress.resultJson.methodDefinition);
      ("hack.FunctionDefinition.1", progress.resultJson.functionDefinition);
      ("hack.EnumDefinition.1", progress.resultJson.enumDefinition);
      ("hack.ClassConstDefinition.1", progress.resultJson.classConstDefinition);
      ("hack.PropertyDefinition.1", progress.resultJson.propertyDefinition);
      ("hack.TypeConstDefinition.1", progress.resultJson.typeConstDefinition);
      ("hack.ClassDefinition.1", progress.resultJson.classDefinition);
      ("hack.TraitDefinition.1", progress.resultJson.traitDefinition);
      ("hack.InterfaceDefinition.1", progress.resultJson.interfaceDefinition);
      ("hack.GlobalConstDefinition.1", progress.resultJson.globalConstDefinition);
      ("hack.DeclarationLocation.1", progress.resultJson.declarationLocation);
      ("hack.MethodDeclaration.1", progress.resultJson.methodDeclaration);
      ("hack.ClassConstDeclaration.1", progress.resultJson.classConstDeclaration);
      ("hack.PropertyDeclaration.1", progress.resultJson.propertyDeclaration);
      ("hack.TypeConstDeclaration.1", progress.resultJson.typeConstDeclaration);
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
