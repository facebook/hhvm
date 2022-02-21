(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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
[@@deriving eq]

type src = FileLines [@@deriving eq]

type t =
  | Hack of hack
  | Src of src
[@@deriving eq]

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
