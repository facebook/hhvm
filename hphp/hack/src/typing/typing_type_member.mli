(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Typing_env_types

(** This module is meant to eventually replace Typing_taccess,
    with drastically simpler logic. The idea here is to only
    have *lookup* logic on class types, unlike Typing_taccess
    that aims to work on any locl type. *)

(** The localized result of a type member lookup; in case it is
    [Abstract] existing lower and upper bounds are also returned. *)
type type_member =
  | Error of Typing_error.t option
  | Exact of locl_ty
  | Abstract of {
      lower: locl_ty option;
      upper: locl_ty option;
    }

(** Lookups a type member in a class definition, or in refinement
    information (exact). *)
val lookup_class_type_member :
  env ->
  on_error:Typing_error.Reasons_callback.t option ->
  this_ty:locl_ty ->
  pos_id * exact ->
  pos_id ->
  env * type_member

(** Given the bound on an expression-dependent type (Tdependent(_)),
    creates a rigid type variable that stands for the type constant
    found in the concrete class of the runtime object. *)
val make_dep_bound_type_member :
  env ->
  on_error:Typing_error.Reasons_callback.t option ->
  this_ty:locl_ty ->
  dependent_type ->
  locl_ty ->
  pos_id ->
  env * locl_ty
