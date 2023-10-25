(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Glean_schema.Hack

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

type src = FileLines

type gencode = GenCode

type t =
  | Hack of hack
  | Src of src
  | Gencode of gencode

type predicate = t

(* Containers in inheritance relationships which share the four member
   types (excludes enum) *)
type parent_container_type =
  | ClassContainer
  | InterfaceContainer
  | TraitContainer

val parent_decl_predicate : parent_container_type -> t

val get_parent_kind : Ast_defs.classish_kind -> parent_container_type

val container_ref : parent_container_type -> Fact_id.t -> Declaration.t

val container_decl :
  parent_container_type -> Fact_id.t -> ContainerDeclaration.t

val container_decl_qname :
  parent_container_type -> QName.t -> ContainerDeclaration.t

module Fact_acc : sig
  (* fact accumulator. This is used to maintain state through indexing
     a batch. State is mostly generated facts *)

  type t

  val init : ownership:bool -> t

  (** Returns a list of json objects. If [ownership] is false, objects
      are of the form {'predicate': PREDICATE, 'facts': FACTS'} and all
      predicates are different. If [ownership] is set, objects are of the form
     {'unit: UNIT, 'predicate': PREDICATE, 'facts': FACTS}. *)
  val to_json : t -> Hh_json.json list

  (** set the current ownership unit. All facts added after the unit is set
     will be marked with this owner. Initially, ownership_unit is set to None
     which corresponds to fact with no owners. If [ownership] is false,
     the ownership_unit is ignored. *)
  val set_ownership_unit : t -> string option -> unit

  val set_pos_map : t -> Xrefs.pos_map -> unit

  val get_pos_map : t -> Xrefs.pos_map option

  (** [add_fact pred fact t] returns an [id] and a new accumulator [t'].
     If a fact already exists in [t] for this [pred], returns [t] unchanged
     together with its id. Otherwise, [id] is a new fact id, and a fact
     of the form { 'id': id, 'key': fact } is added to the accumulator.

     If [ownership] is set, we distinguish between identical facts with different
     owners *)
  val add_fact :
    predicate -> Hh_json.json -> ?value:Hh_json.json -> t -> Fact_id.t * t
end
