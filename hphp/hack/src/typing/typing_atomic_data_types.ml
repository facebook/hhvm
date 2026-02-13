(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Typing_env_types
module Tag = Typing_data_type.Tag
module DataTypeReason = Typing_data_type.DataTypeReason
module DataType = Typing_data_type.DataType

type t = DataType.t

type atomic_ty =
  | Primitive of Aast.tprim
  | Function
  | Nonnull
  | Tuple
  | Shape
  | Label
  | Class of string * locl_ty list

let trail = DataTypeReason.make_trail

let function_ = DataType.fun_to_datatypes ~trail

let nonnull = DataType.nonnull_to_datatypes ~trail

let tuple = DataType.tuple_to_datatypes ~trail

let shape = DataType.shape_to_datatypes ~trail

let mixed = DataType.mixed ~reason:DataTypeReason.(make NoSubreason trail)

let label = DataType.label_to_datatypes ~trail

let of_ty ~safe_for_are_disjoint env : atomic_ty -> env * DataType.t = function
  | Primitive prim -> (env, DataType.prim_to_datatypes ~trail prim)
  | Function -> (env, function_)
  | Nonnull -> (env, nonnull)
  | Tuple -> (env, tuple)
  | Shape -> (env, shape)
  | Label -> (env, label)
  | Class (name, args) ->
    DataType.Class.to_datatypes ~safe_for_are_disjoint ~trail env name
    @@ Tag.generics_for_class_and_tyl env name args

let of_tag ~safe_for_are_disjoint env tag : env * DataType.t =
  let of_ty = of_ty ~safe_for_are_disjoint in
  match tag with
  | BoolTag -> of_ty env (Primitive Aast.Tbool)
  | IntTag -> of_ty env (Primitive Aast.Tint)
  | ArraykeyTag -> of_ty env (Primitive Aast.Tarraykey)
  | FloatTag -> of_ty env (Primitive Aast.Tfloat)
  | NumTag -> of_ty env (Primitive Aast.Tnum)
  | ResourceTag -> of_ty env (Primitive Aast.Tresource)
  | NullTag -> of_ty env (Primitive Aast.Tnull)
  | ClassTag (name, args) ->
    DataType.Class.to_datatypes ~safe_for_are_disjoint ~trail env name
    @@ Tag.generics_for_class_and_tag_generic_l env name args

let empty = DataType.Set.empty

let complement dt = DataType.Set.diff mixed dt

let union dt1 dt2 = DataType.Set.union dt1 dt2

let are_disjoint env dt1 dt2 = DataType.Set.are_disjoint env dt1 dt2

let are_locl_tys_disjoint env ty1 ty2 =
  let safe_for_are_disjoint = true in
  let (env, tags1) = DataType.fromTy ~safe_for_are_disjoint env ty1 in
  let (env, tags2) = DataType.fromTy ~safe_for_are_disjoint env ty2 in
  DataType.Set.are_disjoint env tags1 tags2
