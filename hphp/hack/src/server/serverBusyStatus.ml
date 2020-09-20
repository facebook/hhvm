(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let send (env : ServerEnv.env) (status : ServerCommandTypes.busy_status) : unit
    =
  match env.ServerEnv.persistent_client with
  | None -> ()
  | Some client ->
    let message = ServerCommandTypes.BUSY_STATUS status in
    (try ClientProvider.send_push_message_to_client client message with
    | ClientProvider.Client_went_away -> ()
    | e ->
      Hh_logger.log "Failed to send busy status - %s" (Printexc.to_string e))
