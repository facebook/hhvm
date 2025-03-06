(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let () =
  let expected = String.equal Sys.argv.(2) "true" in
  let actual = Worker_utils.is_worker_process () in
  assert (expected == actual)
