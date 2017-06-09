(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

type t

(**
 * Initiates a client.
 *
 * Connects to the Watcher that's watching the repo at this path.
 *
 * If a connection cannot be made to a Watcher, returns None.
 *)
val init : Path.t -> t option

(**
 * Non-blocking poll on the connection - returns true if the Watcher reports
 * settled state, or we have previously already read the settled message.
 *
 * If Watchman Event Watcher connection fails, None is returned.
 *)
val get_status : t -> WatchmanEventWatcherConfig.Responses.t option
