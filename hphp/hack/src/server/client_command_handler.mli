(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Handle client command messages and connections from persistent clients. *)
val handle_client_command_or_persistent_connection :
  Server_env.genv ->
  Server_env.env ->
  Client_provider.client ->
  Server_env.env Server_utils.handle_command_result
