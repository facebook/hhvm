(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let update = "hg.update"

let transaction = "hg.transaction"

let is_hg_state state =
  String.equal state update || String.equal state transaction
