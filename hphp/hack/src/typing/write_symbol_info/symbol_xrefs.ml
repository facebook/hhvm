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

type pos_map = Hh_json.json PosMap.t

type t = {
  fact_map: fact_map;
  pos_map: pos_map;
}

let add { fact_map; pos_map } target_id pos target_json =
  let fact_map =
    Fact_id.Map.update
      target_id
      (function
        | None -> Some (target_json, [pos])
        | Some (json, refs) -> Some (json, pos :: refs))
      fact_map
  in
  let pos_map = PosMap.add pos target_json pos_map in
  { fact_map; pos_map }

let empty = { fact_map = Fact_id.Map.empty; pos_map = PosMap.empty }
