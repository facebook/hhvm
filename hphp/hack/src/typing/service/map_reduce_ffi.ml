(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Auxiliary type used for communicating map-reduce data across FFI boundaries. *)
type t = { tast_hashes: Tast_hashes.t option }

let empty = { tast_hashes = None }
