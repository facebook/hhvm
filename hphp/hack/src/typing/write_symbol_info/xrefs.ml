(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Glean_schema.Hack

module PosMap = WrappedMap.Make (struct
  let compare = Pos.compare

  type t = Pos.t
end)

type fact_map = (XRefTarget.t * Pos.t list) Fact_id.Map.t

type target_info = {
  target: XRefTarget.t;
  receiver_type: Declaration.t option;
}

type pos_map = target_info list PosMap.t

type t = {
  fact_map: fact_map;
  pos_map: pos_map;
}

let add { fact_map; pos_map } target_id pos target_info =
  let fact_map =
    Fact_id.Map.update
      target_id
      (function
        | None -> Some (target_info.target, [pos])
        | Some (json, refs) -> Some (json, pos :: refs))
      fact_map
  in
  let pos_map =
    PosMap.update
      pos
      (function
        | None -> Some [target_info]
        | Some tis -> Some (target_info :: tis))
      pos_map
  in
  { fact_map; pos_map }

let empty = { fact_map = Fact_id.Map.empty; pos_map = PosMap.empty }
