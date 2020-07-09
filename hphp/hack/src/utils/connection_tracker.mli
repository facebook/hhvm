(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  id: string;
  mutable t_sleep_and_check: float;
      (** 1. The server starts a loop doing slices of major GC *)
  mutable t_monitor_fd_ready: float;
      (** 2. until it detects something on the monitor FD *)
  mutable t_got_client_fd: float;
      (** 3. It synchronously reads the client FD from the monitor *)
  mutable t_got_tracker: float;
      (** 4. and synchronouysly reads the tracker from the monitor *)
  mutable t_start_server: float;
      (** 5. This is where tracker has been read from monitor and server_one_iteration is ready *)
  mutable t_done_recheck: float;
      (** 6. It finishes 'recheck_loop' to process all outstanding changes *)
  mutable t_sent_diagnostics: float;
      (** 7. It sends any diagnostics needed to the persistent connection *)
  mutable t_start_handle_connection: float;
      (** 8. At this point it can turn its attention to the client *)
  mutable t_sent_hello: float;
      (** 9. It has sent a hello message to the client *)
  mutable t_got_connection_type: float;
      (** 10. It received the connection type from the client *)
  mutable t_waiting_for_cmd: float;
      (** 11. Now it's ready to receive the cmd from the client *)
  mutable t_got_cmd: float;
      (** 12. At this point it got the command from the client *)
  mutable t_done_full_recheck: float;
      (** 13. It does another recheck if needed *)
  mutable t_start_server_handle: float;
      (** 14. It has sent a ping. It is now proceeds to ServerRpc.handle *)
  mutable t_end_server_handle: float;
      (** 15. It has finished ServerRpc.handle *)
  mutable t_end_server_handle2: float;
      (** 16. and finished further minor processing. Now it will send the response to the client. *)
}

val create : unit -> t

val log_id : t -> string
