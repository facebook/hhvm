(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** In the naming table, global constants and global functions can be
syntactically disambiguated at their use-site, and therefore can live in
separate namespaces. However, types (such as classes) cannot be syntactically
disambiguated, and they live in the same namespace. So in the naming table,
we also have to store what kind of type that symbol was. *)
type kind_of_type =
  | TClass
  | TTypedef
  | TRecordDef
[@@deriving eq, enum]
