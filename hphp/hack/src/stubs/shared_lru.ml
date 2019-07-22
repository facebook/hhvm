(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type host_env = unit

let init
    ~cache_name:_
    ~cache_size_in_bytes:_
    ~cache_dir_path:_
    ~num_workers:_ =
  failwith "not implemented"

let run ~host_env:_ ~job:_ ~reduce:_ ~inputs:_ = failwith "not implemented"
