(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

type generic =
  | Reified of locl_ty
  | Erased

val equal_generic : generic -> generic -> bool

type class_kind =
  | Class of generic list
  | FinalClass of generic list
  | Interface

val equal_class_kind : class_kind -> class_kind -> bool

(** Modelled after data types in HHVM. See hphp/runtime/base/datatype.h *)
type t =
  | DictData
  | ShapeData
  | VecData
  | TupleData
  | KeysetData
  | StringData
  | ResourceData
  | BoolData
  | IntData
  | FloatData
  | NullData
  | ObjectData
  | InstanceOf of {
      name: string;
      kind: class_kind;
    }
  | LabelData
  | BuiltInData

val equal : t -> t -> bool

val zip_generics : generic list -> generic list -> (generic * generic) list

val has_reified : class_kind -> bool

val get_generics_from_kind : class_kind -> generic list

val all_nonnull_tags : t list

val all_tags : t list
