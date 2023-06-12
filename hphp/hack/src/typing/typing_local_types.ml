(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Typing_defs

(* Along with a type, each local variable has a expression id associated with
   * it. This is used when generating expression dependent types for the 'this'
   * type. The idea is that if two local variables have the same expression_id
   * then they refer to the same late bound type, and thus have compatible
   * 'this' types.
   * It also has a position that indicates where the local got this type.
   *
*)
type expression_id = Ident.t [@@deriving eq, show]

type local = {
  ty: locl_ty;
  bound_ty: locl_ty option;
  pos: Pos.t;
  eid: expression_id;
}
[@@deriving show]

type t = local Local_id.Map.t

let empty = Local_id.Map.empty

let add_to_local_types id local m = Local_id.Map.add ?combine:None id local m
