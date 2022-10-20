(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val main :
  ClientEnv.client_check_env -> ServerLocalConfig.t -> Exit_status.t Lwt.t
