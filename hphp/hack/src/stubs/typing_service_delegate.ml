(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type state = unit [@@deriving show]

let create () = ()

let start delegate_env state =
  ignore delegate_env;
  state

let stop state = state

let next files_to_process files_in_progress state =
  ignore (files_to_process, files_in_progress);
  (state, None)

let merge state = state

let on_cancelled state = ([], state)

let process f = f ()
