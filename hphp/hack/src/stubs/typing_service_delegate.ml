(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type state = Typing_service_delegate_types.state

let pp_state = Typing_service_delegate_types.pp_state

let show_state = Typing_service_delegate_types.show_state

let default = Typing_service_delegate_types.default

let create = Typing_service_delegate_types.create

let start delegate_env state =
  ignore delegate_env;
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

let get_progress state =
  ignore state;
  None
