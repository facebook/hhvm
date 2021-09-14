(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* The types in this file duplicate what's in FileInfo.name_type, but with more structure.
It'd be nice to unify them. *)

(** In the naming table, global constants and global functions can be
syntactically disambiguated at their use-site, and therefore can live in
separate namespaces. However, types (such as classes) cannot be syntactically
disambiguated, and they live in the same namespace. So in the naming table,
we also have to store what kind of type that symbol was. *)
type kind_of_type =
  | TClass
  | TTypedef
  | TRecordDef
[@@deriving show, eq]

type name_kind =
  | Type_kind of kind_of_type
  | Fun_kind
  | Const_kind
[@@deriving show, eq]

val name_kind_to_enum : name_kind -> int

val name_kind_of_enum : int -> name_kind option
