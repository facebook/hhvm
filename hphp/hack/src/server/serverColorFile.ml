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

type result = ((int * int) * Coverage_level.level) list

let go env f_in oc =
  let type_acc = ref [] in
  let fn = Typing.with_expr_hook
    (fun e ty -> type_acc := (fst e, ty) :: !type_acc)
    (fun () ->
      ServerIdeUtils.check_file_input env.ServerEnv.files_info f_in
    ) in
  let result = Coverage_level.mk_level_list fn !type_acc in
  let result = rev_rev_map (fun (p, cl) -> Pos.info_raw p, cl) result in
  Marshal.to_channel oc (result : result) [];
  flush oc
