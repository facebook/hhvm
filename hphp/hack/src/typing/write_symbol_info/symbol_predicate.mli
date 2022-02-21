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

type src = FileLines

type t =
  | Hack of hack
  | Src of src

type predicate = t

(* Containers in inheritance relationships which share the four member
   types (excludes enum) *)
type parent_container_type =
  | ClassContainer
  | InterfaceContainer
  | TraitContainer

val parent_decl_predicate : parent_container_type -> string * t

val get_parent_kind : ('a, 'b) Aast.class_ -> parent_container_type

module Fact_acc : sig
  (* fact accumulator *)
  type t

  val init : t

  val to_json : t -> Hh_json.json list

  (* Add a fact of the given predicate type to the running result, if an identical
     fact has not yet been added. Return the fact's id (which can be referenced in
     other facts), and the updated accumulator. *)
  val add_fact : predicate -> Hh_json.json -> t -> Symbol_fact_id.t * t
end
