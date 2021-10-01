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

let type_kind_to_name_type (kind_of_type : kind_of_type) : FileInfo.name_type =
  match kind_of_type with
  | TClass -> FileInfo.Class
  | TRecordDef -> FileInfo.RecordDef
  | TTypedef -> FileInfo.Typedef

let name_kind_to_name_type (name_kind : name_kind) : FileInfo.name_type =
  match name_kind with
  | Type_kind type_kind -> type_kind_to_name_type type_kind
  | Fun_kind -> FileInfo.Fun
  | Const_kind -> FileInfo.Const

let name_kind_of_name_type (name_type : FileInfo.name_type) : name_kind =
  match name_type with
  | FileInfo.Class -> Type_kind TClass
  | FileInfo.Typedef -> Type_kind TTypedef
  | FileInfo.RecordDef -> Type_kind TRecordDef
  | FileInfo.Fun -> Fun_kind
  | FileInfo.Const -> Const_kind

let type_kind_of_name_type (name_type : FileInfo.name_type) :
    kind_of_type option =
  match name_type with
  | FileInfo.Class -> Some TClass
  | FileInfo.Typedef -> Some TTypedef
  | FileInfo.RecordDef -> Some TRecordDef
  | FileInfo.Fun
  | FileInfo.Const ->
    None

let name_kind_to_enum (name_kind : name_kind) : int =
  (* These values are stored in the naming table database, and must match [flag_of_enum] and naming_types_impl.rs
     We happen for convenience to use the same numerical values as FileInfo.name_type. *)
  name_kind |> name_kind_to_name_type |> FileInfo.name_type_to_enum

let name_kind_of_enum (i : int) : name_kind option =
  (* These values are stored in the naming table database, and must match [flag_to_enum] and naming_types_impl.rs *)
  i |> FileInfo.name_type_of_enum |> Option.map name_kind_of_name_type
