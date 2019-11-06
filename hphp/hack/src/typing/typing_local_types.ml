(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs

let show_locl_ty _ = "<locl_ty>"

let pp_locl_ty _ _ = Printf.printf "%s\n" "<locl_ty>"

(* Along with a type, each local variable has a expression id associated with
* type. The idea is that if two local variables have the same expression_id
* it. This is used when generating expression dependent types for the 'this'
* then they refer to the same late bound type, and thus have compatible
* 'this' types.
*)
type expression_id = Ident.t [@@deriving eq, show]

type local = locl_ty * expression_id [@@deriving show]

let show_t _ = "<local_types.t>"

let pp_t _ _ = Printf.printf "%s\n" "<local_types.t>"

type t = local Local_id.Map.t

let empty = Local_id.Map.empty

let add_to_local_types id local m = Local_id.Map.add ?combine:None id local m
