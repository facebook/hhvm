(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

val entry: (ServerArgs.options, unit, unit) Daemon.entry

val daemon_main:
  ServerArgs.options -> ('a Daemon.in_channel * 'b Daemon.out_channel) -> unit
