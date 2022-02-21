(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Fact_id = Symbol_fact_id
open Hh_prelude

(* Predicate types for the JSON facts emitted *)
type hack =
  | ClassConstDeclaration
  | ClassConstDefinition
  | ClassDeclaration
  | ClassDefinition
  | DeclarationComment
  | DeclarationLocation
  | DeclarationSpan
  | EnumDeclaration
  | EnumDefinition
  | Enumerator
  | FileDeclarations
  | FileXRefs
  | FunctionDeclaration
  | FunctionDefinition
  | GlobalConstDeclaration
  | GlobalConstDefinition
  | InterfaceDeclaration
  | InterfaceDefinition
  | MethodDeclaration
  | MethodDefinition
  | MethodOccurrence
  | MethodOverrides
  | NamespaceDeclaration
  | PropertyDeclaration
  | PropertyDefinition
  | TraitDeclaration
  | TraitDefinition
  | TypeConstDeclaration
  | TypeConstDefinition
  | TypedefDeclaration
  | TypedefDefinition
[@@deriving ord]

type src = FileLines [@@deriving ord]

type t =
  | Hack of hack
  | Src of src
[@@deriving ord]

type predicate = t

let hack_to_string = function
  | ClassConstDeclaration -> "ClassConstDeclaration"
  | ClassConstDefinition -> "ClassConstDefinition"
  | ClassDeclaration -> "ClassDeclaration"
  | ClassDefinition -> "ClassDefinition"
  | DeclarationComment -> "DeclarationComment"
  | DeclarationLocation -> "DeclarationLocation"
  | DeclarationSpan -> "DeclarationSpan"
  | EnumDeclaration -> "EnumDeclaration"
  | EnumDefinition -> "EnumDefinition"
  | Enumerator -> "Enumerator"
  | FileDeclarations -> "FileDeclarations"
  | FileXRefs -> "FileXRefs"
  | FunctionDeclaration -> "FunctionDeclaration"
  | FunctionDefinition -> "FunctionDefinition"
  | GlobalConstDeclaration -> "GlobalConstDeclaration"
  | GlobalConstDefinition -> "GlobalConstDefinition"
  | InterfaceDeclaration -> "InterfaceDeclaration"
  | InterfaceDefinition -> "InterfaceDefinition"
  | MethodDeclaration -> "MethodDeclaration"
  | MethodDefinition -> "MethodDefinition"
  | MethodOccurrence -> "MethodOccurrence"
  | MethodOverrides -> "MethodOverrides"
  | NamespaceDeclaration -> "NamespaceDeclaration"
  | PropertyDeclaration -> "PropertyDeclaration"
  | PropertyDefinition -> "PropertyDefinition"
  | TraitDeclaration -> "TraitDeclaration"
  | TraitDefinition -> "TraitDefinition"
  | TypeConstDeclaration -> "TypeConstDeclaration"
  | TypeConstDefinition -> "TypeConstDefinition"
  | TypedefDeclaration -> "TypedefDeclaration"
  | TypedefDefinition -> "TypedefDefinition"

let src_to_string = function
  | FileLines -> "FileLines"

let to_string = function
  | Hack x -> "hack." ^ hack_to_string x ^ "." ^ Hh_glean_version.hack_version
  | Src x -> "src." ^ src_to_string x ^ ".1"

(* Containers in inheritance relationships which share the four member
types (excludes enum) *)
type parent_container_type =
  | ClassContainer
  | InterfaceContainer
  | TraitContainer

(* Get the container name and predicate type for a given parent
container kind. *)
let parent_decl_predicate parent_container_type =
  match parent_container_type with
  | ClassContainer -> ("class_", Hack ClassDeclaration)
  | InterfaceContainer -> ("interface_", Hack InterfaceDeclaration)
  | TraitContainer -> ("trait", Hack TraitDeclaration)

let get_parent_kind clss =
  match clss.Aast.c_kind with
  | Ast_defs.Cenum_class _ ->
    raise (Failure "Unexpected enum class as parent container kind")
  | Ast_defs.Cenum -> raise (Failure "Unexpected enum as parent container kind")
  | Ast_defs.Cinterface -> InterfaceContainer
  | Ast_defs.Ctrait -> TraitContainer
  | Ast_defs.Cclass _ -> ClassContainer

let should_cache = function
  | Hack ClassConstDeclaration
  | Hack ClassDeclaration
  | Hack EnumDeclaration
  | Hack Enumerator
  | Hack FunctionDeclaration
  | Hack GlobalConstDeclaration
  | Hack InterfaceDeclaration
  | Hack MethodDeclaration
  | Hack PropertyDeclaration
  | Hack TraitDeclaration
  | Hack TypeConstDeclaration
  | Hack TypedefDeclaration ->
    true
  | _ -> false

module Fact_acc = struct
  (* TODO replace this with a predicate-indexed map *)
  type glean_json = {
    classConstDeclaration: Hh_json.json list;
    classConstDefinition: Hh_json.json list;
    classDeclaration: Hh_json.json list;
    classDefinition: Hh_json.json list;
    declarationComment: Hh_json.json list;
    declarationLocation: Hh_json.json list;
    declarationSpan: Hh_json.json list;
    enumDeclaration: Hh_json.json list;
    enumDefinition: Hh_json.json list;
    enumerator: Hh_json.json list;
    fileDeclarations: Hh_json.json list;
    fileLines: Hh_json.json list;
    fileXRefs: Hh_json.json list;
    functionDeclaration: Hh_json.json list;
    functionDefinition: Hh_json.json list;
    globalConstDeclaration: Hh_json.json list;
    globalConstDefinition: Hh_json.json list;
    interfaceDeclaration: Hh_json.json list;
    interfaceDefinition: Hh_json.json list;
    methodDeclaration: Hh_json.json list;
    methodDefinition: Hh_json.json list;
    methodOccurrence: Hh_json.json list;
    methodOverrides: Hh_json.json list;
    namespaceDeclaration: Hh_json.json list;
    propertyDeclaration: Hh_json.json list;
    propertyDefinition: Hh_json.json list;
    traitDeclaration: Hh_json.json list;
    traitDefinition: Hh_json.json list;
    typeConstDeclaration: Hh_json.json list;
    typeConstDefinition: Hh_json.json list;
    typedefDeclaration: Hh_json.json list;
    typedefDefinition: Hh_json.json list;
  }

  module JsonPredicateMap = WrappedMap.Make (struct
    type json = Hh_json.json

    let compare_json = Hh_json.JsonKey.compare

    let compare_predicate = compare

    type t = json * predicate [@@deriving ord]
  end)

  type t = {
    resultJson: glean_json;
    factIds: Fact_id.t JsonPredicateMap.t;
  }

  let update_glean_json predicate json factkey_opt progress =
    let resultJson =
      match predicate with
      | Hack ClassConstDeclaration ->
        {
          progress.resultJson with
          classConstDeclaration =
            json :: progress.resultJson.classConstDeclaration;
        }
      | Hack ClassConstDefinition ->
        {
          progress.resultJson with
          classConstDefinition =
            json :: progress.resultJson.classConstDefinition;
        }
      | Hack ClassDeclaration ->
        {
          progress.resultJson with
          classDeclaration = json :: progress.resultJson.classDeclaration;
        }
      | Hack ClassDefinition ->
        {
          progress.resultJson with
          classDefinition = json :: progress.resultJson.classDefinition;
        }
      | Hack DeclarationComment ->
        {
          progress.resultJson with
          declarationComment = json :: progress.resultJson.declarationComment;
        }
      | Hack DeclarationLocation ->
        {
          progress.resultJson with
          declarationLocation = json :: progress.resultJson.declarationLocation;
        }
      | Hack DeclarationSpan ->
        {
          progress.resultJson with
          declarationSpan = json :: progress.resultJson.declarationSpan;
        }
      | Hack EnumDeclaration ->
        {
          progress.resultJson with
          enumDeclaration = json :: progress.resultJson.enumDeclaration;
        }
      | Hack EnumDefinition ->
        {
          progress.resultJson with
          enumDefinition = json :: progress.resultJson.enumDefinition;
        }
      | Hack Enumerator ->
        {
          progress.resultJson with
          enumerator = json :: progress.resultJson.enumerator;
        }
      | Hack FileDeclarations ->
        {
          progress.resultJson with
          fileDeclarations = json :: progress.resultJson.fileDeclarations;
        }
      | Src FileLines ->
        {
          progress.resultJson with
          fileLines = json :: progress.resultJson.fileLines;
        }
      | Hack FileXRefs ->
        {
          progress.resultJson with
          fileXRefs = json :: progress.resultJson.fileXRefs;
        }
      | Hack FunctionDeclaration ->
        {
          progress.resultJson with
          functionDeclaration = json :: progress.resultJson.functionDeclaration;
        }
      | Hack FunctionDefinition ->
        {
          progress.resultJson with
          functionDefinition = json :: progress.resultJson.functionDefinition;
        }
      | Hack GlobalConstDeclaration ->
        {
          progress.resultJson with
          globalConstDeclaration =
            json :: progress.resultJson.globalConstDeclaration;
        }
      | Hack GlobalConstDefinition ->
        {
          progress.resultJson with
          globalConstDefinition =
            json :: progress.resultJson.globalConstDefinition;
        }
      | Hack InterfaceDeclaration ->
        {
          progress.resultJson with
          interfaceDeclaration =
            json :: progress.resultJson.interfaceDeclaration;
        }
      | Hack InterfaceDefinition ->
        {
          progress.resultJson with
          interfaceDefinition = json :: progress.resultJson.interfaceDefinition;
        }
      | Hack MethodDeclaration ->
        {
          progress.resultJson with
          methodDeclaration = json :: progress.resultJson.methodDeclaration;
        }
      | Hack MethodDefinition ->
        {
          progress.resultJson with
          methodDefinition = json :: progress.resultJson.methodDefinition;
        }
      | Hack MethodOccurrence ->
        {
          progress.resultJson with
          methodOccurrence = json :: progress.resultJson.methodOccurrence;
        }
      | Hack MethodOverrides ->
        {
          progress.resultJson with
          methodOverrides = json :: progress.resultJson.methodOverrides;
        }
      | Hack NamespaceDeclaration ->
        {
          progress.resultJson with
          namespaceDeclaration =
            json :: progress.resultJson.namespaceDeclaration;
        }
      | Hack PropertyDeclaration ->
        {
          progress.resultJson with
          propertyDeclaration = json :: progress.resultJson.propertyDeclaration;
        }
      | Hack PropertyDefinition ->
        {
          progress.resultJson with
          propertyDefinition = json :: progress.resultJson.propertyDefinition;
        }
      | Hack TraitDeclaration ->
        {
          progress.resultJson with
          traitDeclaration = json :: progress.resultJson.traitDeclaration;
        }
      | Hack TraitDefinition ->
        {
          progress.resultJson with
          traitDefinition = json :: progress.resultJson.traitDefinition;
        }
      | Hack TypeConstDeclaration ->
        {
          progress.resultJson with
          typeConstDeclaration =
            json :: progress.resultJson.typeConstDeclaration;
        }
      | Hack TypeConstDefinition ->
        {
          progress.resultJson with
          typeConstDefinition = json :: progress.resultJson.typeConstDefinition;
        }
      | Hack TypedefDeclaration ->
        {
          progress.resultJson with
          typedefDeclaration = json :: progress.resultJson.typedefDeclaration;
        }
      | Hack TypedefDefinition ->
        {
          progress.resultJson with
          typedefDefinition = json :: progress.resultJson.typedefDefinition;
        }
    in
    let factIds =
      match factkey_opt with
      | None -> progress.factIds
      | Some (fact_id, json_key) ->
        JsonPredicateMap.add (json_key, predicate) fact_id progress.factIds
    in
    { resultJson; factIds }

  let add_fact predicate json_key progress =
    let fact_id = Fact_id.next () in
    let json_fact =
      Hh_json.JSON_Object
        [("id", Fact_id.to_json_number fact_id); ("key", json_key)]
    in
    match
      ( should_cache predicate,
        JsonPredicateMap.find_opt (json_key, predicate) progress.factIds )
    with
    | (false, _) ->
      (fact_id, update_glean_json predicate json_fact None progress)
    | (true, None) ->
      ( fact_id,
        update_glean_json
          predicate
          json_fact
          (Some (fact_id, json_key))
          progress )
    | (true, Some fid) -> (fid, progress)

  let init =
    let resultJson =
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
    { resultJson; factIds = JsonPredicateMap.empty }

  let to_json progress =
    let resultJson = progress.resultJson in
    let preds =
      (* The order is the reverse of how these items appear in the JSON,
         which is significant because later entries can refer to earlier ones
         by id only *)
      List.map
        ~f:(fun (pred, res) -> (to_string pred, res))
        [
          (Src FileLines, resultJson.fileLines);
          (Hack FileDeclarations, resultJson.fileDeclarations);
          (Hack FileXRefs, resultJson.fileXRefs);
          (Hack MethodOverrides, resultJson.methodOverrides);
          (Hack MethodDefinition, resultJson.methodDefinition);
          (Hack FunctionDefinition, resultJson.functionDefinition);
          (Hack EnumDefinition, resultJson.enumDefinition);
          (Hack ClassConstDefinition, resultJson.classConstDefinition);
          (Hack PropertyDefinition, resultJson.propertyDefinition);
          (Hack TypeConstDefinition, resultJson.typeConstDefinition);
          (Hack ClassDefinition, resultJson.classDefinition);
          (Hack TraitDefinition, resultJson.traitDefinition);
          (Hack InterfaceDefinition, resultJson.interfaceDefinition);
          (Hack TypedefDefinition, resultJson.typedefDefinition);
          (Hack GlobalConstDefinition, resultJson.globalConstDefinition);
          (Hack DeclarationComment, resultJson.declarationComment);
          (Hack DeclarationLocation, resultJson.declarationLocation);
          (Hack DeclarationSpan, resultJson.declarationSpan);
          (Hack MethodDeclaration, resultJson.methodDeclaration);
          (Hack ClassConstDeclaration, resultJson.classConstDeclaration);
          (Hack PropertyDeclaration, resultJson.propertyDeclaration);
          (Hack TypeConstDeclaration, resultJson.typeConstDeclaration);
          (Hack FunctionDeclaration, resultJson.functionDeclaration);
          (Hack Enumerator, resultJson.enumerator);
          (Hack EnumDeclaration, resultJson.enumDeclaration);
          (Hack ClassDeclaration, resultJson.classDeclaration);
          (Hack TraitDeclaration, resultJson.traitDeclaration);
          (Hack InterfaceDeclaration, resultJson.interfaceDeclaration);
          (Hack TypedefDeclaration, resultJson.typedefDeclaration);
          (Hack GlobalConstDeclaration, resultJson.globalConstDeclaration);
          (Hack NamespaceDeclaration, resultJson.namespaceDeclaration);
          (Hack MethodOccurrence, resultJson.methodOccurrence);
        ]
    in
    let json_array =
      List.fold preds ~init:[] ~f:(fun acc (pred, json_lst) ->
          Hh_json.(
            JSON_Object
              [("predicate", JSON_String pred); ("facts", JSON_Array json_lst)]
            :: acc))
    in
    json_array
end
