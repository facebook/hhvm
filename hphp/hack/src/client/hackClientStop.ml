(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module HackConfig : ClientStop.STOP_CONFIG = struct
  let server_desc = "Hack"
  let server_name = "hh_server"
end

module HackStopCommand = ClientStop.StopCommand (HackConfig)

let main = HackStopCommand.main

let kill_server root =
  HackStopCommand.kill_server { ClientStop.root = root; }
