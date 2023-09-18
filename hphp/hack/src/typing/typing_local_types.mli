(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type expression_id = Ident_provider.Ident.t [@@deriving eq, show]

type local = {
  ty: Typing_defs.locl_ty;
  defined: bool;
  bound_ty: Typing_defs.locl_ty option;
  pos: Pos.t;
  eid: expression_id;
}
[@@deriving show]

type t = local Local_id.Map.t

val empty : t

val add_to_local_types :
  Local_id.t -> 'a -> 'a Local_id.Map.t -> 'a Local_id.Map.t
