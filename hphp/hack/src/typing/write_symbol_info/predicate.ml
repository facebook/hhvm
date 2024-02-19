(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hh_json
open Hack

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
  | InheritedMembers
  | InterfaceDeclaration
  | InterfaceDefinition
  | MemberCluster
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
  | ModuleDeclaration
  | ModuleDefinition
  | FileCall
  | GlobalNamespaceAlias
  | IndexerInputsHash
  | TypeInfo
[@@deriving ord]

type src = FileLines [@@deriving ord]

type gencode = GenCode [@@deriving ord]

type t =
  | Hack of hack
  | Src of src
  | Gencode of gencode
[@@deriving ord]

type predicate = t

let compare_predicate = compare

let gencode_to_string = function
  | GenCode -> "GenCode"

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
  | InheritedMembers -> "InheritedMembers"
  | InterfaceDeclaration -> "InterfaceDeclaration"
  | InterfaceDefinition -> "InterfaceDefinition"
  | MemberCluster -> "MemberCluster"
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
  | ModuleDeclaration -> "ModuleDeclaration"
  | ModuleDefinition -> "ModuleDefinition"
  | FileCall -> "FileCall"
  | GlobalNamespaceAlias -> "GlobalNamespaceAlias"
  | IndexerInputsHash -> "IndexerInputsHash"
  | TypeInfo -> "TypeInfo"

(* List of all predicates, in the order in which they should appear in the JSON.
   This guarantee that facts are introduced before they are referenced. *)
let ordered_all =
  [
    Gencode GenCode;
    Hack MethodOccurrence;
    Hack NamespaceDeclaration;
    Hack GlobalConstDeclaration;
    Hack TypedefDeclaration;
    Hack ModuleDeclaration;
    Hack InterfaceDeclaration;
    Hack TraitDeclaration;
    Hack ClassDeclaration;
    Hack EnumDeclaration;
    Hack Enumerator;
    Hack FunctionDeclaration;
    Hack TypeConstDeclaration;
    Hack TypeInfo;
    Hack PropertyDeclaration;
    Hack ClassConstDeclaration;
    Hack MethodDeclaration;
    Hack DeclarationSpan;
    Hack DeclarationLocation;
    Hack DeclarationComment;
    Hack MemberCluster;
    Hack InheritedMembers;
    Hack GlobalConstDefinition;
    Hack TypedefDefinition;
    Hack ModuleDefinition;
    Hack InterfaceDefinition;
    Hack TraitDefinition;
    Hack ClassDefinition;
    Hack TypeConstDefinition;
    Hack PropertyDefinition;
    Hack ClassConstDefinition;
    Hack EnumDefinition;
    Hack FunctionDefinition;
    Hack MethodDefinition;
    Hack MethodOverrides;
    Hack FileXRefs;
    Hack FileDeclarations;
    Hack FileCall;
    Hack GlobalNamespaceAlias;
    Hack IndexerInputsHash;
    Src FileLines;
  ]

let src_to_string = function
  | FileLines -> "FileLines"

let to_string = function
  | Hack x -> "hack." ^ hack_to_string x ^ ".6"
  | Src x -> "src." ^ src_to_string x ^ ".1"
  | Gencode x -> "gencode." ^ gencode_to_string x ^ ".1"

(* Containers in inheritance relationships which share the four member
   types (excludes enum) *)
type parent_container_type =
  | ClassContainer
  | InterfaceContainer
  | TraitContainer

let parent_decl_predicate parent_container_type =
  match parent_container_type with
  | ClassContainer -> Hack ClassDeclaration
  | InterfaceContainer -> Hack InterfaceDeclaration
  | TraitContainer -> Hack TraitDeclaration

let container_decl container_type id =
  match container_type with
  | ClassContainer -> ContainerDeclaration.Class_ (ClassDeclaration.Id id)
  | InterfaceContainer ->
    ContainerDeclaration.Interface_ (InterfaceDeclaration.Id id)
  | TraitContainer -> ContainerDeclaration.Trait (TraitDeclaration.Id id)

let container_decl_qname container_type name =
  match container_type with
  | ClassContainer ->
    ContainerDeclaration.Class_ ClassDeclaration.(Key { name })
  | InterfaceContainer ->
    ContainerDeclaration.Interface_ InterfaceDeclaration.(Key { name })
  | TraitContainer -> ContainerDeclaration.Trait TraitDeclaration.(Key { name })

let container_ref container_type id =
  Declaration.Container (container_decl container_type id)

let get_parent_kind = function
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
  | Hack TypedefDeclaration
  | Hack ModuleDeclaration ->
    true
  | _ -> false

module Map = WrappedMap.Make (struct
  type t = predicate

  let compare = compare_predicate
end)

module Fact_acc = struct
  type ownership_unit = string option [@@deriving eq, ord]

  type owned_facts = (ownership_unit * json list) list

  module JsonPredicateMap = WrappedMap.Make (struct
    let compare_json = JsonKey.compare

    let compare_predicate = compare

    type t = ownership_unit * json * predicate [@@deriving ord]
  end)

  type t = {
    resultJson: owned_facts Map.t;
    factIds: Fact_id.t JsonPredicateMap.t;
    mutable ownership_unit: ownership_unit;
    mutable xrefs: Xrefs.pos_map option;
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
      predicate json factkey_opt ({ ownership; ownership_unit; _ } as fa) =
    let update (opt_key : owned_facts option) : owned_facts option =
      match opt_key with
      | Some facts -> Some (add_to_owned_facts ownership_unit json facts)
      | None -> failwith "All predicate keys should be in the map"
    in
    let resultJson = Map.update predicate update fa.resultJson in
    let factIds =
      match factkey_opt with
      | None -> fa.factIds
      | Some (fact_id, json_key) ->
        JsonPredicateMap.add
          (cache_key ownership ownership_unit json_key predicate)
          fact_id
          fa.factIds
    in
    { fa with resultJson; factIds }

  let add_fact
      predicate json_key ?value ({ ownership_unit; ownership; _ } as fa) =
    let value =
      match value with
      | None -> []
      | Some v -> [("value", v)]
    in
    let fact_id = Fact_id.next () in
    let fields =
      [("id", Fact_id.to_json_number fact_id); ("key", json_key)] @ value
    in
    let json_fact = JSON_Object fields in
    match
      ( should_cache predicate,
        JsonPredicateMap.find_opt
          (cache_key ownership ownership_unit json_key predicate)
          fa.factIds )
    with
    | (false, _) -> (fact_id, update_glean_json predicate json_fact None fa)
    | (true, None) ->
      ( fact_id,
        update_glean_json predicate json_fact (Some (fact_id, json_key)) fa )
    | (true, Some fid) -> (fid, fa)

  let init ~ownership =
    let resultJson =
      List.fold ordered_all ~init:Map.empty ~f:(fun acc k -> Map.add k [] acc)
    in
    {
      resultJson;
      factIds = JsonPredicateMap.empty;
      ownership_unit = None;
      ownership;
      xrefs = None;
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

  let to_json ({ ownership; _ } as fa) =
    let preds =
      List.map
        ~f:(fun pred -> (to_string pred, Map.find pred fa.resultJson))
        ordered_all
    in
    List.concat_map preds ~f:(owned_facts_to_json ~ownership)

  let set_pos_map t xrefs = t.xrefs <- Some xrefs

  let get_pos_map t = t.xrefs
end
