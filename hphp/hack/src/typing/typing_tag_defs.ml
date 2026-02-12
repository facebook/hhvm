(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs

type generic =
  | Reified of locl_ty
  | Erased

let equal_generic a b =
  match (a, b) with
  | (Erased, Erased) -> true
  | (Reified a, Reified b) -> ty_equal a b
  | _ -> false

type class_kind =
  | Class of generic list
  | FinalClass of generic list
  | Interface

let equal_class_kind a b =
  match (a, b) with
  | (Class a_generics, Class b_generics)
  | (FinalClass a_generics, FinalClass b_generics) ->
    let rec zip_generics al bl =
      match (al, bl) with
      | ([], []) -> []
      | (a :: al, b :: bl) -> (a, b) :: zip_generics al bl
      | (a :: al, []) -> (a, Erased) :: zip_generics al bl
      | ([], b :: bl) -> (Erased, b) :: zip_generics al bl
    in
    let pairs = zip_generics a_generics b_generics in
    List.for_all pairs ~f:(fun (a, b) -> equal_generic a b)
  | (Interface, Interface) -> true
  | _ -> false

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
  | LabelData  (** Corresponds to EnumClassLabel *)
  | BuiltInData
      (** Catch all for data types that are built into the runtime but not
          exposed in the type system. Used primarily for soundly representing
          the mixed type *)
[@@deriving eq]

(* Pad with Erased because for Foo<T>, Foo and Foo<_> should be the same;
   It's also convenient to not check for arity mismatch and failwith *)
let rec zip_generics al bl =
  match (al, bl) with
  | ([], []) -> []
  | (a :: al, b :: bl) -> (a, b) :: zip_generics al bl
  | (a :: al, []) -> (a, Erased) :: zip_generics al bl
  | ([], b :: bl) -> (Erased, b) :: zip_generics al bl

let has_reified kind =
  match kind with
  | Interface -> false
  | Class generics
  | FinalClass generics ->
    List.exists generics ~f:(fun g ->
        match g with
        | Reified _ -> true
        | Erased -> false)

let get_generics_from_kind kind =
  match kind with
  | Class generics
  | FinalClass generics ->
    generics
  | Interface -> []

let all_nonnull_tags =
  [
    DictData;
    VecData;
    KeysetData;
    StringData;
    ResourceData;
    BoolData;
    IntData;
    FloatData;
    ObjectData;
    LabelData;
    BuiltInData;
  ]

let all_tags = NullData :: all_nonnull_tags
