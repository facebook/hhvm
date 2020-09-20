(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * A client to get the repo state from an already-running
 * server watching a repo.
 *
 * Since the server sends at most 3 messages to connected clients, this client
 * reads at most 3 messages (unknown, mid_update, settled).
 * After reading the "settled" message, future calls to issettled
 * return true immediately.
 *)

module Abstract_types = struct
  type t
end

module type S = sig
  include module type of Abstract_types

  (**
   * Initiates a client.
   *
   * Connects to the Watcher that's watching the repo at this path.
   *
   * If a connection cannot be made to a Watcher, returns None.
   *)
  val init : Path.t -> t

  (**
   * Non-blocking poll on the connection - returns true if the Watcher reports
   * settled state, or we have previously already read the settled message.
   *
   * If Watchman Event Watcher connection fails, None is returned.
   *)
  val get_status : t -> WatchmanEventWatcherConfig.Responses.t option

  module Mocking : sig
    val get_status_returns :
      WatchmanEventWatcherConfig.Responses.t option -> unit
  end
end
