(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module type CHECKER_UTILS = sig
  val start_server : ClientEnv.client_check_env -> unit
  val server_name : string
end

module type STATUS_CHECKER = sig
  val check_status : ClientEnv.client_check_env -> unit
end

module StatusChecker (CheckerUtils : CHECKER_UTILS) : STATUS_CHECKER

val check_status : ClientEnv.client_check_env -> unit
