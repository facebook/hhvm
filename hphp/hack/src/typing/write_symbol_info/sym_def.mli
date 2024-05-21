(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [SymbolOccurrence.t] don't contain all information for computing xrefs.
    - Occurrence of kind "Class" can refer to classes, traits, interfaces, enum, typedefs
    - Members occurrence come with their "receiver" class but not the "origin" class
      (e.g. in Foo::Bar, Bar origin can be a parent of Foo).

  [resolve] gets these information from an occurrence. It is mostly a specialized and
  simplified implementation of [ServerSymbolDefinition.go].  *)

type kind =
  | Function
  | Class
  | Method
  | Property
  | ClassConst
  | GlobalConst
  | Enum
  | Interface
  | Trait
  | Typeconst
  | Typedef
  | Module

val kind_to_string : kind -> string

type t = {
  kind: kind;
  name: string;
  full_name: string;
}
[@@deriving show]

val resolve :
  Provider_context.t -> Relative_path.t SymbolOccurrence.t -> t option

val get_class_by_name :
  Provider_context.t -> string -> [ `None | `Enum | `Class of Nast.class_ ]

val get_kind : Provider_context.t -> string -> Ast_defs.classish_kind option

val get_overridden_method_origin :
  Provider_context.t ->
  class_name:string ->
  method_name:string ->
  is_static:bool ->
  (string * Predicate.parent_container_type) option

(** file in which a t is defined *)
val filename : Provider_context.t -> t -> Relative_path.t option
