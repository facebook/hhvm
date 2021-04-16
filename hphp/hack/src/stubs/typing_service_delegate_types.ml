(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type state = unit [@@deriving show]

let default = ()

let create
    ~job_runner
    ~artifact_store_config
    ~max_batch_size
    ~min_batch_size
    ~initial_payload_size
    ~raise_on_failure =
  ignore
    ( job_runner,
      artifact_store_config,
      max_batch_size,
      min_batch_size,
      initial_payload_size,
      raise_on_failure )
