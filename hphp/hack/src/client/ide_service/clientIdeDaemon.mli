(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val daemon_entry_point : (ClientIdeMessage.daemon_args, unit, unit) Daemon.entry

module Test : sig
  type env

  val init : custom_config:ServerConfig.t option -> naming_sqlite:Path.t -> env

  val index : env -> Relative_path.Set.t -> env

  val handle : env -> 'a ClientIdeMessage.t -> env * 'a
end
