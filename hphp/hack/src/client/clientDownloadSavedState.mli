(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env = {
  root: Path.t;
  from: string;
  saved_state_manifold_api_key: string option;
  should_save_replay: bool;
  replay_token: string option;
}

val main : env -> ServerLocalConfig.t -> Exit_status.t Lwt.t
