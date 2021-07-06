(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_json

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
  classConstDeclaration: json list;
  classConstDefinition: json list;
  classDeclaration: json list;
  classDefinition: json list;
  declarationComment: json list;
  declarationLocation: json list;
  declarationSpan: json list;
  enumDeclaration: json list;
  enumDefinition: json list;
  enumerator: json list;
  fileDeclarations: json list;
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
  methodOverrides: json list;
  namespaceDeclaration: json list;
  propertyDeclaration: json list;
  propertyDefinition: json list;
  traitDeclaration: json list;
  traitDefinition: json list;
  typeConstDeclaration: json list;
  typeConstDefinition: json list;
  typedefDeclaration: json list;
  typedefDefinition: json list;
}

type result_progress = {
  resultJson: glean_json;
  (* Maps fact JSON key to a list of predicate/fact id pairs *)
  factIds: (predicate * int) list JMap.t;
}

type file_info =
  Relative_path.t * Tast.program * Full_fidelity_source_text.t option

(* Containers in inheritance relationships which share the four member
types (excludes enum) *)
type parent_container_type =
  | ClassContainer
  | InterfaceContainer
  | TraitContainer
