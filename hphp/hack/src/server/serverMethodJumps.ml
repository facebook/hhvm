(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open ServerEnv

let go class_ find_children env genv oc =
  let res_list =
    MethodJumps.get_inheritance class_ ~find_children
      env.naming_table genv.workers in
  Marshal.to_channel oc res_list [];
  flush oc;
  ()
