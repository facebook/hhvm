(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Validation information for fake members *)
type t

(* Validation blame: call or lambda *)
type blame =
  | Blame_call of Pos.t
  | Blame_lambda of Pos.t

(* Initial validation *)
val empty : t

(* Does one validation information entail another? *)
val sub : t -> t -> bool

(* Merge validation information at a join point.
 * If join x y = z
 * then sub x z = true and sub y z = true
 *)
val join : t -> t -> t

(* Is this identifier a fake member that is valid? *)
val is_valid : t -> Local_id.t -> bool

(* Has this identifier been invalidated? If so, return position of
 * call or lambda that is responsible *)
val is_invalid : t -> Local_id.t -> blame option

(* Invalidate all fake members, and remember the position of the call
 * or lambda that is responsible *)
val forget : t -> blame -> t

(* Add a valid fake member access *)
val add : t -> Local_id.t -> t

(* Convert to a value for logging *)
val as_log_value : t -> Typing_log_value.value

(* Make identifiers *)
val make_static_id : Nast.class_id_ -> string -> Local_id.t

val make_id : Nast.expr -> string -> Local_id.t
