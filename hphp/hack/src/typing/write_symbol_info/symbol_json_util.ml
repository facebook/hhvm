(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hh_json

let get_next_elem_id () =
  let x = ref 1 in
  fun () ->
    let r = !x in
    x := !x + 1;
    r

let json_element_id = get_next_elem_id ()

let get_type_from_hint ctx h =
  let mode = FileInfo.Mhhi in
  let decl_env = Decl_env.{ mode; droot = None; ctx } in
  let tcopt = Provider_context.get_tcopt ctx in
  Typing_print.full_decl tcopt (Decl_hint.hint decl_env h)

(* Replace any codepoints that are not valid UTF-8 with
the unrepresentable character. *)
let check_utf8 str =
  let b = Buffer.create (String.length str) in
  let replace_malformed () _index = function
    | `Uchar u -> Uutf.Buffer.add_utf_8 b u
    | `Malformed _ -> Uutf.Buffer.add_utf_8 b Uutf.u_rep
  in
  Uutf.String.fold_utf_8 replace_malformed () str;
  Buffer.contents b

let source_at_span source_text pos =
  let st = Pos.start_offset pos in
  let fi = Pos.end_offset pos in
  let source_text = Full_fidelity_source_text.sub source_text st (fi - st) in
  check_utf8 source_text

let strip_nested_quotes str =
  let len = String.length str in
  let firstc = str.[0] in
  let lastc = str.[len - 1] in
  if
    len >= 2
    && ((Char.equal '"' firstc && Char.equal '"' lastc)
       || (Char.equal '\'' firstc && Char.equal '\'' lastc))
  then
    String.sub str ~pos:1 ~len:(len - 2)
  else
    str

let strip_tparams name =
  match String.index name '<' with
  | None -> name
  | Some i -> String.sub name ~pos:0 ~len:i

let ends_in_newline source_text =
  let last_char =
    Full_fidelity_source_text.(get source_text (source_text.length - 1))
  in
  Char.equal '\n' last_char || Char.equal '\r' last_char

let has_tabs_or_multibyte_codepoints source_text =
  let open Full_fidelity_source_text in
  let check_codepoint (num, found) _index = function
    | `Uchar u -> (num + 1, found || Uchar.equal u (Uchar.of_char '\t'))
    | `Malformed _ -> (num + 1, true)
  in
  let (num_chars, found_tab_or_malformed) =
    Uutf.String.fold_utf_8 check_codepoint (0, false) source_text.text
  in
  found_tab_or_malformed || num_chars < source_text.length

let rec find_fid fid_list pred =
  match fid_list with
  | [] -> None
  | (p, fid) :: tail ->
    if phys_equal p pred then
      Some fid
    else
      find_fid tail pred

let split_name (s : string) : (string * string) option =
  match String.rindex s '\\' with
  | None -> None
  | Some pos ->
    let name_start = pos + 1 in
    let name =
      String.sub s ~pos:name_start ~len:(String.length s - name_start)
    in
    let parent_namespace = String.sub s ~pos:0 ~len:(name_start - 1) in
    if String.is_empty parent_namespace || String.is_empty name then
      None
    else
      Some (parent_namespace, name)

let parent_decl_predicate parent_container_type =
  let open Symbol_builder_types in
  match parent_container_type with
  | ClassContainer -> ("class_", ClassDeclaration)
  | InterfaceContainer -> ("interface_", InterfaceDeclaration)
  | TraitContainer -> ("trait", TraitDeclaration)

let get_parent_kind clss =
  match clss.Aast.c_kind with
  | Ast_defs.Cenum_class _ ->
    raise (Failure "Unexpected enum class as parent container kind")
  | Ast_defs.Cenum -> raise (Failure "Unexpected enum as parent container kind")
  | Ast_defs.Cinterface -> Symbol_builder_types.InterfaceContainer
  | Ast_defs.Ctrait -> Symbol_builder_types.TraitContainer
  | Ast_defs.Cclass _ -> Symbol_builder_types.ClassContainer

let init_progress =
  let default_json =
    Symbol_builder_types.
      {
        classConstDeclaration = [];
        classConstDefinition = [];
        classDeclaration = [];
        classDefinition = [];
        declarationComment = [];
        declarationLocation = [];
        declarationSpan = [];
        enumDeclaration = [];
        enumDefinition = [];
        enumerator = [];
        fileDeclarations = [];
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
        methodOccurrence = [];
        methodOverrides = [];
        namespaceDeclaration = [];
        propertyDeclaration = [];
        propertyDefinition = [];
        traitDeclaration = [];
        traitDefinition = [];
        typeConstDeclaration = [];
        typeConstDefinition = [];
        typedefDeclaration = [];
        typedefDefinition = [];
      }
  in
  Symbol_builder_types.{ resultJson = default_json; factIds = JMap.empty }

let should_cache predicate =
  let open Symbol_builder_types in
  match predicate with
  | ClassConstDeclaration
  | ClassDeclaration
  | EnumDeclaration
  | Enumerator
  | FunctionDeclaration
  | GlobalConstDeclaration
  | InterfaceDeclaration
  | MethodDeclaration
  | PropertyDeclaration
  | TraitDeclaration
  | TypeConstDeclaration
  | TypedefDeclaration ->
    true
  | _ -> false

let update_json_data predicate json progress =
  let open Symbol_builder_types in
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
    | DeclarationComment ->
      {
        progress.resultJson with
        declarationComment = json :: progress.resultJson.declarationComment;
      }
    | DeclarationLocation ->
      {
        progress.resultJson with
        declarationLocation = json :: progress.resultJson.declarationLocation;
      }
    | DeclarationSpan ->
      {
        progress.resultJson with
        declarationSpan = json :: progress.resultJson.declarationSpan;
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
    | Enumerator ->
      {
        progress.resultJson with
        enumerator = json :: progress.resultJson.enumerator;
      }
    | FileDeclarations ->
      {
        progress.resultJson with
        fileDeclarations = json :: progress.resultJson.fileDeclarations;
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
    | MethodOccurrence ->
      {
        progress.resultJson with
        methodOccurrence = json :: progress.resultJson.methodOccurrence;
      }
    | MethodOverrides ->
      {
        progress.resultJson with
        methodOverrides = json :: progress.resultJson.methodOverrides;
      }
    | NamespaceDeclaration ->
      {
        progress.resultJson with
        namespaceDeclaration = json :: progress.resultJson.namespaceDeclaration;
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
    | TypedefDefinition ->
      {
        progress.resultJson with
        typedefDefinition = json :: progress.resultJson.typedefDefinition;
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
      Symbol_builder_types.
        {
          resultJson = progress.resultJson;
          factIds =
            (if should_cache predicate then
              JMap.add
                json_key
                [(predicate, newFactId)]
                progress.factIds
                ~combine:List.append
            else
              progress.factIds);
        }
    in
    (newFactId, true, progress)
  in
  let (id, is_new, progress) =
    if should_cache predicate then
      match JMap.find_opt json_key progress.Symbol_builder_types.factIds with
      | None -> add_id
      | Some fid_list ->
        (match find_fid fid_list predicate with
        | None -> add_id
        | Some fid -> (fid, false, progress))
    else
      add_id
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
