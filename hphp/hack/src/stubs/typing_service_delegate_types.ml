(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type state = unit [@@deriving show]

let default = ()

let make ~job_runner ~tenant = ignore (job_runner, tenant)
