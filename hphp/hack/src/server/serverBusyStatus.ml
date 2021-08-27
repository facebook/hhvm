(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let send (status : ServerCommandTypes.busy_status) : ServerEnv.seconds option =
  match Ide_info_store.get_client () with
  | None -> None
  | Some client ->
    let message = ServerCommandTypes.BUSY_STATUS status in
    (try
       ClientProvider.send_push_message_to_client client message;
       Some (Unix.gettimeofday ())
     with
    | ClientProvider.Client_went_away -> None
    | e ->
      Hh_logger.log "Failed to send busy status - %s" (Printexc.to_string e);
      None)
