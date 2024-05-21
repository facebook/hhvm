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

(* class_name is the canonical qualified name to a container kind.
   starts with a backslash *)
type t =
  | Function of { name: string }
  | Method of {
      class_name: string;
      name: string;
    }
  | Property of {
      class_name: string;
      name: string;
    }
  | ClassConst of {
      class_name: string;
      name: string;
    }
  | GlobalConst of { name: string }
  | Class of {
      kind: Ast_defs.classish_kind;
      name: string;
    }
  | Typeconst of {
      class_name: string;
      name: string;
    }
  | Typedef of { name: string }
  | Module of { name: string }
[@@deriving show]

val resolve :
  Provider_context.t -> Relative_path.t SymbolOccurrence.t -> t option

val get_kind : Provider_context.t -> string -> Ast_defs.classish_kind option

val get_overridden_method_origin :
  Provider_context.t ->
  class_name:string ->
  method_name:string ->
  is_static:bool ->
  (string * Predicate.parent_container_type) option

(** file in which a t is defined *)
val filename : Provider_context.t -> t -> Relative_path.t option
