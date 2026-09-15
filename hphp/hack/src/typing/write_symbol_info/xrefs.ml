(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hack

module PosMap = Wrapped_map.Make (struct
  let compare = Pos.compare

  type t = Pos.t
end)

type fact_map = (XRefTarget.t * Pos.t list) Fact_id.Map.t

type target_info = {
  target: XRefTarget.t;
  receiver_type: Declaration.t option;
  fact_id: Fact_id.t;
}

type pos_map = target_info list PosMap.t

type used_in_prod_build_map = bool Fact_id.Map.t

type t = {
  fact_map: fact_map;
  pos_map: pos_map;
  used_in_prod_build: used_in_prod_build_map;
}

let add { fact_map; pos_map; used_in_prod_build } target_id pos target_info =
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
  { fact_map; pos_map; used_in_prod_build }

let mark_target_used_in_prod_build t fact_id affects_prod_build =
  let used_in_prod_build =
    Fact_id.Map.update
      fact_id
      (function
        | None -> Some affects_prod_build
        | Some prev -> Some (prev || affects_prod_build))
      t.used_in_prod_build
  in
  { t with used_in_prod_build }

let empty =
  {
    fact_map = Fact_id.Map.empty;
    pos_map = PosMap.empty;
    used_in_prod_build = Fact_id.Map.empty;
  }
