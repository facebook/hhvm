(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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

let name_kind_to_enum (name_kind : name_kind) : int =
  (* These values are stored in the naming table database, and must match [flag_of_enum] and naming_types_impl.rs *)
  match name_kind with
  | Type_kind TClass -> 0
  | Type_kind TTypedef -> 1
  | Type_kind TRecordDef -> 2
  | Fun_kind -> 3
  | Const_kind -> 4

let name_kind_of_enum (i : int) : name_kind option =
  (* These values are stored in the naming table database, and must match [flag_to_enum] and naming_types_impl.rs *)
  match i with
  | 0 -> Some (Type_kind TClass)
  | 1 -> Some (Type_kind TTypedef)
  | 2 -> Some (Type_kind TRecordDef)
  | 3 -> Some Fun_kind
  | 4 -> Some Const_kind
  | _ -> None
