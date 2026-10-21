(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let prepare_client_startup () = ()

type fetch = {
  start_time: float;
  end_time: float;
}

let apply_qe_overrides ~silent:_ config = (config, [])
