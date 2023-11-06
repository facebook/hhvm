(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-66"] (* yojson *)

(** Auxiliary type used for communicating map-reduce data across FFI boundaries. *)
type t = {
  tast_hashes: Tast_hashes.t option; [@yojson.option]
  tast_collector: Tast_collector.t option; [@yojson.option]
  type_counter: Type_counter.t option; [@yojson.option]
  reason_collector: Reason_collector.t option; [@yojson.option]
}
[@@deriving yojson_of]

let empty =
  {
    tast_hashes = None;
    tast_collector = None;
    type_counter = None;
    reason_collector = None;
  }
