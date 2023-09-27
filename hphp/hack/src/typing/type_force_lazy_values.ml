(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let locl_ty (ty : Typing_defs_core.locl_ty) : Typing_defs_core.locl_ty =
  let visitor =
    object
      inherit [unit] Type_mapper_generic.deep_type_mapper

      method! on_reason () r = ((), Typing_reason.force_lazy_values r)
    end
  in
  let ((), ty) = visitor#on_type () ty in
  ty

let internal_type (ty : Typing_defs_core.internal_type) =
  let visitor =
    object
      inherit [unit] Type_mapper_generic.internal_type_mapper

      method! on_reason () r = ((), Typing_reason.force_lazy_values r)
    end
  in
  let ((), ty) = visitor#on_internal_type () ty in
  ty
