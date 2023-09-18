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
      name: pos_id;
      lower: locl_ty option;
      upper: locl_ty option;
    }

(** Locl types are able to describe unknown concrete class types.
    Either via expression-dependent types (those represent the
    runtime class type of an object resulting from an expression
    evaluation). Or via the this type in a possibly abstract class
    (meant to represent the concrete class type of the object that
    will be the receiver when the method is invoked). *)
type unknown_concrete_class_kind =
  | EDT of Ident_provider.Ident.t
  | This

(** Lookups a type member in a class definition, or in the exact
    refinement information available. *)
val lookup_class_type_member :
  env ->
  on_error:Typing_error.Reasons_callback.t option ->
  this_ty:locl_ty ->
  pos_id * exact ->
  pos_id ->
  env * type_member

(** Given an unknown concrete class ([Tdependent(_)] or
    [Tgeneric("this")]) and its bounds, creates a rigid type
    variable that stands for the type constant found in the
    concrete class. *)
val make_type_member :
  env ->
  on_error:Typing_error.Reasons_callback.t option ->
  this_ty:locl_ty ->
  unknown_concrete_class_kind ->
  locl_ty list ->
  pos_id ->
  env * locl_ty
