(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type env = {
  root: Path.path;
  wait: bool;
}

let main env =
  if ClientUtils.server_exists env.root
  then begin
    ClientStop.kill_server env.root;
    ClientStart.start_server ~wait:env.wait env.root
  end else Printf.fprintf stderr "Error: no server to restart for %s\n%!"
    (Path.string_of_path env.root)
