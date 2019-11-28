(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type saved_state_type = Naming_table

type env = {
  root: Path.t;
  from: string;
  saved_state_type: saved_state_type;
}

val main : env -> Exit_status.t Lwt.t
