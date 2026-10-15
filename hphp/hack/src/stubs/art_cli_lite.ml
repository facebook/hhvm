(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let outgoing _ _ = []

let stamp_env _ env = env

let detach () = ()

type request = unit

let request_begin _ _ = ()

let request_run () f = f ()

let request_end () = ()
