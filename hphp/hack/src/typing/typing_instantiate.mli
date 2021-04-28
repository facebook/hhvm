(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

(*****************************************************************************)
(* Types *)
(*****************************************************************************)

type subst

(*****************************************************************************)
(* Builds a substitution out of a list of type parameters and a list of types.
 *
 * Typical use-case:
 *   class Y<T> { ... }
 *   class X extends Y<int>
 *
 * To build the type of X, we to replace all the occurrences of T in Y by int.
 * The function make_subst, builds the substition (the map associating types
 * to a type parameter name), in this case, it would build the map(T => int).
 *)
(*****************************************************************************)

val make_subst : decl_tparam list -> decl_ty list -> subst

(*****************************************************************************)
(* Primitive instantiating a type.
 * TODO: explain what instantiation is about.
 *)
(*****************************************************************************)

val instantiate : subst -> decl_ty -> decl_ty

val instantiate_ce : subst -> class_elt -> class_elt

val instantiate_typeconst_type : subst -> typeconst_type -> typeconst_type
