(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_service_types

type state = unit [@@deriving show]

let create () = ()

let init state = state

let next files_to_process state =
  ignore (files_to_process, state);
  None

let merge state =
  ((Errors.empty, { completed = []; remaining = []; deferred = [] }), state)

let on_cancelled state = ([], state)

let process state = state
