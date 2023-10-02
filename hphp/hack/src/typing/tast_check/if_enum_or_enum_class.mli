(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type kind =
  | Enum of {
      name: string;
      class_decl: Decl_provider.class_decl;
    }
  | EnumClass of {
      name: string;
      interface: Typing_defs.locl_ty;
      class_decl: Decl_provider.class_decl;
    }
  | EnumClassLabel of {
      name: string;
      interface: Typing_defs.locl_ty;
      class_decl: Decl_provider.class_decl;
    }

val name : kind -> string

val decl : kind -> Decl_provider.class_decl

val apply :
  Tast_env.env ->
  default:'a ->
  f:(kind -> 'a) ->
  Decl_provider.type_key ->
  Typing_defs.locl_phase Typing_defs.ty list ->
  'a
