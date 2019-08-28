(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module dealing with inheritance.
 * When we want to declare a new class, we first have to retrieve all the
 * types that where inherited from their parents.
 *)
(*****************************************************************************)

open Decl_defs
open Typing_defs

type inherited = {
  ih_substs: subst_context SMap.t;
  ih_cstr: element option * consistent_kind;
  ih_consts: class_const SMap.t;
  ih_typeconsts: typeconst_type SMap.t;
  ih_pu_enums: pu_enum_type SMap.t;
  ih_props: element SMap.t;
  ih_sprops: element SMap.t;
  ih_methods: element SMap.t;
  ih_smethods: element SMap.t;
}

(* Builds the inherited type *)
val make : Decl_env.env -> Shallow_decl_defs.shallow_class -> inherited
