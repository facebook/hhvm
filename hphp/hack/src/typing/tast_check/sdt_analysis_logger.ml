(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let create_handler ctx =
  let worker_id = !Typing_deps.worker_id |> Option.value ~default:0 in
  let db_dir = Sdt_analysis.default_db_dir in
  Sdt_analysis.create_handler ~db_dir ~worker_id ctx
