(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module NormalizeSig : sig
  val typedef : Typing_defs.typedef_type -> Typing_defs.typedef_type

  val fun_elt : Typing_defs.fun_elt -> Typing_defs.fun_elt

  val class_type : Decl_defs.decl_class_type -> Decl_defs.decl_class_type

  val ty : Typing_defs.decl_ty -> Typing_defs.decl_ty

  val shallow_class_const :
    Shallow_decl_defs.shallow_class_const ->
    Shallow_decl_defs.shallow_class_const

  val shallow_typeconst :
    Shallow_decl_defs.shallow_typeconst -> Shallow_decl_defs.shallow_typeconst

  val shallow_method :
    Shallow_decl_defs.shallow_method -> Shallow_decl_defs.shallow_method

  val shallow_prop :
    Shallow_decl_defs.shallow_prop -> Shallow_decl_defs.shallow_prop

  val shallow_class :
    Shallow_decl_defs.shallow_class -> Shallow_decl_defs.shallow_class
end
