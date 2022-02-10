(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [tagged_elt] is a representation internal to Decl_inheritance which is used
    for both methods and properties (members represented using
    {!Typing_defs.class_elt}). Tagging these members with [inherit_when_private]
    allows us to assign private trait members to the class which used the trait
    and to filter out other private members. *)
type tagged_elt = {
  id: string;
  inherit_when_private: bool;
  elt: Typing_defs.class_elt;
}

val shallow_method_to_class_elt :
  string ->
  (* class name *)
  string option ->
  (* module name *)
  Decl_defs.mro_element ->
  Typing_defs.decl_ty SMap.t ->
  Shallow_decl_defs.shallow_method ->
  Typing_defs.class_elt

val shallow_method_to_telt :
  string ->
  (* class name *)
  string option ->
  (* module name *)
  Decl_defs.mro_element ->
  Typing_defs.decl_ty SMap.t ->
  Shallow_decl_defs.shallow_method ->
  tagged_elt

val shallow_prop_to_telt :
  string ->
  (* class name *)
  string option ->
  (* module name *)
  Decl_defs.mro_element ->
  Typing_defs.decl_ty SMap.t ->
  Shallow_decl_defs.shallow_prop ->
  tagged_elt

val shallow_const_to_class_const :
  string ->
  Decl_defs.mro_element ->
  Typing_defs.decl_ty SMap.t ->
  Shallow_decl_defs.shallow_class_const ->
  string * Typing_defs.class_const

val classname_const : Typing_defs.pos_id -> string * Typing_defs.class_const

val typeconst_structure :
  Decl_defs.mro_element ->
  string ->
  Shallow_decl_defs.shallow_typeconst ->
  string * Typing_defs.class_const

val shallow_typeconst_to_typeconst_type :
  string ->
  Decl_defs.mro_element ->
  Typing_defs.decl_ty SMap.t ->
  Shallow_decl_defs.shallow_typeconst ->
  string * Typing_defs.typeconst_type
