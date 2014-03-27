(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils
open ServerEnv

(* Initialization of the server *)
let init_hack genv env get_next =
  let files_info, errorl1, failed1 =
    Parsing_service.go genv.workers env.files_info ~get_next in

  let is_check_mode =
    ServerArgs.check_mode genv.options &&
    ServerArgs.convert genv.options = None
  in
  if not is_check_mode then begin
    Typing_deps.update_files genv.workers files_info;
  end;

  let nenv = env.nenv in

  let errorl2, failed2, nenv =
    SMap.fold Naming.ndecl_file files_info ([], SSet.empty, nenv) in


  let fast = FileInfo.simplify_fast files_info in
  let fast = SSet.fold SMap.remove failed2 fast in
  let errorl3, failed3 = Typing_decl_service.go genv.workers nenv fast in

  let fast = SSet.fold SMap.remove failed3 fast in
  let errorl4, failed4 = Typing_check_service.go genv.workers fast in

  let failed =
    List.fold_right
      SSet.union [failed1; failed2; failed3; failed4] SSet.empty in
  let env = { env with files_info = files_info; nenv = nenv } in

  SharedMem.init_done();

  let errorl = List.fold_right List.rev_append
      [errorl1; errorl2; errorl3; errorl4] [] in
  env, errorl, failed

(* entry point *)
let init genv env next_files =
  let env, errorl, failed = init_hack genv env next_files in
  let env = { env with errorl = errorl;
              failed_parsing = failed } in
  ServerError.print_errorl (ServerArgs.json_mode genv.options) env.errorl stdout;
  if !(env.skip)
  then { env with errorl = [];
        failed_parsing = SSet.empty;
        failed_check = SSet.empty }
  else env
