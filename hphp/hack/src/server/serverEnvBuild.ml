(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* Building the environment *)
(*****************************************************************************)
open Utils
open ServerEnv

let make_genv ~multicore options =
  let root         = ServerArgs.root options in
  let check_mode   = ServerArgs.check_mode options in
  Typing_deps.trace :=
    not check_mode || ServerArgs.convert options <> None;
  let nbr_procs    = ServerConfig.nbr_procs in
  let workers = 
    if multicore then Some (Worker.make nbr_procs) else None
  in
  if not check_mode
  then begin
    if not check_mode && not (Lock.check root "lock")
    then begin
      Printf.fprintf stderr "Error: another server is already running?\n";
      exit 1
    end;
    ()
  end;
  { options      = options;
    workers      = workers;
  }

let make_env options =
  { nenv           = Naming.empty;
    files_info     = SMap.empty;
    errorl         = [];
    skip           = ref (ServerArgs.skip_init options);
    failed_parsing = SSet.empty;
    failed_decl    = SSet.empty;
    failed_check   = SSet.empty;
  }
