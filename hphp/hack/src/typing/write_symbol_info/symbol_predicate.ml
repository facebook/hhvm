(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Fact_id = Symbol_fact_id
open Hh_prelude
open Hh_json

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
  type ownership_unit = string option [@@deriving eq, ord]

  type owned_facts = (ownership_unit * json list) list

  (* TODO replace this with a predicate-indexed map *)
  type glean_json = {
    classConstDeclaration: owned_facts;
    classConstDefinition: owned_facts;
    classDeclaration: owned_facts;
    classDefinition: owned_facts;
    declarationComment: owned_facts;
    declarationLocation: owned_facts;
    declarationSpan: owned_facts;
    enumDeclaration: owned_facts;
    enumDefinition: owned_facts;
    enumerator: owned_facts;
    fileDeclarations: owned_facts;
    fileLines: owned_facts;
    fileXRefs: owned_facts;
    functionDeclaration: owned_facts;
    functionDefinition: owned_facts;
    globalConstDeclaration: owned_facts;
    globalConstDefinition: owned_facts;
    interfaceDeclaration: owned_facts;
    interfaceDefinition: owned_facts;
    methodDeclaration: owned_facts;
    methodDefinition: owned_facts;
    methodOccurrence: owned_facts;
    methodOverrides: owned_facts;
    namespaceDeclaration: owned_facts;
    propertyDeclaration: owned_facts;
    propertyDefinition: owned_facts;
    traitDeclaration: owned_facts;
    traitDefinition: owned_facts;
    typeConstDeclaration: owned_facts;
    typeConstDefinition: owned_facts;
    typedefDeclaration: owned_facts;
    typedefDefinition: owned_facts;
  }

  module JsonPredicateMap = WrappedMap.Make (struct
    let compare_json = JsonKey.compare

    let compare_predicate = compare

    type t = ownership_unit * json * predicate [@@deriving ord]
  end)

  type t = {
    resultJson: glean_json;
    factIds: Fact_id.t JsonPredicateMap.t;
    mutable ownership_unit: ownership_unit;
    ownership: bool;
  }

  (* if ownership is set, we distinguish facts with different owners,
     otherwise we "collapse" the ownership_unit to None. *)
  let cache_key ownership ownership_unit json_key predicate =
    if ownership then
      (ownership_unit, json_key, predicate)
    else
      (None, json_key, predicate)

  let add_to_owned_facts ownership_unit json = function
    | (ou, jsons) :: l when equal_ownership_unit ou ownership_unit ->
      (ownership_unit, json :: jsons) :: l
    | l -> (ownership_unit, [json]) :: l

  let update_glean_json
      predicate json factkey_opt ({ ownership; ownership_unit; _ } as progress)
      =
    let add = add_to_owned_facts ownership_unit in
    let resultJson =
      match predicate with
      | Hack ClassConstDeclaration ->
        {
          progress.resultJson with
          classConstDeclaration =
            add json progress.resultJson.classConstDeclaration;
        }
      | Hack ClassConstDefinition ->
        {
          progress.resultJson with
          classConstDefinition =
            add json progress.resultJson.classConstDefinition;
        }
      | Hack ClassDeclaration ->
        {
          progress.resultJson with
          classDeclaration = add json progress.resultJson.classDeclaration;
        }
      | Hack ClassDefinition ->
        {
          progress.resultJson with
          classDefinition = add json progress.resultJson.classDefinition;
        }
      | Hack DeclarationComment ->
        {
          progress.resultJson with
          declarationComment = add json progress.resultJson.declarationComment;
        }
      | Hack DeclarationLocation ->
        {
          progress.resultJson with
          declarationLocation = add json progress.resultJson.declarationLocation;
        }
      | Hack DeclarationSpan ->
        {
          progress.resultJson with
          declarationSpan = add json progress.resultJson.declarationSpan;
        }
      | Hack EnumDeclaration ->
        {
          progress.resultJson with
          enumDeclaration = add json progress.resultJson.enumDeclaration;
        }
      | Hack EnumDefinition ->
        {
          progress.resultJson with
          enumDefinition = add json progress.resultJson.enumDefinition;
        }
      | Hack Enumerator ->
        {
          progress.resultJson with
          enumerator = add json progress.resultJson.enumerator;
        }
      | Hack FileDeclarations ->
        {
          progress.resultJson with
          fileDeclarations = add json progress.resultJson.fileDeclarations;
        }
      | Src FileLines ->
        {
          progress.resultJson with
          fileLines = add json progress.resultJson.fileLines;
        }
      | Hack FileXRefs ->
        {
          progress.resultJson with
          fileXRefs = add json progress.resultJson.fileXRefs;
        }
      | Hack FunctionDeclaration ->
        {
          progress.resultJson with
          functionDeclaration = add json progress.resultJson.functionDeclaration;
        }
      | Hack FunctionDefinition ->
        {
          progress.resultJson with
          functionDefinition = add json progress.resultJson.functionDefinition;
        }
      | Hack GlobalConstDeclaration ->
        {
          progress.resultJson with
          globalConstDeclaration =
            add json progress.resultJson.globalConstDeclaration;
        }
      | Hack GlobalConstDefinition ->
        {
          progress.resultJson with
          globalConstDefinition =
            add json progress.resultJson.globalConstDefinition;
        }
      | Hack InterfaceDeclaration ->
        {
          progress.resultJson with
          interfaceDeclaration =
            add json progress.resultJson.interfaceDeclaration;
        }
      | Hack InterfaceDefinition ->
        {
          progress.resultJson with
          interfaceDefinition = add json progress.resultJson.interfaceDefinition;
        }
      | Hack MethodDeclaration ->
        {
          progress.resultJson with
          methodDeclaration = add json progress.resultJson.methodDeclaration;
        }
      | Hack MethodDefinition ->
        {
          progress.resultJson with
          methodDefinition = add json progress.resultJson.methodDefinition;
        }
      | Hack MethodOccurrence ->
        {
          progress.resultJson with
          methodOccurrence = add json progress.resultJson.methodOccurrence;
        }
      | Hack MethodOverrides ->
        {
          progress.resultJson with
          methodOverrides = add json progress.resultJson.methodOverrides;
        }
      | Hack NamespaceDeclaration ->
        {
          progress.resultJson with
          namespaceDeclaration =
            add json progress.resultJson.namespaceDeclaration;
        }
      | Hack PropertyDeclaration ->
        {
          progress.resultJson with
          propertyDeclaration = add json progress.resultJson.propertyDeclaration;
        }
      | Hack PropertyDefinition ->
        {
          progress.resultJson with
          propertyDefinition = add json progress.resultJson.propertyDefinition;
        }
      | Hack TraitDeclaration ->
        {
          progress.resultJson with
          traitDeclaration = add json progress.resultJson.traitDeclaration;
        }
      | Hack TraitDefinition ->
        {
          progress.resultJson with
          traitDefinition = add json progress.resultJson.traitDefinition;
        }
      | Hack TypeConstDeclaration ->
        {
          progress.resultJson with
          typeConstDeclaration =
            add json progress.resultJson.typeConstDeclaration;
        }
      | Hack TypeConstDefinition ->
        {
          progress.resultJson with
          typeConstDefinition = add json progress.resultJson.typeConstDefinition;
        }
      | Hack TypedefDeclaration ->
        {
          progress.resultJson with
          typedefDeclaration = add json progress.resultJson.typedefDeclaration;
        }
      | Hack TypedefDefinition ->
        {
          progress.resultJson with
          typedefDefinition = add json progress.resultJson.typedefDefinition;
        }
    in
    let factIds =
      match factkey_opt with
      | None -> progress.factIds
      | Some (fact_id, json_key) ->
        JsonPredicateMap.add
          (cache_key ownership ownership_unit json_key predicate)
          fact_id
          progress.factIds
    in
    { progress with resultJson; factIds }

  let add_fact predicate json_key ({ ownership_unit; ownership; _ } as progress)
      =
    let fact_id = Fact_id.next () in
    let fields = [("id", Fact_id.to_json_number fact_id); ("key", json_key)] in
    let json_fact = JSON_Object fields in
    match
      ( should_cache predicate,
        JsonPredicateMap.find_opt
          (cache_key ownership ownership_unit json_key predicate)
          progress.factIds )
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

  let init ~ownership =
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
    {
      resultJson;
      factIds = JsonPredicateMap.empty;
      ownership_unit = None;
      ownership;
    }

  let set_ownership_unit t ou = t.ownership_unit <- ou

  let owned_facts_to_json ~ownership (predicate, owned_facts) =
    let fact_object ~ou facts =
      let obj =
        [("predicate", JSON_String predicate); ("facts", JSON_Array facts)]
      in
      JSON_Object
        (match ou with
        | Some ou -> ("unit", JSON_String ou) :: obj
        | None -> obj)
    in
    match ownership with
    | true -> List.map owned_facts ~f:(fun (ou, facts) -> fact_object ~ou facts)
    | false -> [fact_object ~ou:None (List.concat_map owned_facts ~f:snd)]

  let to_json ({ ownership; _ } as progress) =
    let resultJson = progress.resultJson in
    let preds =
      (* This is the order in which these items appear in the JSON,
         which is significant because later entries can refer to earlier ones
         by id only *)
      List.map
        ~f:(fun (pred, res) -> (to_string pred, res))
        [
          (Hack MethodOccurrence, resultJson.methodOccurrence);
          (Hack NamespaceDeclaration, resultJson.namespaceDeclaration);
          (Hack GlobalConstDeclaration, resultJson.globalConstDeclaration);
          (Hack TypedefDeclaration, resultJson.typedefDeclaration);
          (Hack InterfaceDeclaration, resultJson.interfaceDeclaration);
          (Hack TraitDeclaration, resultJson.traitDeclaration);
          (Hack ClassDeclaration, resultJson.classDeclaration);
          (Hack EnumDeclaration, resultJson.enumDeclaration);
          (Hack Enumerator, resultJson.enumerator);
          (Hack FunctionDeclaration, resultJson.functionDeclaration);
          (Hack TypeConstDeclaration, resultJson.typeConstDeclaration);
          (Hack PropertyDeclaration, resultJson.propertyDeclaration);
          (Hack ClassConstDeclaration, resultJson.classConstDeclaration);
          (Hack MethodDeclaration, resultJson.methodDeclaration);
          (Hack DeclarationSpan, resultJson.declarationSpan);
          (Hack DeclarationLocation, resultJson.declarationLocation);
          (Hack DeclarationComment, resultJson.declarationComment);
          (Hack GlobalConstDefinition, resultJson.globalConstDefinition);
          (Hack TypedefDefinition, resultJson.typedefDefinition);
          (Hack InterfaceDefinition, resultJson.interfaceDefinition);
          (Hack TraitDefinition, resultJson.traitDefinition);
          (Hack ClassDefinition, resultJson.classDefinition);
          (Hack TypeConstDefinition, resultJson.typeConstDefinition);
          (Hack PropertyDefinition, resultJson.propertyDefinition);
          (Hack ClassConstDefinition, resultJson.classConstDefinition);
          (Hack EnumDefinition, resultJson.enumDefinition);
          (Hack FunctionDefinition, resultJson.functionDefinition);
          (Hack MethodDefinition, resultJson.methodDefinition);
          (Hack MethodOverrides, resultJson.methodOverrides);
          (Hack FileXRefs, resultJson.fileXRefs);
          (Hack FileDeclarations, resultJson.fileDeclarations);
          (Src FileLines, resultJson.fileLines);
        ]
    in
    List.concat_map preds ~f:(owned_facts_to_json ~ownership)
end
