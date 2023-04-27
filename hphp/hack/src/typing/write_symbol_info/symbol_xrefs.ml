(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Fact_id = Symbol_fact_id

module PosMap = WrappedMap.Make (struct
  let compare = Pos.compare

  type t = Pos.t
end)

type fact_map = (Hh_json.json * Pos.t list) Fact_id.Map.t

type target_info = {
  target: Hh_json.json;
  receiver_type: Hh_json.json option;
}

type pos_map = target_info PosMap.t

type t = {
  fact_map: fact_map;
  pos_map: pos_map;
}

let add { fact_map; pos_map } target_id pos { target; receiver_type } =
  let fact_map =
    Fact_id.Map.update
      target_id
      (function
        | None -> Some (target, [pos])
        | Some (json, refs) -> Some (json, pos :: refs))
      fact_map
  in
  let pos_map = PosMap.add pos { target; receiver_type } pos_map in
  { fact_map; pos_map }

let empty = { fact_map = Fact_id.Map.empty; pos_map = PosMap.empty }
