(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Predicate types for the JSON facts emitted *)
type predicate =
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

type result_progress = {
  resultJson: glean_json;
  (* Maps fact JSON key to a list of predicate/fact id pairs *)
  factIds: (predicate * int) list Hh_json.JMap.t;
}

type file_info =
  Relative_path.t * Tast.program * Full_fidelity_source_text.t option

(* Containers in inheritance relationships which share the four member
types (excludes enum) *)
type parent_container_type =
  | ClassContainer
  | InterfaceContainer
  | TraitContainer
