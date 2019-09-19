(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let load ~repo:_ ~saved_state_type:_ =
  Future.of_value (Error "Not implemented")

let wait_for_finish _ = failwith "Not implemented"
