(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val daemon_entry_point :
  (Client_ide_message.daemon_args, unit, unit) Daemon.entry

module Test : sig
  type env

  val init : custom_config:Server_config.t option -> naming_sqlite:Path.t -> env

  val index : env -> Relative_path.Set.t -> env

  val handle : env -> 'a Client_ide_message.t -> env * 'a
end
