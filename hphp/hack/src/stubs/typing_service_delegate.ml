(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type state = unit [@@deriving show]

let create ?(max_batch_size = 0) ?(min_batch_size = 0) () =
  ignore (max_batch_size, min_batch_size)

let start delegate_env state ~recheck_id =
  ignore (delegate_env, recheck_id);
  state

let stop state = state

let next files_to_process files_in_progress state =
  ignore (files_to_process, files_in_progress);
  (state, None)

let merge state errors computation_progress =
  ignore (errors, computation_progress);
  state

let on_cancelled state = ([], state)

let process f = f ()

let steal state n =
  ignore n;
  ([], state)

let add_telemetry state telemetry =
  ignore state;
  telemetry
