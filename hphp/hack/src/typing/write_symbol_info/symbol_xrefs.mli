(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Symbol_glean_schema.Hack
module Fact_id = Symbol_fact_id

module PosMap : WrappedMap_sig.S with type key = Pos.t

(** maps a target fact id to the json representation of the corresponding fact,
   and the positions of symbol that reference it *)
type fact_map = (XRefTarget.t * Pos.t list) Fact_id.Map.t

type target_info = {
  target: XRefTarget.t;
  receiver_type: Declaration.t option;
}

(** maps a position to various info about the target *)
type pos_map = target_info list PosMap.t

(* Maps of XRefs, constructed for a given file *)
type t = private {
  fact_map: fact_map;
  pos_map: pos_map;
}

(* Updates both maps. It is expected that for a given fact_id, all json are equal. *)
val add : t -> Fact_id.t -> Pos.t -> target_info -> t

val empty : t
