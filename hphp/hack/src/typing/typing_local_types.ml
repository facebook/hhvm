(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Typing_defs

type local = {
  ty: locl_ty;  (** The type of the local *)
  defined: bool;
      (** True if the variable is definitely defined. False if it might not be.
        This will happen when there is a typed local declared along some control
        paths but not others. In this case, using the variable is still an
        error, but any assigning to it should still follow the bound. *)
  bound_ty: locl_ty option;
      (** The bound on the variable if it is a typed local. *)
  pos: Pos.t;  (** The position at which the variable got its type. *)
  eid: Expression_id.t;
      (** Along with a type, each local variable has a expression id associated with
          it. This is used when generating expression dependent types for the 'this'
          type. The idea is that if two local variables have the same expression_id
          then they refer to the same late bound type, and thus have compatible
          'this' types. *)
}
[@@deriving show]

type t = local Local_id.Map.t

let empty = Local_id.Map.empty

let add_to_local_types id local m = Local_id.Map.add ?combine:None id local m
