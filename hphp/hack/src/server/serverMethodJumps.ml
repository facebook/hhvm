(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open ServerEnv

let go class_ find_children env genv oc =
  let ctx = Provider_utils.ctx_from_server_env env in
  let res_list =
    (* Might raise {!Naming_table.File_info_not_found} *)
    MethodJumps.get_inheritance
      ctx
      class_
      ~find_children
      env.naming_table
      genv.workers
  in
  Marshal.to_channel oc res_list [];
  flush oc;
  ()
